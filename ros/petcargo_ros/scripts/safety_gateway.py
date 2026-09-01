#!/usr/bin/env python3
"""Lease-based sole publisher from PetCargo to the uCar /cmd_vel topic."""

import json
import time

import rospy
from geometry_msgs.msg import Twist
from std_msgs.msg import Bool, String


class SafetyGateway:
    def __init__(self) -> None:
        self.lease = float(rospy.get_param("~velocity_lease", 0.25))
        self.command = Twist()
        self.command_time = 0.0
        self.serial_connected = False
        self.latched = False
        self.reason = "startup"
        self.published_reason = None

        self.output_pub = rospy.Publisher(
            rospy.get_param("~output_cmd_topic", "/cmd_vel"), Twist, queue_size=1
        )
        self.state_pub = rospy.Publisher("/petcargo/safety_state", String, queue_size=4, latch=True)
        rospy.Subscriber(
            rospy.get_param("~internal_cmd_topic", "/petcargo/cmd_vel_request"),
            Twist,
            self.on_command,
            queue_size=1,
        )
        rospy.Subscriber("/petcargo/serial_connected", Bool, self.on_connected, queue_size=2)
        rospy.Subscriber("/petcargo/safety_set", Bool, self.on_safety, queue_size=4)
        self.timer = rospy.Timer(rospy.Duration(0.02), self.on_timer)
        self.publish_state()

    @staticmethod
    def clamp(value: float, limit: float) -> float:
        return max(-limit, min(limit, value))

    def on_command(self, message: Twist) -> None:
        safe = Twist()
        safe.linear.x = self.clamp(message.linear.x, 0.18)
        safe.linear.y = self.clamp(message.linear.y, 0.18)
        safe.angular.z = self.clamp(message.angular.z, 0.5)
        self.command = safe
        self.command_time = time.monotonic()

    def on_connected(self, message: Bool) -> None:
        self.serial_connected = bool(message.data)
        if not self.serial_connected:
            self.reason = "serial_disconnected"
            self.output_pub.publish(Twist())
        self.publish_state()

    def on_safety(self, message: Bool) -> None:
        self.latched = bool(message.data)
        self.reason = "emergency_stop" if self.latched else "ready"
        self.output_pub.publish(Twist())
        self.publish_state()

    def publish_state(self) -> None:
        self.published_reason = self.reason
        self.state_pub.publish(
            json.dumps(
                {
                    "latched": self.latched,
                    "serial_connected": self.serial_connected,
                    "reason": self.reason,
                    "timestamp": time.time(),
                }
            )
        )

    def on_timer(self, _event) -> None:
        now = time.monotonic()
        if self.latched:
            self.reason = "emergency_stop"
            output = Twist()
        elif not self.serial_connected:
            self.reason = "serial_disconnected"
            output = Twist()
        elif now - self.command_time > self.lease:
            self.reason = "velocity_lease_expired"
            output = Twist()
        else:
            self.reason = "moving"
            output = self.command
        self.output_pub.publish(output)
        if self.reason != self.published_reason:
            self.publish_state()


if __name__ == "__main__":
    rospy.init_node("safety_gateway")
    SafetyGateway()
    rospy.spin()
