#!/usr/bin/env python3
"""Generate valid PetCargo frames for host-side serial/ROS integration tests."""

import argparse
import os
import struct
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "ros", "petcargo_ros", "src"))

from petcargo_ros.protocol import Frame, MessageType, MotionKind, MotionRequest, Telemetry, encode_frame


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", help="Serial port connected to serial_bridge; omit to print hex")
    parser.add_argument("--interval", type=float, default=0.2)
    parser.add_argument("--escape-after", type=float, default=2.0)
    args = parser.parse_args()
    stream = None
    if args.port:
        import serial
        stream = serial.Serial(args.port, 115200, timeout=0.05)

    start = time.monotonic(); seq = 0; escaped = False
    try:
        while True:
            elapsed = time.monotonic() - start; seq = (seq + 1) & 0xFF
            bright = elapsed >= args.escape_after
            telemetry = Telemetry(
                int(elapsed * 1000), 32 if bright else 8, 263,
                0, 0, 1000, 0, 0, 50, 72 if bright else 5, 0, 0,
            )
            packets = [encode_frame(Frame(MessageType.TELEMETRY, seq, telemetry.pack()))]
            if bright and not escaped:
                escaped = True; seq = (seq + 1) & 0xFF
                packets.append(encode_frame(Frame(MessageType.EVENT, seq, struct.pack("<Bh", 1, 32))))
                seq = (seq + 1) & 0xFF
                request = MotionRequest(1, MotionKind.LINEAR, -500, 0, 180)
                packets.append(encode_frame(Frame(MessageType.MOTION_REQUEST, seq, request.pack())))
            for packet in packets:
                if stream: stream.write(packet)
                else: print(packet.hex(" "))
            time.sleep(args.interval)
    except KeyboardInterrupt:
        pass
    finally:
        if stream: stream.close()


if __name__ == "__main__":
    main()
