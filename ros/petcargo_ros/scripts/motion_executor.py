#!/usr/bin/env python3
"""Execute relative linear and rotational PetCargo motions from odometry."""

import json
import math
import time

import rospy
from geometry_msgs.msg import Twist
from nav_msgs.msg import Odometry
from std_msgs.msg import Bool, Empty, String

from petcargo_ros.motion import (
    AngleAccumulator,
    angular_speed_for_error,
    linear_speed_for_error,
    project_along_heading,
    project_lateral_to_heading,
)
from petcargo_ros.protocol import MotionKind, MotionResultCode


def yaw_from_odom(message: Odometry) -> float:
    q = message.pose.pose.orientation
    return math.atan2(2.0 * (q.w * q.z + q.x * q.y), 1.0 - 2.0 * (q.y * q.y + q.z * q.z))


class MotionExecutor:
    def __init__(self) -> None:
        self.command_pub = rospy.Publisher(
            rospy.get_param("~internal_cmd_topic", "/petcargo/cmd_vel_request"),
            Twist,
            queue_size=1,
        )
        self.result_pub = rospy.Publisher("/petcargo/motion_result", String, queue_size=10)
        self.status_pub = rospy.Publisher("/petcargo/motion_status", String, queue_size=10, latch=True)
        rospy.Subscriber(
            rospy.get_param("~odom_topic", "/odom"), Odometry, self.on_odom, queue_size=10
        )
        rospy.Subscriber("/petcargo/motion_request", String, self.on_request, queue_size=10)
        rospy.Subscriber("/petcargo/jog_request", String, self.on_jog, queue_size=10)
        rospy.Subscriber("/petcargo/safety_set", Bool, self.on_safety, queue_size=4)
        rospy.Subscriber("/petcargo/cancel_motion", Empty, self.on_cancel, queue_size=4)
        rospy.Subscriber("/petcargo/serial_connected", Bool, self.on_connected, queue_size=4)

        self.linear_timeout = float(rospy.get_param("~linear_timeout", 7.0))
        self.angular_timeout = float(rospy.get_param("~angular_timeout", 15.0))
        self.odom = None
        self.odom_time = 0.0
        self.active = None
        self.jog = None
        self.safety_latched = False
        self.completed_ids = set()
        self.timer = rospy.Timer(rospy.Duration(0.05), self.on_timer)
        self.publish_status("idle")

    def on_odom(self, message: Odometry) -> None:
        self.odom = message
        self.odom_time = time.monotonic()

    def on_safety(self, message: Bool) -> None:
        self.safety_latched = bool(message.data)
        if self.safety_latched:
            if self.active is not None:
                self.finish(MotionResultCode.STOPPED)
            self.stop_jog("safety")

    def on_cancel(self, _message: Empty) -> None:
        if self.active is not None:
            self.finish(MotionResultCode.STOPPED)
        self.stop_jog("cancelled")

    def on_connected(self, message: Bool) -> None:
        if not message.data:
            if self.active is not None:
                self.finish(MotionResultCode.STOPPED)
            self.stop_jog("serial_disconnected")

    def publish_status(self, status: str, **extra) -> None:
        value = {"status": status, "timestamp": time.time()}
        value.update(extra)
        self.status_pub.publish(json.dumps(value))

    def publish_result(self, request: dict, code: MotionResultCode, actual_mm=0, actual_cdeg=0) -> None:
        value = {
            "motion_id": int(request["motion_id"]),
            "code": int(code),
            "code_name": code.name.lower(),
            "actual_mm": int(actual_mm),
            "actual_cdeg": int(actual_cdeg),
        }
        self.result_pub.publish(json.dumps(value))
        self.publish_status("idle", last_result=value)

    def on_request(self, message: String) -> None:
        try:
            request = json.loads(message.data)
            request["motion_id"] = int(request["motion_id"])
            request["kind"] = int(request["kind"])
            request["distance_mm"] = int(request.get("distance_mm", 0))
            request["angle_cdeg"] = int(request.get("angle_cdeg", 0))
            request["max_speed"] = int(request.get("max_speed", 0))
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
        if request["kind"] not in (
            int(MotionKind.LINEAR), int(MotionKind.ROTATE), int(MotionKind.LATERAL)
        ):
            self.publish_result(request, MotionResultCode.REJECTED_SAFETY)
            return

        self.stop_jog("fixed_motion")
        pose = self.odom.pose.pose.position
        yaw = yaw_from_odom(self.odom)
        self.active = {
            "request": request,
            "start_time": time.monotonic(),
            "start_x": pose.x,
            "start_y": pose.y,
            "start_yaw": yaw,
            "angle": AngleAccumulator(yaw),
            "progress": 0.0,
        }
        self.publish_status("running", source="voice_or_light", request=request)

    def on_jog(self, message: String) -> None:
        try:
            value = json.loads(message.data)
            direction = int(value["direction"])
            speed = min(0.18, max(0.06, int(value.get("speed_mm_s", 120)) / 1000.0))
            lease = min(0.5, max(0.1, int(value.get("lease_ms", 300)) / 1000.0))
            source = str(value.get("source", "stc_remote"))
            if source not in ("stc_remote", "windows_stc"):
                raise ValueError("invalid source")
        except (KeyError, TypeError, ValueError, json.JSONDecodeError) as exc:
            rospy.logwarn("Invalid PetCargo jog request: %s", exc)
            return
        if direction == 0:
            self.stop_jog("released")
            return
        if direction not in (1, 2, 3, 4) or self.safety_latched:
            return
        if self.active is not None:
            self.finish(MotionResultCode.STOPPED)
        names = {1: "forward", 2: "backward", 3: "left", 4: "right"}
        self.jog = {
            "direction": direction,
            "direction_name": names[direction],
            "speed": speed,
            "deadline": time.monotonic() + lease,
            "source": source,
        }
        self.publish_status("running", source=source, direction=names[direction], lease_ms=round(lease * 1000))

    def stop_jog(self, reason: str) -> None:
        if self.jog is None:
            return
        source = self.jog.get("source", "stc_remote")
        self.jog = None
        self.command_pub.publish(Twist())
        self.publish_status("idle", source=source, reason=reason)

    def finish(self, code: MotionResultCode) -> None:
        if self.active is None:
            return
        self.command_pub.publish(Twist())
        request = self.active["request"]
        progress = self.active["progress"]
        if request["kind"] in (int(MotionKind.LINEAR), int(MotionKind.LATERAL)):
            actual_mm, actual_cdeg = round(progress * 1000.0), 0
        else:
            actual_mm, actual_cdeg = 0, round(math.degrees(progress) * 100.0)
        self.completed_ids.add(request["motion_id"])
        if len(self.completed_ids) > 128:
            self.completed_ids.clear()
            self.completed_ids.add(request["motion_id"])
        self.active = None
        self.publish_result(request, code, actual_mm, actual_cdeg)

    def on_timer(self, _event) -> None:
        if self.jog is not None:
            now = time.monotonic()
            if self.safety_latched or now >= self.jog["deadline"]:
                self.stop_jog("safety" if self.safety_latched else "lease_expired")
                return
            if self.odom is None or now - self.odom_time > 0.5:
                self.stop_jog("odom_error")
                return
            command = Twist()
            direction = self.jog["direction"]
            speed = self.jog["speed"]
            if direction == 1:
                command.linear.x = speed
            elif direction == 2:
                command.linear.x = -speed
            elif direction == 3:
                command.linear.y = speed
            else:
                command.linear.y = -speed
            self.command_pub.publish(command)
            return
        if self.active is None:
            return
        if self.safety_latched:
            self.finish(MotionResultCode.STOPPED)
            return
        now = time.monotonic()
        if self.odom is None or now - self.odom_time > 0.5:
            self.finish(MotionResultCode.ODOM_ERROR)
            return

        request = self.active["request"]
        command = Twist()
        if request["kind"] in (int(MotionKind.LINEAR), int(MotionKind.LATERAL)):
            timeout = self.linear_timeout
            pose = self.odom.pose.pose.position
            projector = (
                project_lateral_to_heading
                if request["kind"] == int(MotionKind.LATERAL)
                else project_along_heading
            )
            progress = projector(
                self.active["start_x"], self.active["start_y"],
                self.active["start_yaw"], pose.x, pose.y
            )
            target = request["distance_mm"] / 1000.0
            error = target - progress
            maximum = min(0.18, max(0.06, request["max_speed"] / 1000.0))
            speed = linear_speed_for_error(error, maximum)
            if request["kind"] == int(MotionKind.LATERAL):
                command.linear.y = speed
            else:
                command.linear.x = speed
        else:
            timeout = self.angular_timeout
            progress = self.active["angle"].update(yaw_from_odom(self.odom))
            target = math.radians(request["angle_cdeg"] / 100.0)
            error = target - progress
            maximum = min(0.5, max(0.2, request["max_speed"] / 1000.0))
            command.angular.z = angular_speed_for_error(error, maximum)

        self.active["progress"] = progress
        if (request["kind"] == int(MotionKind.LINEAR) and command.linear.x == 0.0) or (
            request["kind"] == int(MotionKind.LATERAL) and command.linear.y == 0.0
        ) or (
            request["kind"] == int(MotionKind.ROTATE) and command.angular.z == 0.0
        ):
            self.finish(MotionResultCode.COMPLETED)
        elif now - self.active["start_time"] > timeout:
            self.finish(MotionResultCode.TIMEOUT)
        else:
            self.command_pub.publish(command)


if __name__ == "__main__":
    rospy.init_node("motion_executor")
    MotionExecutor()
    rospy.spin()
