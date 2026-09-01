import os
import sys
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "ros", "petcargo_ros", "src"))

from petcargo_ros.state_store import DashboardState  # noqa: E402


class DashboardStateTests(unittest.TestCase):
    def test_snapshot_is_not_a_live_reference(self):
        state = DashboardState()
        state.update_telemetry({"fear": 10})
        snapshot = state.snapshot()
        snapshot["telemetry"]["fear"] = 99
        self.assertEqual(state.snapshot()["telemetry"]["fear"], 10)

    def test_event_limit_and_order(self):
        state = DashboardState(event_limit=2)
        state.add_event({"name": "first", "timestamp": 1})
        state.add_event({"name": "second", "timestamp": 2})
        state.add_event({"name": "third", "timestamp": 3})
        self.assertEqual([event["name"] for event in state.events()], ["third", "second"])


if __name__ == "__main__":
    unittest.main()

