import math
import os
import sys
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "ros", "petcargo_ros", "src"))

from petcargo_ros.motion import (  # noqa: E402
    AngleAccumulator,
    angular_speed_for_error,
    linear_speed_for_error,
    normalize_angle,
    project_along_heading,
    project_lateral_to_heading,
)


class MotionMathTests(unittest.TestCase):
    def test_backward_projection(self):
        self.assertAlmostEqual(project_along_heading(1, 1, 0, 0.2, 1), -0.8)
        self.assertAlmostEqual(project_along_heading(1, 1, math.pi / 2, 1, 0.2), -0.8)

    def test_lateral_projection(self):
        self.assertAlmostEqual(project_lateral_to_heading(1, 1, 0, 1, 1.5), 0.5)
        self.assertAlmostEqual(project_lateral_to_heading(1, 1, math.pi / 2, 0.5, 1), 0.5)

    def test_speed_limits_and_deadband(self):
        self.assertEqual(linear_speed_for_error(0.01, 0.18), 0)
        self.assertAlmostEqual(linear_speed_for_error(-1.0, 0.18), -0.18)
        self.assertLess(linear_speed_for_error(0.05, 0.18), 0.1)

    def test_angle_unwrap(self):
        acc = AngleAccumulator(math.radians(170))
        self.assertAlmostEqual(acc.update(math.radians(-170)), math.radians(20), places=6)
        self.assertAlmostEqual(acc.update(math.radians(-150)), math.radians(40), places=6)

    def test_normalize_and_angular_deadband(self):
        self.assertAlmostEqual(normalize_angle(3 * math.pi), math.pi, places=6)
        self.assertEqual(angular_speed_for_error(math.radians(2), 0.5), 0)


if __name__ == "__main__":
    unittest.main()
