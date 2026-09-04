#!/usr/bin/env python3
"""Forward a USB-connected STC navigation remote to the robot dashboard."""

import argparse
import json
import sys
import threading
import time
import urllib.error
import urllib.request
from pathlib import Path

import serial
from serial.tools import list_ports

sys.path.insert(0, str(Path(__file__).resolve().parent))
from remote_protocol import RemoteParser  # noqa: E402

DIRECTION_NAMES = {0: "released", 1: "forward", 2: "backward", 3: "left", 4: "right", 5: "stop"}


def choose_port(requested: str | None) -> str:
    if requested:
        return requested
    candidates = [p.device for p in list_ports.comports()
                  if "CH340" in p.description.upper() or "1A86" in (p.hwid or "").upper()]
    if len(candidates) != 1:
        found = ", ".join(f"{p.device} ({p.description})" for p in list_ports.comports()) or "none"
        raise RuntimeError(f"Expected exactly one CH340; found {found}. Pass --port COMx.")
    return candidates[0]


def post_jog(base_url: str, direction: int, timeout: float) -> None:
    body = json.dumps({"direction": 0 if direction in (0, 5) else direction,
                       "speed_mm_s": 250, "lease_ms": 500}).encode("utf-8")
    request = urllib.request.Request(base_url.rstrip("/") + "/api/jog", data=body,
                                     headers={"Content-Type": "application/json"}, method="POST")
    with urllib.request.urlopen(request, timeout=timeout) as response:
        if response.status != 200:
            raise RuntimeError(f"robot returned HTTP {response.status}")


def robot_preflight(base_url: str, timeout: float) -> list[str]:
    """Return actionable reasons why HTTP can work while motion is rejected."""
    with urllib.request.urlopen(base_url.rstrip("/") + "/api/state", timeout=timeout) as response:
        value = json.loads(response.read().decode("utf-8"))
    reasons = []
    if value.get("safety", {}).get("latched"):
        reasons.append("locked emergency stop")
    lidar = value.get("lidar", {})
    if not lidar.get("online"):
        reasons.append("lidar offline: " + str(lidar.get("reason", "unknown")))
    return reasons


class LatestJogSender:
    """Keep serial ingestion independent from potentially slow HTTP requests."""

    def __init__(self, base_url: str, timeout: float) -> None:
        self.base_url = base_url
        self.timeout = timeout
        self.condition = threading.Condition()
        self.pending = None
        self.closed = False
        self.worker = threading.Thread(target=self.run, name="petcargo-jog-http", daemon=True)
        self.worker.start()

    def submit(self, direction: int, sequence: int) -> None:
        with self.condition:
            # One-slot mailbox: a release/direction change supersedes stale
            # renewals immediately instead of waiting behind an HTTP queue.
            self.pending = (direction, sequence)
            self.condition.notify()

    def run(self) -> None:
        while True:
            with self.condition:
                while self.pending is None and not self.closed:
                    self.condition.wait()
                if self.closed:
                    return
                direction, sequence = self.pending
                self.pending = None
            try:
                post_jog(self.base_url, direction, self.timeout)
                print(
                    f"seq={sequence:03d} direction={direction} "
                    f"({DIRECTION_NAMES[direction]}) accepted",
                    flush=True,
                )
            except (OSError, urllib.error.URLError, RuntimeError) as exc:
                print(f"network error: {exc}; robot lease will expire", file=sys.stderr, flush=True)

    def close(self) -> None:
        with self.condition:
            self.closed = True
            self.condition.notify()
        self.worker.join(timeout=max(0.5, self.timeout + 0.1))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--robot", required=True, help="e.g. http://192.168.1.20:8080")
    parser.add_argument("--port", help="COM port; auto-detects when exactly one CH340 exists")
    parser.add_argument("--http-timeout", type=float, default=0.25)
    args = parser.parse_args()
    if args.http_timeout <= 0:
        parser.error("--http-timeout must be positive")
    port = choose_port(args.port)
    decoder = RemoteParser()
    last_frame = time.monotonic()
    stopped = True
    sender = None
    print(f"PetCargo remote: {port} -> {args.robot}", flush=True)
    try:
        reasons = robot_preflight(args.robot, max(args.http_timeout, 0.5))
        if reasons:
            print("robot will reject movement: " + "; ".join(reasons), file=sys.stderr, flush=True)
    except (OSError, ValueError, urllib.error.URLError) as exc:
        print(f"robot preflight unavailable: {exc}", file=sys.stderr, flush=True)
    try:
        stream = serial.Serial(port=None, baudrate=9600, timeout=0.1)
        stream.dtr = False
        stream.rts = False
        stream.port = port
        stream.open()
        sender = LatestJogSender(args.robot, args.http_timeout)
        with stream:
            while True:
                data = stream.read(64)
                frames = decoder.feed(data)
                # HTTP can briefly block while serial bytes accumulate. Forward only
                # the newest state so a stale direction cannot outrun a queued release.
                for frame in frames[-1:]:
                    direction = frame["direction"]
                    last_frame = time.monotonic()
                    sender.submit(direction, frame["sequence"])
                    stopped = direction in (0, 5)
                if not stopped and time.monotonic() - last_frame >= 0.3:
                    sender.submit(0, 0)
                    stopped = True
                    print("serial lease expired -> stop", flush=True)
    except (KeyboardInterrupt, serial.SerialException) as exc:
        if not isinstance(exc, KeyboardInterrupt):
            print(f"serial error: {exc}", file=sys.stderr)
    finally:
        if sender is not None:
            sender.close()
        try:
            post_jog(args.robot, 0, args.http_timeout)
        except Exception:
            pass
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
