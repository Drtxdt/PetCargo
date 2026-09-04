import json
import os
import random
import struct
import sys
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "ros", "petcargo_ros", "src"))

from petcargo_ros.protocol import (  # noqa: E402
    EventCode,
    Frame,
    FrameParser,
    JogRequest,
    MessageType,
    MotionRequest,
    MotionSourceTracker,
    Telemetry,
    crc16_ccitt,
    encode_frame,
)


class ProtocolTests(unittest.TestCase):
    def test_bright_light_source_is_single_use_and_expires(self):
        tracker = MotionSourceTracker(window_seconds=1.0)
        self.assertEqual(tracker.consume(0.0), "voice")
        tracker.note_event(EventCode.BRIGHT_LIGHT, 10.0)
        self.assertEqual(tracker.consume(10.5), "bright_light")
        self.assertEqual(tracker.consume(10.6), "voice")
        tracker.note_event(EventCode.BRIGHT_LIGHT, 20.0)
        self.assertEqual(tracker.consume(21.1), "voice")

    def test_known_crc_vector(self):
        self.assertEqual(crc16_ccitt(b"123456789"), 0x29B1)

    def test_fragmented_and_noisy_stream(self):
        expected = Frame(MessageType.HEARTBEAT, 7, struct.pack("<I", 12345))
        encoded = b"noise" + encode_frame(expected)
        parser = FrameParser()
        found = []
        for byte in encoded:
            found.extend(parser.feed(bytes((byte,))))
        self.assertEqual(found, [expected])

    def test_crc_failure_resynchronizes(self):
        bad = bytearray(encode_frame(Frame(MessageType.HELLO, 1, b"x")))
        bad[-1] ^= 0x80
        good = Frame(MessageType.STOP, 2, b"\x01")
        parser = FrameParser()
        self.assertEqual(parser.feed(bytes(bad) + encode_frame(good)), [good])
        self.assertEqual(parser.crc_errors, 1)

    def test_motion_request_round_trip(self):
        value = MotionRequest(42, 2, 0, 36000, 500)
        self.assertEqual(MotionRequest.unpack(value.pack()), value)

    def test_telemetry_round_trip(self):
        value = Telemetry(1000, 23, 263, 2, -10, 1001, 0, 1, 88, 9, 0, 5)
        self.assertEqual(Telemetry.unpack(value.pack()), value)

    def test_jog_request_round_trip(self):
        value = JogRequest(3, 120, 300)
        self.assertEqual(JogRequest.unpack(value.pack()), value)

    def test_random_chunking(self):
        frames = [Frame(MessageType.EVENT, n, bytes((n, 0, 0, 0))) for n in range(20)]
        stream = b"".join(encode_frame(frame) for frame in frames)
        parser = FrameParser(); decoded=[]; offset=0; random.seed(9)
        while offset < len(stream):
            size=random.randint(1,11); decoded.extend(parser.feed(stream[offset:offset+size])); offset+=size
        self.assertEqual(decoded, frames)


if __name__ == "__main__":
    unittest.main()
