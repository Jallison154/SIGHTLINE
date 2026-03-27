#!/usr/bin/env python3
"""
SIGHTLINE local development server.

Hosts:
- Controller web UI (/controller)
- Fixture node web UI (/node?nodeId=fixture-1)
- Simulation REST APIs used by both pages
"""

from __future__ import annotations

import argparse
import json
import mimetypes
import sys
from http import HTTPStatus
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import parse_qs, urlparse

ROOT = Path(__file__).resolve().parents[1]
TEST_ENV = Path(__file__).resolve().parent
if str(TEST_ENV) not in sys.path:
    sys.path.insert(0, str(TEST_ENV))

from shared.sim_runtime import SimRuntime


def _json_bytes(d: dict) -> bytes:
    return json.dumps(d, separators=(",", ":")).encode("utf-8")


class SimHandler(BaseHTTPRequestHandler):
    runtime: SimRuntime = None  # type: ignore[assignment]

    def log_message(self, format: str, *args) -> None:
        # Keep terminal cleaner than default http.server logging.
        self.runtime.log.log(f"http {self.address_string()} {format % args}", also_stderr=False)

    def _send_json(self, status: int, payload: dict) -> None:
        body = _json_bytes(payload)
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def _read_json(self) -> dict:
        n = int(self.headers.get("Content-Length", "0") or "0")
        if n <= 0:
            return {}
        raw = self.rfile.read(n)
        try:
            return json.loads(raw.decode("utf-8"))
        except json.JSONDecodeError:
            return {}

    def _serve_file(self, path: Path) -> None:
        if not path.exists() or not path.is_file():
            self.send_error(HTTPStatus.NOT_FOUND, "Not found")
            return
        ctype = mimetypes.guess_type(str(path))[0] or "application/octet-stream"
        data = path.read_bytes()
        self.send_response(HTTPStatus.OK)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def _static_path_for(self, req_path: str) -> Path | None:
        data_root = ROOT / "data"
        if req_path in ("/", "/controller", "/controller/"):
            return data_root / "controller" / "index.html"
        if req_path in ("/node", "/node/"):
            return data_root / "node" / "index.html"
        if req_path == "/sightline_logo.png":
            return data_root / "sightline_logo.png"
        if req_path.startswith("/controller/"):
            rel = req_path[len("/controller/") :]
            return data_root / "controller" / rel
        if req_path.startswith("/node/"):
            rel = req_path[len("/node/") :]
            return data_root / "node" / rel
        if req_path.startswith("/shared/"):
            rel = req_path[len("/shared/") :]
            return data_root / "shared" / rel
        return None

    def do_GET(self) -> None:
        p = urlparse(self.path)
        path = p.path
        q = parse_qs(p.query)

        if path == "/api/state":
            self._send_json(200, self.runtime.get_state())
            return
        if path == "/api/nodes":
            self._send_json(200, self.runtime.get_nodes())
            return
        if path == "/api/target-fixture":
            self._send_json(200, self.runtime.get_target_fixture())
            return
        if path == "/api/status":
            node_id = (q.get("nodeId") or [None])[0]
            if node_id:
                self._send_json(200, self.runtime.get_node_status(node_id))
            else:
                self._send_json(200, self.runtime.get_controller_status())
            return
        if path == "/api/config":
            node_id = (q.get("nodeId") or ["fixture-1"])[0]
            self._send_json(200, self.runtime.get_node_config(node_id))
            return
        if path == "/api/profiles":
            node_id = (q.get("nodeId") or ["fixture-1"])[0]
            self._send_json(200, self.runtime.get_node_profiles(node_id))
            return
        if path == "/api/profile/export":
            node_id = (q.get("nodeId") or ["fixture-1"])[0]
            profile_id = (q.get("profileId") or [""])[0]
            self._send_json(200, self.runtime.node_profile_export(node_id, profile_id))
            return

        static_path = self._static_path_for(path)
        if static_path is not None:
            self._serve_file(static_path)
            return
        self.send_error(HTTPStatus.NOT_FOUND, "Unknown route")

    def do_POST(self) -> None:
        p = urlparse(self.path)
        path = p.path
        q = parse_qs(p.query)
        payload = self._read_json()

        if path == "/api/state":
            self._send_json(200, self.runtime.patch_state(payload))
            return
        if path == "/api/select-node":
            node_id = str(payload.get("nodeId") or "")
            self._send_json(200, self.runtime.select_node(node_id))
            return
        if path == "/api/config":
            node_id = (q.get("nodeId") or ["fixture-1"])[0]
            self._send_json(200, self.runtime.save_node_config(node_id, payload))
            return
        if path == "/api/profile":
            node_id = (q.get("nodeId") or ["fixture-1"])[0]
            self._send_json(200, self.runtime.node_profile_action(node_id, payload))
            return
        if path == "/api/profile/import":
            node_id = (q.get("nodeId") or ["fixture-1"])[0]
            self._send_json(200, self.runtime.node_profile_import(node_id, payload))
            return

        self.send_error(HTTPStatus.NOT_FOUND, "Unknown route")


def main() -> None:
    ap = argparse.ArgumentParser(description="SIGHTLINE local dev simulation server")
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=8088)
    args = ap.parse_args()

    runtime = SimRuntime(profile_dir=TEST_ENV / "profiles", log_dir=TEST_ENV / "logs")
    runtime.start()
    SimHandler.runtime = runtime

    srv = ThreadingHTTPServer((args.host, args.port), SimHandler)
    print(f"SIGHTLINE dev server running on http://{args.host}:{args.port}")
    print(f"Controller UI: http://{args.host}:{args.port}/controller")
    print(f"Fixture UI:    http://{args.host}:{args.port}/node?nodeId=fixture-1")
    try:
        srv.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        srv.server_close()
        runtime.stop()


if __name__ == "__main__":
    main()
