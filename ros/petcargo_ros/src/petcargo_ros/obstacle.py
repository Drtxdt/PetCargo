"""Pure geometry helpers for PetCargo's local lidar obstacle guard."""

from __future__ import annotations

import math
from typing import Iterable, List, Optional, Sequence, Tuple


Point = Tuple[float, float]

DIRECTION_VECTORS = {
    1: (1.0, 0.0),   # forward
    2: (-1.0, 0.0),  # backward
    3: (0.0, 1.0),   # left
    4: (0.0, -1.0),  # right
}


def transform_scan_points(
    ranges: Sequence[float],
    angle_min: float,
    angle_increment: float,
    range_min: float,
    range_max: float,
    translation_x: float,
    translation_y: float,
    transform_yaw: float,
) -> Tuple[List[Point], int]:
    """Convert finite laser ranges to points in the robot base frame."""
    points: List[Point] = []
    valid = 0
    cosine = math.cos(transform_yaw)
    sine = math.sin(transform_yaw)
    for index, raw in enumerate(ranges):
        value = float(raw)
        if not math.isfinite(value) or value < range_min or value > range_max:
            continue
        angle = angle_min + index * angle_increment
        laser_x = value * math.cos(angle)
        laser_y = value * math.sin(angle)
        points.append(
            (
                translation_x + cosine * laser_x - sine * laser_y,
                translation_y + sine * laser_x + cosine * laser_y,
            )
        )
        valid += 1
    return points, valid


def direction_vector(direction: int) -> Point:
    try:
        return DIRECTION_VECTORS[int(direction)]
    except (KeyError, TypeError, ValueError):
        raise ValueError("direction must be 1..4")


def opposite_direction(direction: int) -> int:
    return {1: 2, 2: 1, 3: 4, 4: 3}[int(direction)]


def detour_sides(direction: int) -> Tuple[int, int]:
    """Return (relative-left, relative-right); ties must choose the latter."""
    return {
        1: (3, 4),
        2: (4, 3),
        3: (2, 1),
        4: (1, 2),
    }[int(direction)]


def body_extent(direction: int, half_length: float, half_width: float) -> float:
    dx, dy = direction_vector(direction)
    return abs(dx) * half_length + abs(dy) * half_width


def corridor_clearance(
    points: Iterable[Point],
    direction: int,
    half_length: float = 0.171,
    half_width: float = 0.128,
    corridor_margin: float = 0.03,
) -> float:
    """Nearest body-edge clearance inside the swept corridor, or infinity."""
    dx, dy = direction_vector(direction)
    # Left-perpendicular unit vector for the requested direction.
    px, py = -dy, dx
    leading_edge = body_extent(direction, half_length, half_width)
    cross_half = abs(px) * half_length + abs(py) * half_width + corridor_margin
    nearest = math.inf
    for x, y in points:
        along = x * dx + y * dy
        cross = x * px + y * py
        if along > leading_edge and abs(cross) <= cross_half:
            nearest = min(nearest, along - leading_edge)
    return nearest


def rotation_blocked(
    points: Iterable[Point],
    half_length: float = 0.171,
    half_width: float = 0.128,
    clearance: float = 0.10,
) -> bool:
    """Conservative inflated rectangle used before and during rotation."""
    x_limit = half_length + clearance
    y_limit = half_width + clearance
    return any(abs(x) <= x_limit and abs(y) <= y_limit for x, y in points)


def body_intrusion(
    points: Iterable[Point],
    half_length: float = 0.171,
    half_width: float = 0.128,
    margin: float = 0.03,
) -> bool:
    """Detect a return in the safety shell immediately outside the chassis.

    Returns inside the physical footprint are lidar self-reflections and must
    be filtered rather than interpreted as an external obstacle.
    """
    return any(
        abs(x) <= half_length + margin
        and abs(y) <= half_width + margin
        and not (abs(x) <= half_length and abs(y) <= half_width)
        for x, y in points
    )


def filter_footprint_points(
    points: Iterable[Point],
    half_length: float = 0.171,
    half_width: float = 0.128,
) -> Tuple[List[Point], int]:
    """Remove lidar returns originating inside the robot's own footprint."""
    kept = []
    removed = 0
    for x, y in points:
        if abs(x) <= half_length and abs(y) <= half_width:
            removed += 1
        else:
            kept.append((x, y))
    return kept, removed


def offset_points(points: Iterable[Point], offset_x: float, offset_y: float) -> List[Point]:
    """Express base-frame points relative to a planned future robot pose."""
    return [(x - offset_x, y - offset_y) for x, y in points]


def detour_path_clearance(
    points: Iterable[Point],
    original_direction: int,
    side_direction: int,
    shift_distance: float = 0.40,
    pass_distance: float = 0.50,
    half_length: float = 0.171,
    half_width: float = 0.128,
    corridor_margin: float = 0.03,
) -> float:
    """Minimum spare distance over all three swept detour segments.

    A positive value means every segment can reach its target before the body
    edge reaches the nearest lidar point.  This prevents choosing a clear
    first side-step whose bypass lane or return path is actually blocked.
    """
    materialized = list(points)
    original_dx, original_dy = direction_vector(original_direction)
    side_dx, side_dy = direction_vector(side_direction)
    segments = (
        (materialized, side_direction, shift_distance),
        (
            offset_points(materialized, side_dx * shift_distance, side_dy * shift_distance),
            original_direction,
            pass_distance,
        ),
        (
            offset_points(
                materialized,
                side_dx * shift_distance + original_dx * pass_distance,
                side_dy * shift_distance + original_dy * pass_distance,
            ),
            opposite_direction(side_direction),
            shift_distance,
        ),
    )
    spare = math.inf
    for segment_points, direction, distance in segments:
        clearance = corridor_clearance(
            segment_points, direction, half_length, half_width, corridor_margin
        )
        spare = min(spare, clearance - distance)
    return spare


def choose_detour_path(
    points: Iterable[Point],
    direction: int,
    required_clearance: float = 0.15,
    shift_distance: float = 0.40,
    pass_distance: float = 0.50,
    half_length: float = 0.171,
    half_width: float = 0.128,
    corridor_margin: float = 0.03,
) -> Tuple[Optional[int], float, float]:
    """Choose a side only when its complete three-segment path is clear."""
    materialized = list(points)
    left, right = detour_sides(direction)
    left_spare = detour_path_clearance(
        materialized, direction, left, shift_distance, pass_distance,
        half_length, half_width, corridor_margin,
    )
    right_spare = detour_path_clearance(
        materialized, direction, right, shift_distance, pass_distance,
        half_length, half_width, corridor_margin,
    )
    candidates = []
    if left_spare >= required_clearance:
        candidates.append((left_spare, 0, left))
    if right_spare >= required_clearance:
        candidates.append((right_spare, 1, right))
    if not candidates:
        return None, left_spare, right_spare
    selected = max(candidates, key=lambda item: (item[0], item[1]))[2]
    return selected, left_spare, right_spare


def choose_detour_side(
    points: Iterable[Point],
    direction: int,
    required_clearance: float = 0.45,
    half_length: float = 0.171,
    half_width: float = 0.128,
    corridor_margin: float = 0.03,
) -> Tuple[Optional[int], float, float]:
    """Choose the clearer perpendicular direction, preferring relative right."""
    materialized = list(points)
    left, right = detour_sides(direction)
    left_clearance = corridor_clearance(
        materialized, left, half_length, half_width, corridor_margin
    )
    right_clearance = corridor_clearance(
        materialized, right, half_length, half_width, corridor_margin
    )
    candidates = []
    if left_clearance >= required_clearance:
        candidates.append((left_clearance, 0, left))
    if right_clearance >= required_clearance:
        candidates.append((right_clearance, 1, right))
    if not candidates:
        return None, left_clearance, right_clearance
    # Larger clearance wins. The second key makes relative right win exact ties.
    selected = max(candidates, key=lambda item: (item[0], item[1]))[2]
    return selected, left_clearance, right_clearance


def speed_for_clearance(
    requested_speed: float,
    clearance: float,
    slow_distance: float = 0.35,
    stop_distance: float = 0.15,
    minimum_speed: float = 0.08,
) -> float:
    """Reduce, but never increase, an already bounded positive speed."""
    speed = max(0.0, float(requested_speed))
    if clearance <= stop_distance:
        return 0.0
    if not math.isfinite(clearance) or clearance >= slow_distance:
        return speed
    ratio = (clearance - stop_distance) / (slow_distance - stop_distance)
    reduced = minimum_speed + ratio * max(0.0, speed - minimum_speed)
    return min(speed, max(0.0, reduced))


def next_detour_phase(
    phase: str,
    original_direction: int,
    side_direction: int,
    shift_distance: float = 0.40,
    pass_distance: float = 0.50,
):
    """Return the next (phase, direction, distance), or None when complete."""
    if phase == "shift_out":
        return "pass", int(original_direction), float(pass_distance)
    if phase == "pass":
        return "shift_back", opposite_direction(side_direction), float(shift_distance)
    if phase == "shift_back":
        return None
    raise ValueError("unknown detour phase")


def detour_target_satisfied(progress: float, target: float, tolerance: float = 0.03) -> bool:
    """Avoidance overshoot is accepted; the robot must never reverse to 0.5 m."""
    return progress >= target - tolerance


def lidar_health(last_scan_time: float, now: float, error: str, timeout: float = 0.5):
    if not last_scan_time:
        return False, "waiting_for_scan", None
    age = now - last_scan_time
    if age > timeout:
        return False, "scan_stale", age
    if error:
        return False, error, age
    return True, "ready", age


def update_obstacle_confirmation(
    hits: int,
    last_generation: int,
    generation: int,
    clearance: float,
    stop_clearance: float = 0.15,
    emergency_clearance: float = 0.10,
    required_hits: int = 2,
):
    """Count distinct scans only; emergency clearance bypasses confirmation."""
    if clearance <= emergency_clearance:
        return True, hits, generation
    if last_generation == generation:
        return hits >= required_hits, hits, last_generation
    hits = hits + 1 if clearance <= stop_clearance else 0
    return hits >= required_hits, hits, generation
