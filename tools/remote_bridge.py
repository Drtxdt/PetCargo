#!/usr/bin/env python3
"""Forward a USB-connected STC navigation remote to the robot dashboard."""

import argparse
import json
import sys
import time
import urllib.error
import urllib.request
from pathlib import Path

import serial
from serial.tools import list_ports

sys.path.insert(0, str(Path(__file__).resolve().parent))
from remote_protocol import RemoteParser  # noqa: E402


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
                       "speed_mm_s": 120, "lease_ms": 300}).encode("utf-8")
    request = urllib.request.Request(base_url.rstrip("/") + "/api/jog", data=body,
                                     headers={"Content-Type": "application/json"}, method="POST")
    with urllib.request.urlopen(request, timeout=timeout) as response:
        if response.status != 200:
            raise RuntimeError(f"robot returned HTTP {response.status}")


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
    print(f"PetCargo remote: {port} -> {args.robot}", flush=True)
    try:
        stream = serial.Serial(port=None, baudrate=9600, timeout=0.1)
        stream.dtr = False
        stream.rts = False
        stream.port = port
        stream.open()
        with stream:
            while True:
                data = stream.read(64)
                frames = decoder.feed(data)
                # HTTP can briefly block while serial bytes accumulate. Forward only
                # the newest state so a stale direction cannot outrun a queued release.
                for frame in frames[-1:]:
                    direction = frame["direction"]
                    last_frame = time.monotonic()
                    try:
                        post_jog(args.robot, direction, args.http_timeout)
                        stopped = direction in (0, 5)
                        print(f"seq={frame['sequence']:03d} direction={direction} ok", flush=True)
                    except (OSError, urllib.error.URLError, RuntimeError) as exc:
                        print(f"network error: {exc}; robot lease will expire", file=sys.stderr, flush=True)
                if not stopped and time.monotonic() - last_frame >= 0.3:
                    try:
                        post_jog(args.robot, 0, args.http_timeout)
                    except (OSError, urllib.error.URLError, RuntimeError):
                        pass
                    stopped = True
                    print("serial lease expired -> stop", flush=True)
    except (KeyboardInterrupt, serial.SerialException) as exc:
        if not isinstance(exc, KeyboardInterrupt):
            print(f"serial error: {exc}", file=sys.stderr)
    finally:
        try:
            post_jog(args.robot, 0, args.http_timeout)
        except Exception:
            pass
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
