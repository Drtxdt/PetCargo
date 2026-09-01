"""Pure motion math shared by the ROS node and host-side tests."""

from __future__ import annotations

import math


def clamp(value: float, low: float, high: float) -> float:
    return max(low, min(high, value))


def normalize_angle(value: float) -> float:
    return math.atan2(math.sin(value), math.cos(value))


def project_along_heading(start_x: float, start_y: float, heading: float, x: float, y: float) -> float:
    return math.cos(heading) * (x - start_x) + math.sin(heading) * (y - start_y)


def project_lateral_to_heading(start_x: float, start_y: float, heading: float, x: float, y: float) -> float:
    """Positive progress is to the robot's initial left side."""
    return -math.sin(heading) * (x - start_x) + math.cos(heading) * (y - start_y)


def linear_speed_for_error(error: float, maximum: float, minimum: float = 0.055) -> float:
    if abs(error) <= 0.03:
        return 0.0
    magnitude = clamp(abs(error) * 0.75, minimum, maximum)
    return math.copysign(magnitude, error)


def angular_speed_for_error(error: float, maximum: float, minimum: float = 0.18) -> float:
    if abs(error) <= math.radians(4.0):
        return 0.0
    magnitude = clamp(abs(error) * 0.9, minimum, maximum)
    return math.copysign(magnitude, error)


class AngleAccumulator:
    def __init__(self, initial_yaw: float) -> None:
        self.last_yaw = initial_yaw
        self.total = 0.0

    def update(self, yaw: float) -> float:
        self.total += normalize_angle(yaw - self.last_yaw)
        self.last_yaw = yaw
        return self.total
