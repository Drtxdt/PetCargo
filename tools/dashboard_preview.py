#!/usr/bin/env python3
"""Preview the dashboard with deterministic fake data, without ROS."""

import argparse
import json
import mimetypes
import os
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlparse

ROOT = Path(__file__).resolve().parents[1]
WEB = ROOT / "dashboard"
STARTED = time.time()


class Handler(BaseHTTPRequestHandler):
    def log_message(self, _format, *_args):
        pass

    def json(self, value, status=200):
        body = json.dumps(value, ensure_ascii=False).encode("utf-8")
        self.send_response(status); self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body))); self.end_headers(); self.wfile.write(body)

    def do_GET(self):
        path = urlparse(self.path).path
        if path == "/api/state":
            elapsed = time.time() - STARTED
            fear = 72 if int(elapsed / 4) % 2 else 18
            self.json({
                "serial_connected": True,
                "telemetry": {"light_raw": 27, "temp_x10": 268, "vibration": 0,
                              "accel_x_mg": 15, "accel_y_mg": -8, "accel_z_mg": 1002,
                              "feed_count": 5, "happiness": 68, "fear": fear,
                              "sleeping": False, "remote_active": False, "received_at": time.time()},
                "motion": {"status": "idle", "last_result": {"motion_id": 7}},
                "safety": {"latched": False, "reason": "velocity_lease_expired"},
                "updated_at": time.time(),
            }); return
        if path == "/api/events":
            now = time.time()
            self.json([
                {"timestamp": now - 3, "name": "bright_light", "value": 27},
                {"timestamp": now - 16, "name": "feed", "value": 5},
                {"timestamp": now - 31, "name": "voice", "value": 2},
            ]); return
        relative = "index.html" if path == "/" else path.lstrip("/")
        target = (WEB / relative).resolve()
        if WEB.resolve() not in target.parents and target != WEB.resolve(): self.send_error(403); return
        if not target.is_file(): self.send_error(404); return
        body = target.read_bytes(); self.send_response(200)
        self.send_header("Content-Type", mimetypes.guess_type(str(target))[0] or "application/octet-stream")
        self.send_header("Content-Length", str(len(body))); self.end_headers(); self.wfile.write(body)

    def do_POST(self):
        length = int(self.headers.get("Content-Length", "0")); self.rfile.read(length)
        self.json({"ok": True})


def main():
    parser = argparse.ArgumentParser(); parser.add_argument("--port", type=int, default=8765); args = parser.parse_args()
    server = ThreadingHTTPServer(("127.0.0.1", args.port), Handler)
    print(f"PetCargo dashboard preview: http://127.0.0.1:{args.port}", flush=True)
    try: server.serve_forever()
    except KeyboardInterrupt: pass
    finally: server.server_close()


if __name__ == "__main__":
    main()
