"""Validation for the laptop-connected STC remote HTTP endpoint."""


def normalize_jog(value):
    try:
        direction = value["direction"]
        speed = value.get("speed_mm_s", 120)
        lease = value.get("lease_ms", 300)
        if any(isinstance(item, bool) or not isinstance(item, int)
               for item in (direction, speed, lease)):
            raise TypeError
    except (KeyError, TypeError, AttributeError) as exc:
        raise ValueError("invalid_jog") from exc
    if direction not in range(5) or not 60 <= speed <= 180 or not 100 <= lease <= 500:
        raise ValueError("jog_out_of_range")
    return {"direction": direction, "speed_mm_s": speed,
            "lease_ms": lease, "source": "windows_stc"}
