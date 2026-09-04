import math
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "ros" / "petcargo_ros" / "src"))

from petcargo_ros.obstacle import (  # noqa: E402
    body_intrusion,
    choose_detour_path,
    choose_detour_side,
    corridor_clearance,
    detour_target_satisfied,
    detour_sides,
    filter_footprint_points,
    next_detour_phase,
    lidar_health,
    rotation_blocked,
    speed_for_clearance,
    transform_scan_points,
    update_obstacle_confirmation,
)


class ObstacleGeometryTests(unittest.TestCase):
    def test_scan_transform_filters_and_applies_tf(self):
        points, valid = transform_scan_points(
            [1.0, math.inf, math.nan, 0.01, 3.0],
            0.0,
            math.pi / 2,
            0.08,
            2.0,
            0.10,
            -0.20,
            math.pi / 2,
        )
        self.assertEqual(valid, 1)
        self.assertAlmostEqual(points[0][0], 0.10, places=6)
        self.assertAlmostEqual(points[0][1], 0.80, places=6)

    def test_directional_corridors_and_body_edge_clearance(self):
        points = [(0.271, 0.0), (-0.371, 0.0), (0.0, 0.228), (0.0, -0.328)]
        self.assertAlmostEqual(corridor_clearance(points, 1), 0.10, places=6)
        self.assertAlmostEqual(corridor_clearance(points, 2), 0.20, places=6)
        self.assertAlmostEqual(corridor_clearance(points, 3), 0.10, places=6)
        self.assertAlmostEqual(corridor_clearance(points, 4), 0.20, places=6)

    def test_side_wall_outside_corridor_is_ignored(self):
        self.assertTrue(math.isinf(corridor_clearance([(0.25, 0.40)], 1)))

    def test_side_selection_and_right_tie_break(self):
        self.assertEqual(detour_sides(1), (3, 4))
        selected, left, right = choose_detour_side([], 1)
        self.assertEqual(selected, 4)
        self.assertTrue(math.isinf(left) and math.isinf(right))
        selected, _, _ = choose_detour_side([(0.0, -0.40)], 1)
        self.assertEqual(selected, 3)
        selected, _, _ = choose_detour_side([(0.0, 0.40), (0.0, -0.40)], 1)
        self.assertIsNone(selected)

    def test_complete_detour_path_is_checked_before_selecting_side(self):
        self.assertEqual(choose_detour_path([], 1)[0], 4)
        # This point is outside the first left-shift corridor, but blocks the
        # future forward bypass lane after that shift.  Full-path validation
        # must reject left and choose right.
        selected, left_spare, right_spare = choose_detour_path([(0.50, 0.40)], 1)
        self.assertEqual(selected, 4)
        self.assertLess(left_spare, 0.15)
        self.assertGreaterEqual(right_spare, 0.15)
        # Block the mirrored bypass lane as well: no detour is safe.
        selected, _, _ = choose_detour_path([(0.50, 0.40), (0.50, -0.40)], 1)
        self.assertIsNone(selected)

    def test_current_body_intrusion_is_immediate(self):
        # A point inside the physical chassis is a self-return, not an
        # external obstacle.  The safety shell immediately outside still trips.
        self.assertFalse(body_intrusion([(-0.114, 0.084)]))
        self.assertTrue(body_intrusion([(0.19, 0.0)]))
        self.assertFalse(body_intrusion([(0.30, 0.0)]))
        kept, removed = filter_footprint_points(
            [(-0.114, 0.084), (0.19, 0.0), (0.40, 0.20)]
        )
        self.assertEqual(removed, 1)
        self.assertEqual(kept, [(0.19, 0.0), (0.40, 0.20)])

    def test_rotation_and_speed_profile(self):
        self.assertTrue(rotation_blocked([(0.25, 0.0)]))
        self.assertFalse(rotation_blocked([(0.40, 0.0)]))
        self.assertEqual(speed_for_clearance(0.20, 0.15), 0.0)
        self.assertEqual(speed_for_clearance(0.20, 0.35), 0.20)
        self.assertAlmostEqual(speed_for_clearance(0.20, 0.25), 0.14, places=6)
        self.assertEqual(speed_for_clearance(0.06, 0.20), 0.06)

    def test_three_phase_sequence_and_overshoot_policy(self):
        phase = next_detour_phase("shift_out", 1, 4)
        self.assertEqual(phase, ("pass", 1, 0.50))
        phase = next_detour_phase(phase[0], 1, 4)
        self.assertEqual(phase, ("shift_back", 3, 0.40))
        self.assertIsNone(next_detour_phase(phase[0], 1, 4))
        self.assertFalse(detour_target_satisfied(0.46, 0.50))
        self.assertTrue(detour_target_satisfied(0.48, 0.50))
        self.assertTrue(detour_target_satisfied(0.70, 0.50))

    def test_confirmation_counts_new_scans_and_emergency_is_immediate(self):
        hits, generation = 0, -1
        for scan in (1,):
            triggered, hits, generation = update_obstacle_confirmation(
                hits, generation, scan, 0.15
            )
            self.assertFalse(triggered)
            duplicate = update_obstacle_confirmation(hits, generation, scan, 0.15)
            self.assertFalse(duplicate[0])
            self.assertEqual(duplicate[1], hits)
        triggered, hits, generation = update_obstacle_confirmation(
            hits, generation, 2, 0.15
        )
        self.assertTrue(triggered)
        self.assertTrue(update_obstacle_confirmation(0, 3, 3, 0.10)[0])

    def test_lidar_health_is_fail_closed(self):
        self.assertEqual(lidar_health(0.0, 4.0, "")[:2], (False, "waiting_for_scan"))
        self.assertEqual(lidar_health(1.0, 1.6, "")[:2], (False, "scan_stale"))
        self.assertEqual(lidar_health(1.0, 1.1, "tf_unavailable")[:2], (False, "tf_unavailable"))
        self.assertEqual(lidar_health(1.0, 1.1, "")[:2], (True, "ready"))


if __name__ == "__main__":
    unittest.main()
