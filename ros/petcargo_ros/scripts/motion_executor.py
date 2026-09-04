#!/usr/bin/env python3
"""Execute PetCargo motions with odometry and fail-closed local lidar avoidance."""

import json
import math
import time
import traceback

import rospy
import tf2_ros
from geometry_msgs.msg import Twist
from nav_msgs.msg import Odometry
from sensor_msgs.msg import LaserScan
from std_msgs.msg import Bool, Empty, String

from petcargo_ros.motion import AngleAccumulator, angular_speed_for_error, linear_speed_for_error
from petcargo_ros.obstacle import (
    choose_detour_path,
    corridor_clearance,
    detour_target_satisfied,
    direction_vector,
    filter_footprint_points,
    next_detour_phase,
    lidar_health,
    rotation_blocked,
    speed_for_clearance,
    transform_scan_points,
    update_obstacle_confirmation,
)
from petcargo_ros.protocol import MotionKind, MotionResultCode


DIRECTION_NAMES = {1: "forward", 2: "backward", 3: "left", 4: "right"}
PHASE_NAMES = {"track": "直行", "shift_out": "侧移", "pass": "越过", "shift_back": "回归"}


def yaw_from_quaternion(q) -> float:
    return math.atan2(2.0 * (q.w * q.z + q.x * q.y), 1.0 - 2.0 * (q.y * q.y + q.z * q.z))


def yaw_from_odom(message: Odometry) -> float:
    return yaw_from_quaternion(message.pose.pose.orientation)


def direction_from_request(request):
    distance = int(request.get("distance_mm", 0))
    if request["kind"] == int(MotionKind.LINEAR) and distance:
        return 1 if distance > 0 else 2
    if request["kind"] == int(MotionKind.LATERAL) and distance:
        return 3 if distance > 0 else 4
    return None


def local_progress(start_x, start_y, heading, direction, x, y):
    """Project odometry displacement onto a robot-local cardinal direction."""
    world_x = x - start_x
    world_y = y - start_y
    local_x = math.cos(heading) * world_x + math.sin(heading) * world_y
    local_y = -math.sin(heading) * world_x + math.cos(heading) * world_y
    dx, dy = direction_vector(direction)
    return local_x * dx + local_y * dy


def command_for_direction(direction, speed):
    command = Twist()
    if direction == 1:
        command.linear.x = speed
    elif direction == 2:
        command.linear.x = -speed
    elif direction == 3:
        command.linear.y = speed
    elif direction == 4:
        command.linear.y = -speed
    return command


class MotionExecutor:
    def __init__(self) -> None:
        self.command_pub = rospy.Publisher(
            rospy.get_param("~internal_cmd_topic", "/petcargo/cmd_vel_request"), Twist, queue_size=1
        )
        self.result_pub = rospy.Publisher("/petcargo/motion_result", String, queue_size=10)
        self.status_pub = rospy.Publisher("/petcargo/motion_status", String, queue_size=10, latch=True)
        self.lidar_pub = rospy.Publisher("/petcargo/lidar_status", String, queue_size=4, latch=True)

        self.linear_timeout = float(rospy.get_param("~linear_timeout", 20.0))
        self.angular_timeout = float(rospy.get_param("~angular_timeout", 15.0))
        self.lidar_timeout = float(rospy.get_param("~lidar_timeout", 0.5))
        self.base_frame = rospy.get_param("~base_frame", "base_link")
        self.half_length = float(rospy.get_param("~footprint_half_length", 0.171))
        self.half_width = float(rospy.get_param("~footprint_half_width", 0.128))
        self.corridor_margin = float(rospy.get_param("~corridor_margin", 0.03))
        self.stop_clearance = float(rospy.get_param("~obstacle_stop_clearance", 0.15))
        self.emergency_clearance = float(rospy.get_param("~obstacle_emergency_clearance", 0.10))
        self.slow_clearance = float(rospy.get_param("~obstacle_slow_clearance", 0.35))
        self.confirm_scans = int(rospy.get_param("~obstacle_confirm_scans", 2))
        self.shift_distance = float(rospy.get_param("~avoid_shift_distance", 0.40))
        self.pass_distance = float(rospy.get_param("~avoid_pass_distance", 0.50))
        self.avoid_speed = float(rospy.get_param("~avoid_speed", 0.10))
        self.min_valid_points = int(rospy.get_param("~lidar_min_valid_points", 10))

        self.tf_buffer = tf2_ros.Buffer(cache_time=rospy.Duration(5.0))
        self.tf_listener = tf2_ros.TransformListener(self.tf_buffer)
        self.scan_points = []
        self.scan_time = 0.0
        self.scan_generation = 0
        self.scan_valid_count = 0
        self.scan_self_filtered = 0
        self.scan_error = "waiting_for_scan"
        self.odom = None
        self.odom_time = 0.0
        self.active = None
        self.jog = None
        self.avoidance = None
        self.jog_blocked_key = None
        self.safety_latched = False
        self.completed_ids = set()
        self.next_lidar_status = 0.0

        # Register callbacks only after every field they can touch exists.  A
        # 10 Hz lidar can otherwise invoke on_scan during construction.
        rospy.Subscriber(rospy.get_param("~odom_topic", "/odom"), Odometry, self.on_odom, queue_size=10)
        rospy.Subscriber(rospy.get_param("~scan_topic", "/scan"), LaserScan, self.on_scan, queue_size=1)
        rospy.Subscriber("/petcargo/motion_request", String, self.on_request, queue_size=10)
        rospy.Subscriber("/petcargo/jog_request", String, self.on_jog, queue_size=10)
        rospy.Subscriber("/petcargo/safety_set", Bool, self.on_safety, queue_size=4)
        rospy.Subscriber("/petcargo/cancel_motion", Empty, self.on_cancel, queue_size=4)
        rospy.Subscriber("/petcargo/serial_connected", Bool, self.on_connected, queue_size=4)
        self.publish_status("idle")
        self.publish_lidar_status()

    def run(self) -> None:
        """Run the safety-critical controller on the node's main thread.

        A rospy.Timer callback runs in a daemon thread and an uncaught callback
        failure can leave every subscriber alive while velocity publication has
        silently stopped.  Keeping the 20 Hz loop here makes that failure visible
        and guarantees a zero command before retrying the next cycle.
        """
        rate = rospy.Rate(20)
        while not rospy.is_shutdown():
            try:
                self.on_timer(None)
            except Exception:
                self.command_pub.publish(Twist())
                rospy.logerr_throttle(
                    1.0, "PetCargo control-loop exception:\n%s", traceback.format_exc()
                )
            try:
                rate.sleep()
            except rospy.ROSInterruptException:
                break

    def on_odom(self, message: Odometry) -> None:
        self.odom = message
        self.odom_time = time.monotonic()

    def on_scan(self, message: LaserScan) -> None:
        now = time.monotonic()
        try:
            transform = self.tf_buffer.lookup_transform(
                self.base_frame, message.header.frame_id, rospy.Time(0), rospy.Duration(0.05)
            )
            translation = transform.transform.translation
            rotation = transform.transform.rotation
            points, valid_count = transform_scan_points(
                message.ranges, message.angle_min, message.angle_increment,
                max(message.range_min, 0.0), message.range_max,
                translation.x, translation.y, yaw_from_quaternion(rotation)
            )
            points, self_filtered = filter_footprint_points(
                points, self.half_length, self.half_width
            )
            error = "" if valid_count >= self.min_valid_points else "insufficient_points"
        except (tf2_ros.LookupException, tf2_ros.ConnectivityException, tf2_ros.ExtrapolationException) as exc:
            points = []
            valid_count = 0
            self_filtered = 0
            error = "tf_unavailable"
            rospy.logwarn_throttle(2.0, "PetCargo lidar TF unavailable: %s", exc)
        self.scan_points = points
        self.scan_valid_count = valid_count
        self.scan_self_filtered = self_filtered
        self.scan_error = error
        self.scan_time = now
        self.scan_generation += 1

    def lidar_health(self):
        return lidar_health(self.scan_time, time.monotonic(), self.scan_error, self.lidar_timeout)

    def clearance(self, direction):
        return corridor_clearance(
            self.scan_points, direction, self.half_length, self.half_width, self.corridor_margin
        )

    def current_direction(self):
        if self.avoidance is not None:
            return self.avoidance["phase_direction"]
        if self.jog is not None:
            return self.jog["direction"]
        if self.active is not None:
            return self.active.get("direction")
        return None

    def publish_lidar_status(self) -> None:
        online, reason, age = self.lidar_health()
        direction = self.current_direction()
        clearance = self.clearance(direction) if online and direction else math.inf
        value = {
            "online": online,
            "reason": reason,
            "age_ms": None if age is None else round(age * 1000),
            "valid_points": self.scan_valid_count,
            "self_filtered_points": self.scan_self_filtered,
            "clearances_m": {
                DIRECTION_NAMES[item]: (
                    None if not online or not math.isfinite(self.clearance(item))
                    else round(self.clearance(item), 3)
                )
                for item in (1, 2, 3, 4)
            },
            "direction": DIRECTION_NAMES.get(direction),
            "clearance_m": None if not math.isfinite(clearance) else round(clearance, 3),
            "phase": self.avoidance["phase"] if self.avoidance else "track",
            "phase_label": PHASE_NAMES[self.avoidance["phase"]] if self.avoidance else "直行",
            "detour_side": DIRECTION_NAMES.get(self.avoidance["side_direction"]) if self.avoidance else None,
            "timestamp": time.time(),
        }
        self.lidar_pub.publish(json.dumps(value, ensure_ascii=False))

    def on_safety(self, message: Bool) -> None:
        self.safety_latched = bool(message.data)
        if self.safety_latched:
            self.command_pub.publish(Twist())
            if self.active is not None:
                self.finish(MotionResultCode.STOPPED)
            self.stop_jog("safety")

    def on_cancel(self, _message: Empty) -> None:
        self.command_pub.publish(Twist())
        if self.active is not None:
            self.finish(MotionResultCode.STOPPED)
        self.stop_jog("cancelled")

    def on_connected(self, message: Bool) -> None:
        if not message.data:
            self.command_pub.publish(Twist())
            if self.active is not None:
                self.finish(MotionResultCode.STOPPED)
            self.stop_jog("serial_disconnected")

    def publish_status(self, status: str, **extra) -> None:
        value = {"status": status, "timestamp": time.time()}
        value.update(extra)
        self.status_pub.publish(json.dumps(value, ensure_ascii=False))

    def publish_result(self, request: dict, code: MotionResultCode, actual_mm=0, actual_cdeg=0) -> None:
        value = {
            "motion_id": int(request["motion_id"]), "code": int(code),
            "code_name": code.name.lower(), "actual_mm": int(actual_mm),
            "actual_cdeg": int(actual_cdeg),
        }
        self.result_pub.publish(json.dumps(value))
        self.publish_status("idle", source=request.get("source", "voice"), last_result=value)

    def on_request(self, message: String) -> None:
        try:
            request = json.loads(message.data)
            request["motion_id"] = int(request["motion_id"])
            request["kind"] = int(request["kind"])
            request["distance_mm"] = int(request.get("distance_mm", 0))
            request["angle_cdeg"] = int(request.get("angle_cdeg", 0))
            request["max_speed"] = int(request.get("max_speed", 0))
            request["source"] = (
                "bright_light" if request.get("source") == "bright_light" else "voice"
            )
        except (KeyError, TypeError, ValueError, json.JSONDecodeError) as exc:
            rospy.logwarn("Invalid PetCargo motion request: %s", exc)
            return
        if request["motion_id"] in self.completed_ids:
            rospy.loginfo("Ignoring duplicate PetCargo motion id %d", request["motion_id"])
            return
        if self.safety_latched:
            self.publish_result(request, MotionResultCode.REJECTED_SAFETY)
            return
        if self.active is not None:
            self.publish_result(request, MotionResultCode.REJECTED_BUSY)
            return
        if self.odom is None or time.monotonic() - self.odom_time > 0.5:
            self.publish_result(request, MotionResultCode.ODOM_ERROR)
            return
        if request["kind"] not in (int(MotionKind.LINEAR), int(MotionKind.ROTATE), int(MotionKind.LATERAL)):
            self.publish_result(request, MotionResultCode.REJECTED_SAFETY)
            return
        if request["kind"] in (int(MotionKind.LINEAR), int(MotionKind.LATERAL)) and direction_from_request(request) is None:
            self.publish_result(request, MotionResultCode.REJECTED_SAFETY)
            return
        online, _, _ = self.lidar_health()
        if not online:
            self.publish_result(request, MotionResultCode.LIDAR_UNAVAILABLE)
            return
        if request["kind"] == int(MotionKind.ROTATE) and rotation_blocked(
            self.scan_points, self.half_length, self.half_width, self.stop_clearance
        ):
            self.publish_result(request, MotionResultCode.OBSTACLE_BLOCKED)
            return
        self.stop_jog("fixed_motion")
        pose = self.odom.pose.pose.position
        heading = yaw_from_odom(self.odom)
        direction = direction_from_request(request)
        self.active = {
            "request": request, "start_time": time.monotonic(), "start_x": pose.x,
            "start_y": pose.y, "heading": heading, "direction": direction,
            "target": abs(request["distance_mm"]) / 1000.0,
            "angle": AngleAccumulator(heading), "progress": 0.0,
            "detour_attempted": False, "obstacle_hits": 0, "last_scan_generation": -1,
        }
        self.publish_status("running", source=request["source"], phase="track", request=request)

    def on_jog(self, message: String) -> None:
        try:
            value = json.loads(message.data)
            direction = int(value["direction"])
            speed = min(0.30, max(0.06, int(value.get("speed_mm_s", 120)) / 1000.0))
            lease = min(0.5, max(0.1, int(value.get("lease_ms", 300)) / 1000.0))
            source = str(value.get("source", "stc_remote"))
            if source not in ("stc_remote", "windows_stc"):
                raise ValueError("invalid source")
        except (KeyError, TypeError, ValueError, json.JSONDecodeError) as exc:
            rospy.logwarn("Invalid PetCargo jog request: %s", exc)
            return
        if direction == 0:
            self.jog_blocked_key = None
            self.stop_jog("released")
            return
        if direction not in (1, 2, 3, 4) or self.safety_latched:
            return
        key = (source, direction)
        if self.jog_blocked_key == key:
            return
        if self.jog_blocked_key is not None and self.jog_blocked_key != key:
            self.jog_blocked_key = None
        if self.jog is not None and self.jog["direction"] == direction and self.jog["source"] == source:
            online, _, _ = self.lidar_health()
            if not online:
                self.stop_jog("lidar_unavailable")
                return
            self.jog["speed"] = speed
            self.jog["deadline"] = time.monotonic() + lease
            return
        if self.jog is not None:
            self.stop_jog("direction_changed")
        if self.active is not None:
            self.finish(MotionResultCode.STOPPED)
        self.command_pub.publish(Twist())
        online, _, _ = self.lidar_health()
        if not online:
            self.publish_status("idle", source=source, reason="lidar_unavailable")
            return
        self.jog = {
            "direction": direction, "direction_name": DIRECTION_NAMES[direction],
            "speed": speed, "deadline": time.monotonic() + lease, "source": source,
            "detour_attempted": False, "obstacle_hits": 0, "last_scan_generation": -1,
        }
        self.publish_status(
            "running", source=source, direction=DIRECTION_NAMES[direction],
            phase="track", lease_ms=round(lease * 1000)
        )

    def stop_jog(self, reason: str, block=False) -> None:
        if self.jog is None:
            if self.avoidance is not None and self.avoidance["owner"] == "jog":
                self.avoidance = None
                self.command_pub.publish(Twist())
            return
        source = self.jog.get("source", "stc_remote")
        if block:
            self.jog_blocked_key = (source, self.jog["direction"])
        self.jog = None
        if self.avoidance is not None and self.avoidance["owner"] == "jog":
            self.avoidance = None
        self.command_pub.publish(Twist())
        self.publish_status("idle", source=source, reason=reason)

    def translation_progress(self, context):
        pose = self.odom.pose.pose.position
        return local_progress(
            context["start_x"], context["start_y"], context["heading"],
            context["direction"], pose.x, pose.y
        )

    def finish(self, code: MotionResultCode) -> None:
        if self.active is None:
            return
        self.command_pub.publish(Twist())
        request = self.active["request"]
        progress = self.active["progress"]
        if self.avoidance is not None and self.avoidance["owner"] == "active":
            self.avoidance = None
        if request["kind"] in (int(MotionKind.LINEAR), int(MotionKind.LATERAL)):
            sign = 1 if request["distance_mm"] >= 0 else -1
            actual_mm, actual_cdeg = round(progress * sign * 1000.0), 0
        else:
            actual_mm, actual_cdeg = 0, round(math.degrees(progress) * 100.0)
        self.completed_ids.add(request["motion_id"])
        if len(self.completed_ids) > 128:
            self.completed_ids.clear()
            self.completed_ids.add(request["motion_id"])
        self.active = None
        self.publish_result(request, code, actual_mm, actual_cdeg)

    def obstacle_triggered(self, context, clearance):
        triggered, hits, generation = update_obstacle_confirmation(
            context["obstacle_hits"], context["last_scan_generation"],
            self.scan_generation, clearance, self.stop_clearance,
            self.emergency_clearance, self.confirm_scans
        )
        context["obstacle_hits"] = hits
        context["last_scan_generation"] = generation
        return triggered

    def begin_avoidance(self, owner, context, direction):
        selected, left_clearance, right_clearance = choose_detour_path(
            self.scan_points, direction, self.stop_clearance,
            self.shift_distance, self.pass_distance,
            self.half_length, self.half_width, self.corridor_margin
        )
        context["detour_attempted"] = True
        if selected is None:
            rospy.logwarn("PetCargo obstacle boxed in: left=%s right=%s", left_clearance, right_clearance)
            return False
        pose = self.odom.pose.pose.position
        self.command_pub.publish(Twist())
        self.avoidance = {
            "owner": owner, "original_direction": direction, "side_direction": selected,
            "phase": "shift_out", "phase_direction": selected,
            "phase_target": self.shift_distance, "start_x": pose.x, "start_y": pose.y,
            "heading": yaw_from_odom(self.odom),
            "start_time": context.get("start_time", time.monotonic()),
        }
        self.publish_avoidance_status()
        return True

    def publish_avoidance_status(self):
        if self.avoidance is None:
            return
        context = self.active if self.avoidance["owner"] == "active" else self.jog
        if context is None:
            return
        source = context["request"]["source"] if self.avoidance["owner"] == "active" else context["source"]
        self.publish_status(
            "running", source=source,
            direction=DIRECTION_NAMES[self.avoidance["original_direction"]],
            phase=self.avoidance["phase"],
            detour_side=DIRECTION_NAMES[self.avoidance["side_direction"]]
        )

    def abort_avoidance(self, code, reason):
        owner = self.avoidance["owner"] if self.avoidance else None
        self.command_pub.publish(Twist())
        self.avoidance = None
        if owner == "active" and self.active is not None:
            self.finish(code)
        elif owner == "jog":
            self.stop_jog(reason, block=(code == MotionResultCode.OBSTACLE_BLOCKED))

    def transition_avoidance(self):
        avoid = self.avoidance
        self.command_pub.publish(Twist())
        next_phase = next_detour_phase(
            avoid["phase"], avoid["original_direction"], avoid["side_direction"],
            self.shift_distance, self.pass_distance
        )
        if next_phase is None:
            owner = avoid["owner"]
            self.avoidance = None
            if owner == "active" and self.active is not None:
                self.active["progress"] = self.translation_progress(self.active)
                if detour_target_satisfied(self.active["progress"], self.active["target"]):
                    self.finish(MotionResultCode.COMPLETED)
                else:
                    self.publish_status(
                        "running", source=self.active["request"]["source"],
                        phase="track", request=self.active["request"]
                    )
            elif owner == "jog" and self.jog is not None:
                self.publish_status(
                    "running", source=self.jog["source"],
                    direction=self.jog["direction_name"], phase="track"
                )
            return
        avoid["phase"], avoid["phase_direction"], avoid["phase_target"] = next_phase
        pose = self.odom.pose.pose.position
        avoid["start_x"] = pose.x
        avoid["start_y"] = pose.y
        avoid["heading"] = yaw_from_odom(self.odom)
        self.publish_avoidance_status()

    def run_avoidance(self, now):
        avoid = self.avoidance
        context = self.active if avoid["owner"] == "active" else self.jog
        if context is None:
            self.avoidance = None
            self.command_pub.publish(Twist())
            return
        if avoid["owner"] == "jog" and now >= context["deadline"]:
            self.stop_jog("lease_expired")
            return
        if now - avoid["start_time"] > self.linear_timeout:
            self.abort_avoidance(MotionResultCode.TIMEOUT, "avoidance_timeout")
            return
        online, _, _ = self.lidar_health()
        if not online:
            self.abort_avoidance(MotionResultCode.LIDAR_UNAVAILABLE, "lidar_unavailable")
            return
        clearance = self.clearance(avoid["phase_direction"])
        if clearance <= self.stop_clearance:
            self.abort_avoidance(MotionResultCode.OBSTACLE_BLOCKED, "obstacle_blocked")
            return
        pose = self.odom.pose.pose.position
        progress = local_progress(
            avoid["start_x"], avoid["start_y"], avoid["heading"],
            avoid["phase_direction"], pose.x, pose.y
        )
        speed = linear_speed_for_error(avoid["phase_target"] - progress, self.avoid_speed)
        if speed == 0.0:
            self.transition_avoidance()
            return
        speed = speed_for_clearance(speed, clearance, self.slow_clearance, self.stop_clearance, 0.08)
        self.command_pub.publish(command_for_direction(avoid["phase_direction"], speed))

    def run_jog(self, now):
        if now >= self.jog["deadline"]:
            self.stop_jog("lease_expired")
            return
        online, _, _ = self.lidar_health()
        if not online:
            self.stop_jog("lidar_unavailable")
            return
        direction = self.jog["direction"]
        clearance = self.clearance(direction)
        if self.obstacle_triggered(self.jog, clearance):
            if self.jog["detour_attempted"] or not self.begin_avoidance("jog", self.jog, direction):
                self.stop_jog("obstacle_blocked", block=True)
            return
        speed = speed_for_clearance(
            self.jog["speed"], clearance, self.slow_clearance, self.stop_clearance, 0.08
        )
        self.command_pub.publish(command_for_direction(direction, speed))

    def run_active(self, now):
        request = self.active["request"]
        timeout = self.angular_timeout if request["kind"] == int(MotionKind.ROTATE) else self.linear_timeout
        if now - self.active["start_time"] > timeout:
            self.finish(MotionResultCode.TIMEOUT)
            return
        online, _, _ = self.lidar_health()
        if not online:
            self.finish(MotionResultCode.LIDAR_UNAVAILABLE)
            return
        if request["kind"] == int(MotionKind.ROTATE):
            if rotation_blocked(self.scan_points, self.half_length, self.half_width, self.stop_clearance):
                self.finish(MotionResultCode.OBSTACLE_BLOCKED)
                return
            progress = self.active["angle"].update(yaw_from_odom(self.odom))
            target = math.radians(request["angle_cdeg"] / 100.0)
            maximum = min(0.5, max(0.2, request["max_speed"] / 1000.0))
            command = Twist()
            command.angular.z = angular_speed_for_error(target - progress, maximum)
            self.active["progress"] = progress
            if command.angular.z == 0.0:
                self.finish(MotionResultCode.COMPLETED)
            else:
                self.command_pub.publish(command)
            return
        direction = self.active["direction"]
        progress = self.translation_progress(self.active)
        self.active["progress"] = progress
        speed = linear_speed_for_error(
            self.active["target"] - progress,
            min(0.18, max(0.06, request["max_speed"] / 1000.0))
        )
        if speed == 0.0:
            self.finish(MotionResultCode.COMPLETED)
            return
        clearance = self.clearance(direction)
        if self.obstacle_triggered(self.active, clearance):
            if request["source"] == "bright_light":
                self.finish(MotionResultCode.OBSTACLE_BLOCKED)
            elif self.active["detour_attempted"] or not self.begin_avoidance("active", self.active, direction):
                self.finish(MotionResultCode.OBSTACLE_BLOCKED)
            return
        speed = speed_for_clearance(speed, clearance, self.slow_clearance, self.stop_clearance, 0.08)
        self.command_pub.publish(command_for_direction(direction, speed))

    def on_timer(self, _event) -> None:
        now = time.monotonic()
        if now >= self.next_lidar_status:
            self.next_lidar_status = now + 0.2
            self.publish_lidar_status()
        if self.safety_latched:
            self.command_pub.publish(Twist())
            return
        if self.odom is None or now - self.odom_time > 0.5:
            if self.active is not None:
                self.finish(MotionResultCode.ODOM_ERROR)
            self.stop_jog("odom_error")
            return
        if self.avoidance is not None:
            self.run_avoidance(now)
        elif self.jog is not None:
            self.run_jog(now)
        elif self.active is not None:
            self.run_active(now)


if __name__ == "__main__":
    rospy.init_node("motion_executor")
    MotionExecutor().run()
