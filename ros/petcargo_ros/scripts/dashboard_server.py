#!/usr/bin/env python3
"""Small same-origin dashboard server using only Python's standard library."""

import json
import mimetypes
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlparse

import rospy
from std_msgs.msg import Bool, String

from petcargo_ros.state_store import DashboardState


class DashboardNode:
    def __init__(self) -> None:
        self.state = DashboardState()
        self.safety_pub = rospy.Publisher("/petcargo/safety_set", Bool, queue_size=4)
        rospy.Subscriber("/petcargo/telemetry", String, self.on_telemetry, queue_size=10)
        rospy.Subscriber("/petcargo/events", String, self.on_event, queue_size=30)
        rospy.Subscriber("/petcargo/motion_status", String, self.on_motion, queue_size=10)
        rospy.Subscriber("/petcargo/safety_state", String, self.on_safety_state, queue_size=10)
        rospy.Subscriber("/petcargo/serial_connected", Bool, self.on_connected, queue_size=4)

        configured = rospy.get_param("~web_root", "").strip()
        source_root = Path(__file__).resolve().parents[3] / "dashboard"
        package_fallback = Path(__file__).resolve().parents[1] / "web"
        self.web_root = Path(configured) if configured else source_root
        if not self.web_root.is_dir():
            self.web_root = package_fallback
        if not (self.web_root / "index.html").is_file():
            raise RuntimeError(f"Dashboard assets not found at {self.web_root}")

        self.host = rospy.get_param("~dashboard_host", "0.0.0.0")
        self.port = int(rospy.get_param("~dashboard_port", 8080))
        self.server = ThreadingHTTPServer((self.host, self.port), self.make_handler())
        self.thread = threading.Thread(target=self.server.serve_forever, name="dashboard-http")
        self.thread.daemon = True
        self.thread.start()
        rospy.on_shutdown(self.shutdown)
        rospy.loginfo("PetCargo dashboard: http://%s:%d", self.host, self.port)

    @staticmethod
    def decode_json(message: String) -> dict:
        try:
            return json.loads(message.data)
        except (TypeError, json.JSONDecodeError):
            return {"raw": message.data}

    def on_telemetry(self, message: String) -> None:
        value = self.decode_json(message)
        current = self.state.snapshot().get("telemetry", {})
        current.update(value)
        self.state.update_telemetry(current)

    def on_event(self, message: String) -> None:
        self.state.add_event(self.decode_json(message))

    def on_motion(self, message: String) -> None:
        self.state.merge("motion", self.decode_json(message))

    def on_safety_state(self, message: String) -> None:
        self.state.merge("safety", self.decode_json(message))

    def on_connected(self, message: Bool) -> None:
        self.state.merge("serial_connected", bool(message.data))

    def make_handler(self):
        node = self

        class Handler(BaseHTTPRequestHandler):
            server_version = "PetCargoDashboard/1.0"

            def log_message(self, fmt, *args):
                rospy.logdebug("dashboard: " + fmt, *args)

            def send_json(self, value, status=200):
                body = json.dumps(value, ensure_ascii=False).encode("utf-8")
                self.send_response(status)
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.send_header("Content-Length", str(len(body)))
                self.send_header("Cache-Control", "no-store")
                self.end_headers()
                self.wfile.write(body)

            def read_json(self):
                length = min(int(self.headers.get("Content-Length", "0")), 4096)
                if not length:
                    return {}
                return json.loads(self.rfile.read(length).decode("utf-8"))

            def do_GET(self):
                path = urlparse(self.path).path
                if path == "/api/state":
                    self.send_json(node.state.snapshot())
                    return
                if path == "/api/events":
                    self.send_json(node.state.events())
                    return
                relative = "index.html" if path == "/" else path.lstrip("/")
                target = (node.web_root / relative).resolve()
                try:
                    target.relative_to(node.web_root.resolve())
                except ValueError:
                    self.send_error(403)
                    return
                if not target.is_file():
                    self.send_error(404)
                    return
                body = target.read_bytes()
                mime = mimetypes.guess_type(str(target))[0] or "application/octet-stream"
                self.send_response(200)
                self.send_header("Content-Type", mime)
                self.send_header("Content-Length", str(len(body)))
                self.send_header("Cache-Control", "no-cache")
                self.end_headers()
                self.wfile.write(body)

            def do_POST(self):
                path = urlparse(self.path).path
                try:
                    value = self.read_json()
                except (ValueError, json.JSONDecodeError):
                    self.send_json({"ok": False, "error": "invalid_json"}, 400)
                    return
                if path == "/api/stop":
                    engaged = bool(value.get("engaged", True))
                    node.safety_pub.publish(engaged)
                    self.send_json({"ok": True, "engaged": engaged})
                else:
                    self.send_error(404)

        return Handler

    def shutdown(self) -> None:
        self.server.shutdown()
        self.server.server_close()


if __name__ == "__main__":
    rospy.init_node("dashboard_server")
    DashboardNode()
    rospy.spin()
