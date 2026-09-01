#!/usr/bin/env python3
"""Bridge the STC USB serial protocol to small JSON ROS topics."""

import json
import struct
import threading
import time

import rospy
import serial
from std_msgs.msg import Bool, Empty, String

from petcargo_ros.protocol import (
    EventCode,
    Frame,
    FrameParser,
    JogRequest,
    MessageType,
    MotionRequest,
    MotionResult,
    Telemetry,
    encode_frame,
    telemetry_to_dict,
)


EVENT_NAMES = {
    int(EventCode.BRIGHT_LIGHT): "bright_light",
    int(EventCode.SHAKE): "shake",
    int(EventCode.FEED): "feed",
    int(EventCode.TEMPERATURE): "temperature",
    int(EventCode.VOICE): "voice",
    int(EventCode.BUTTON): "button",
    int(EventCode.FAULT): "fault",
    int(EventCode.REMOTE): "remote",
}


class SerialBridge:
    def __init__(self) -> None:
        self.port_name = rospy.get_param("~serial_port", "/dev/petcargo_stc")
        self.baud = int(rospy.get_param("~serial_baud", 9600))
        self.link_timeout = float(rospy.get_param("~serial_timeout", 1.5))
        self.parser = FrameParser()
        self.serial = None
        self.serial_lock = threading.RLock()
        self.seq = 0
        self.last_rx = 0.0
        self.connected = False
        self.pending_results = []

        self.telemetry_pub = rospy.Publisher("/petcargo/telemetry", String, queue_size=10)
        self.event_pub = rospy.Publisher("/petcargo/events", String, queue_size=30)
        self.motion_pub = rospy.Publisher("/petcargo/motion_request", String, queue_size=10)
        self.jog_pub = rospy.Publisher("/petcargo/jog_request", String, queue_size=10)
        self.connected_pub = rospy.Publisher(
            "/petcargo/serial_connected", Bool, queue_size=1, latch=True
        )
        self.safety_pub = rospy.Publisher("/petcargo/safety_set", Bool, queue_size=4)
        self.cancel_pub = rospy.Publisher("/petcargo/cancel_motion", Empty, queue_size=4)

        rospy.Subscriber("/petcargo/motion_result", String, self.on_motion_result, queue_size=10)
        rospy.Subscriber("/petcargo/safety_set", Bool, self.on_safety_set, queue_size=4)
        self.connected_pub.publish(False)

    def next_seq(self) -> int:
        self.seq = (self.seq + 1) & 0xFF
        return self.seq

    def send(self, msg_type: int, payload: bytes = b"") -> bool:
        packet = encode_frame(Frame(msg_type, self.next_seq(), payload))
        with self.serial_lock:
            if self.serial is None or not self.serial.is_open:
                return False
            try:
                self.serial.write(packet)
                return True
            except serial.SerialException as exc:
                rospy.logwarn_throttle(2.0, "PetCargo serial write failed: %s", exc)
                self.close_serial()
                return False

    def set_connected(self, value: bool) -> None:
        if value == self.connected:
            return
        self.connected = value
        self.connected_pub.publish(value)
        rospy.loginfo("PetCargo STC link %s", "connected" if value else "disconnected")

    def close_serial(self) -> None:
        with self.serial_lock:
            current = self.serial
            self.serial = None
            if current is not None:
                try:
                    current.close()
                except serial.SerialException:
                    pass
        self.set_connected(False)

    def open_serial(self) -> bool:
        try:
            device = serial.Serial(self.port_name, self.baud, timeout=0.05, write_timeout=0.2)
        except serial.SerialException as exc:
            rospy.logwarn_throttle(3.0, "Waiting for %s: %s", self.port_name, exc)
            return False
        with self.serial_lock:
            self.serial = device
        self.parser = FrameParser()
        self.last_rx = time.monotonic()
        self.send(MessageType.HELLO, struct.pack("<H", 0x0002))
        return True

    def on_motion_result(self, message: String) -> None:
        try:
            value = json.loads(message.data)
            result = MotionResult(
                int(value["motion_id"]),
                int(value["code"]),
                int(value.get("actual_mm", 0)),
                int(value.get("actual_cdeg", 0)),
            )
            payload = result.pack()
            if not self.send(MessageType.MOTION_RESULT, payload):
                self.pending_results = (self.pending_results + [payload])[-8:]
        except (KeyError, TypeError, ValueError, json.JSONDecodeError) as exc:
            rospy.logwarn("Invalid motion result: %s", exc)

    def on_safety_set(self, message: Bool) -> None:
        self.send(MessageType.STOP, bytes((1 if message.data else 0,)))

    def handle_frame(self, frame: Frame) -> None:
        if frame.version != 2:
            rospy.logwarn_throttle(5.0, "Unsupported STC protocol version %d", frame.version)
            return
        self.last_rx = time.monotonic()
        self.set_connected(True)
        if self.pending_results:
            pending = self.pending_results
            self.pending_results = []
            for payload in pending:
                if not self.send(MessageType.MOTION_RESULT, payload):
                    self.pending_results.append(payload)
                    break
        try:
            kind = MessageType(frame.msg_type)
        except ValueError:
            self.send(MessageType.NACK, bytes((frame.msg_type, frame.seq, 1)))
            return

        try:
            if kind == MessageType.TELEMETRY:
                value = telemetry_to_dict(Telemetry.unpack(frame.payload))
                value["received_at"] = time.time()
                self.telemetry_pub.publish(json.dumps(value, ensure_ascii=False))
            elif kind == MessageType.EVENT:
                if len(frame.payload) != 3:
                    raise ValueError("event payload must be 3 bytes")
                event_code, value = struct.unpack("<Bh", frame.payload)
                event = {
                    "code": event_code,
                    "name": EVENT_NAMES.get(event_code, "unknown"),
                    "value": value,
                    "timestamp": time.time(),
                }
                self.event_pub.publish(json.dumps(event, ensure_ascii=False))
            elif kind == MessageType.MOTION_REQUEST:
                request = MotionRequest.unpack(frame.payload)
                self.motion_pub.publish(json.dumps(request.__dict__))
            elif kind == MessageType.JOG_REQUEST:
                request = JogRequest.unpack(frame.payload)
                self.jog_pub.publish(json.dumps(request.__dict__))
            elif kind == MessageType.STOP:
                stop_code = frame.payload[0] if frame.payload else 1
                if stop_code == 2:
                    self.cancel_pub.publish()
                else:
                    self.safety_pub.publish(bool(stop_code))
            elif kind in (MessageType.HELLO, MessageType.HEARTBEAT):
                self.send(MessageType.ACK, bytes((frame.msg_type, frame.seq, 0)))
        except (ValueError, struct.error) as exc:
            rospy.logwarn("Dropped malformed STC frame type 0x%02X: %s", frame.msg_type, exc)
            self.send(MessageType.NACK, bytes((frame.msg_type, frame.seq, 2)))

    def run(self) -> None:
        next_heartbeat = 0.0
        while not rospy.is_shutdown():
            if self.serial is None and not self.open_serial():
                rospy.sleep(0.5)
                continue
            try:
                with self.serial_lock:
                    current = self.serial
                    data = current.read(current.in_waiting or 1) if current is not None else b""
                for frame in self.parser.feed(data):
                    self.handle_frame(frame)
            except serial.SerialException as exc:
                rospy.logwarn("PetCargo serial read failed: %s", exc)
                self.close_serial()
                continue

            now = time.monotonic()
            if now >= next_heartbeat:
                self.send(MessageType.HEARTBEAT, struct.pack("<I", int(now * 1000) & 0xFFFFFFFF))
                next_heartbeat = now + 0.5
            if self.connected and now - self.last_rx > self.link_timeout:
                self.set_connected(False)
        self.close_serial()


if __name__ == "__main__":
    rospy.init_node("serial_bridge")
    SerialBridge().run()
