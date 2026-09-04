"""Thread-safe dashboard state independent from HTTP and ROS glue."""

from __future__ import annotations

import copy
import threading
import time
from collections import deque


class DashboardState:
    def __init__(self, event_limit: int = 80) -> None:
        self._lock = threading.RLock()
        self._state = {
            "serial_connected": False,
            "motion": {"status": "idle"},
            "lidar": {"online": False, "reason": "waiting_for_scan", "phase": "track"},
            "safety": {"latched": False, "reason": "startup"},
            "telemetry": {
                "light_raw": 0,
                "temp_x10": 250,
                "happiness": 50,
                "fear": 0,
                "sleeping": False,
            },
            "updated_at": time.time(),
        }
        self._events = deque(maxlen=event_limit)

    def merge(self, key: str, value) -> None:
        with self._lock:
            self._state[key] = value
            self._state["updated_at"] = time.time()

    def update_telemetry(self, value: dict) -> None:
        with self._lock:
            self._state["telemetry"] = value
            self._state["updated_at"] = time.time()

    def add_event(self, value: dict) -> None:
        event = dict(value)
        event.setdefault("timestamp", time.time())
        with self._lock:
            self._events.appendleft(event)

    def snapshot(self) -> dict:
        with self._lock:
            return copy.deepcopy(self._state)

    def events(self) -> list:
        with self._lock:
            return copy.deepcopy(list(self._events))
