import os
import sys
import threading
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "tools"))
sys.path.insert(0, os.path.join(ROOT, "ros", "petcargo_ros", "src"))

from remote_protocol import RemoteParser, encode  # noqa: E402
import remote_bridge  # noqa: E402
from petcargo_ros.remote_api import normalize_jog  # noqa: E402


class RemoteProtocolTests(unittest.TestCase):
    def test_fragmentation_noise_and_all_commands(self):
        parser = RemoteParser()
        stream = b"noise" + b"".join(encode(i, i + 20) for i in range(6))
        result = []
        for byte in stream:
            result.extend(parser.feed(bytes((byte,))))
        self.assertEqual([x["direction"] for x in result], list(range(6)))
        self.assertEqual([x["sequence"] for x in result], list(range(20, 26)))

    def test_corruption_resynchronizes(self):
        parser = RemoteParser()
        broken = bytearray(encode(3, 1))
        broken[-1] ^= 1
        result = parser.feed(bytes(broken) + encode(4, 2))
        self.assertEqual(result, [{"direction": 4, "sequence": 2}])
        self.assertGreater(parser.errors, 0)

    def test_api_validation(self):
        self.assertEqual(normalize_jog({"direction": 2}),
                         {"direction": 2, "speed_mm_s": 120, "lease_ms": 300,
                          "source": "windows_stc"})
        for value in ({}, {"direction": 5}, {"direction": 1, "speed_mm_s": 59},
                      {"direction": 1, "speed_mm_s": 301},
                      {"direction": 1, "lease_ms": 501}, {"direction": True},
                      {"direction": 1.0}, None):
            with self.assertRaises(ValueError):
                normalize_jog(value)

    def test_http_worker_keeps_latest_state_during_slow_request(self):
        calls = []
        first_started = threading.Event()
        release_first = threading.Event()
        second_done = threading.Event()

        def fake_post(_url, direction, _timeout):
            calls.append(direction)
            if len(calls) == 1:
                first_started.set()
                release_first.wait(1.0)
            else:
                second_done.set()

        original = remote_bridge.post_jog
        remote_bridge.post_jog = fake_post
        sender = remote_bridge.LatestJogSender("http://robot", 0.25)
        try:
            sender.submit(1, 1)
            self.assertTrue(first_started.wait(1.0))
            sender.submit(2, 2)
            sender.submit(0, 3)
            release_first.set()
            self.assertTrue(second_done.wait(1.0))
            self.assertEqual(calls, [1, 0])
        finally:
            sender.close()
            remote_bridge.post_jog = original


if __name__ == "__main__":
    unittest.main()
