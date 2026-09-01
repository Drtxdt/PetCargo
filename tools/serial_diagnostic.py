#!/usr/bin/env python3
"""Decode PetCargo serial frames without ROS."""

import argparse
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "ros", "petcargo_ros", "src"))

from petcargo_ros.protocol import FrameParser, MessageType, Telemetry, telemetry_to_dict


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("port")
    parser.add_argument("--baud", type=int, default=115200)
    args = parser.parse_args()
    import serial
    decoder = FrameParser()
    with serial.Serial(args.port, args.baud, timeout=0.2) as stream:
        while True:
            for frame in decoder.feed(stream.read(stream.in_waiting or 1)):
                try: name = MessageType(frame.msg_type).name
                except ValueError: name = f"0x{frame.msg_type:02X}"
                if frame.msg_type == MessageType.TELEMETRY:
                    print(name, telemetry_to_dict(Telemetry.unpack(frame.payload)))
                else:
                    print(name, "seq=", frame.seq, "payload=", frame.payload.hex(" "))


if __name__ == "__main__":
    main()

