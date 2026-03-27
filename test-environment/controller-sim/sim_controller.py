#!/usr/bin/env python3
"""
Virtual ground controller: encoder/fader simulation → abstract control frames (UDP JSON).

Modes:
  --demo       Smooth pan/tilt sweep + velocity logging
  --static     One-shot or repeated values (--pan, --tilt, --intensity, ...)
  --script     JSON file with frames (see README)
  --interactive  stdin commands: pan 32768, tilt 12000, intensity 200, send, quit
"""

from __future__ import annotations

import argparse
import json
import math
import socket
import sys
import time
from pathlib import Path

# Allow running without installing: parent = test-environment
_ROOT = Path(__file__).resolve().parents[1]
if str(_ROOT) not in sys.path:
    sys.path.insert(0, str(_ROOT))

from shared.control_protocol import AbstractControls, ControlFrameV1
from shared.logging_utils import SimLogger
from shared.num_utils import clamp8, clamp16, norm_to_u16


DEFAULT_CONTROL_PORT = 6470


def parse_dest(s: str) -> tuple[str, int, str]:
    parts = s.split(":")
    if len(parts) != 3:
        raise ValueError(f"expected host:port:node_id, got {s!r}")
    host, port_s, node_id = parts[0], parts[1], parts[2]
    return host, int(port_s), node_id


def run_demo(
    sock: socket.socket,
    dests: list[tuple[str, int, str]],
    controller_id: str,
    session_id: str,
    hz: float,
    duration_s: float,
    log: SimLogger,
) -> None:
    seq = 0
    t0 = time.monotonic()
    while True:
        t = time.monotonic()
        if duration_s > 0 and (t - t0) >= duration_s:
            break
        u = (t - t0) * 0.25
        pan = 0.5 + 0.45 * math.sin(u * 2 * math.pi)
        tilt = 0.5 + 0.35 * math.cos(u * 2 * math.pi * 0.7)
        ctrls = AbstractControls(
            pan16=norm_to_u16(pan),
            tilt16=norm_to_u16(tilt),
            intensity=clamp8(int(180 + 75 * math.sin(u * 0.5))),
            iris=128,
            zoom=200,
        )
        frame = ControlFrameV1(
            session_id=session_id,
            controller_id=controller_id,
            frame_seq=seq,
            controls=ctrls,
        )
        for host, port, node_id in dests:
            frame.target_node_id = node_id
            sock.sendto(frame.to_json_bytes(), (host, port))
        dest_s = "; ".join(f"{h}:{p}/{n}" for h, p, n in dests)
        log.log(
            f"demo frame_seq={seq} pan16={ctrls.pan16} tilt16={ctrls.tilt16} "
            f"intensity={ctrls.intensity} -> {dest_s}",
            also_stderr=True,
        )
        seq += 1
        time.sleep(max(0.001, 1.0 / hz))


def run_static(
    sock: socket.socket,
    dests: list[tuple[str, int, str]],
    controller_id: str,
    session_id: str,
    hz: float,
    count: int,
    ctrls: AbstractControls,
    log: SimLogger,
) -> None:
    seq = 0
    for _ in range(count):
        frame = ControlFrameV1(session_id=session_id, controller_id=controller_id, frame_seq=seq, controls=ctrls)
        for host, port, node_id in dests:
            frame.target_node_id = node_id
            sock.sendto(frame.to_json_bytes(), (host, port))
        log.log(f"static seq={seq} controls={ctrls.to_wire_dict()}", also_stderr=True)
        seq += 1
        if count > 1:
            time.sleep(1.0 / hz)


def run_script(
    sock: socket.socket,
    dests: list[tuple[str, int, str]],
    controller_id: str,
    session_id: str,
    path: Path,
    log: SimLogger,
) -> None:
    raw = json.loads(path.read_text(encoding="utf-8"))
    if isinstance(raw, dict):
        raw = raw.get("frames", raw)
    seq = 0
    for entry in raw:
        if not isinstance(entry, dict):
            continue
        delay_ms = int(entry.get("delay_ms", entry.get("delay", 50)))
        c = entry.get("controls") or entry
        pan16 = c.get("pan16")
        tilt16 = c.get("tilt16")
        if pan16 is None and c.get("pan") is not None:
            pan16 = norm_to_u16(c.get("pan"))
        if tilt16 is None and c.get("tilt") is not None:
            tilt16 = norm_to_u16(c.get("tilt"))
        ctrls = AbstractControls(
            pan16=clamp16(pan16) if pan16 is not None else None,
            tilt16=clamp16(tilt16) if tilt16 is not None else None,
            intensity=clamp8(c.get("intensity")) if c.get("intensity") is not None else None,
            iris=clamp8(c.get("iris")) if c.get("iris") is not None else None,
            zoom=clamp8(c.get("zoom")) if c.get("zoom") is not None else None,
            focus=clamp8(c.get("focus")) if c.get("focus") is not None else None,
            shutter=clamp8(c.get("shutter")) if c.get("shutter") is not None else None,
            color=clamp8(c.get("color")) if c.get("color") is not None else None,
        )
        frame = ControlFrameV1(session_id=session_id, controller_id=controller_id, frame_seq=seq, controls=ctrls)
        for host, port, node_id in dests:
            frame.target_node_id = node_id
            sock.sendto(frame.to_json_bytes(), (host, port))
        log.log(f"script seq={seq} {ctrls.to_wire_dict()}", also_stderr=True)
        seq += 1
        time.sleep(delay_ms / 1000.0)


def run_interactive(
    sock: socket.socket,
    dests: list[tuple[str, int, str]],
    controller_id: str,
    session_id: str,
    log: SimLogger,
) -> None:
    state = AbstractControls(pan16=32768, tilt16=32768, intensity=255)
    seq = 0
    log.log("interactive: commands: pan <0-65535>, tilt <0-65535>, intensity <0-255>, iris, zoom, send, quit", also_stderr=True)
    while True:
        try:
            line = input("sim> ").strip()
        except EOFError:
            break
        if not line or line.startswith("#"):
            continue
        parts = line.split()
        cmd = parts[0].lower()
        if cmd == "quit":
            break
        if cmd == "send":
            frame = ControlFrameV1(session_id=session_id, controller_id=controller_id, frame_seq=seq, controls=state)
            for host, port, node_id in dests:
                frame.target_node_id = node_id
                sock.sendto(frame.to_json_bytes(), (host, port))
            log.log(f"sent seq={seq} {state.to_wire_dict()}", also_stderr=True)
            seq += 1
            continue
        if len(parts) < 2:
            continue
        val = int(parts[1])
        if cmd == "pan":
            state.pan16 = clamp16(val)
        elif cmd == "tilt":
            state.tilt16 = clamp16(val)
        elif cmd == "intensity":
            state.intensity = clamp8(val)
        elif cmd == "iris":
            state.iris = clamp8(val)
        elif cmd == "zoom":
            state.zoom = clamp8(val)
        else:
            log.log(f"unknown: {line}", also_stderr=True)


def main() -> None:
    ap = argparse.ArgumentParser(description="SIGHTLINE virtual controller (UDP JSON control frames)")
    ap.add_argument(
        "--dest",
        action="append",
        metavar="HOST:PORT:NODE_ID",
        help=f"Destination(s). Default 127.0.0.1:{DEFAULT_CONTROL_PORT}:fixture-1",
    )
    ap.add_argument("--controller-id", default="sim-controller")
    ap.add_argument("--session-id", default="sim-session")
    ap.add_argument("--rate", type=float, default=30.0, help="Frames per second (demo/static repeat)")
    ap.add_argument("--log-dir", type=Path, default=None)
    mode = ap.add_mutually_exclusive_group(required=True)
    mode.add_argument("--demo", action="store_true", help="Sine/cosine pan/tilt sweep")
    mode.add_argument("--static", action="store_true", help="Send --pan/--tilt/... values")
    mode.add_argument("--script", type=Path, help="JSON script of frames")
    mode.add_argument("--interactive", action="store_true", help="stdin command loop")
    ap.add_argument("--duration", type=float, default=8.0, help="Demo duration seconds (0 = run forever)")
    ap.add_argument("--count", type=int, default=1, help="Static mode: repeat count")
    ap.add_argument("--pan", type=int, default=32768, help="Pan 16-bit value 0..65535")
    ap.add_argument("--tilt", type=int, default=32768, help="Tilt 16-bit value 0..65535")
    ap.add_argument("--intensity", type=int, default=255)
    ap.add_argument("--iris", type=int, default=128)
    ap.add_argument("--zoom", type=int, default=200)
    args = ap.parse_args()

    if args.dest:
        dests = [parse_dest(d) for d in args.dest]
    else:
        dests = [("127.0.0.1", DEFAULT_CONTROL_PORT, "fixture-1")]

    log = SimLogger("controller", args.log_dir)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    try:
        if args.demo:
            run_demo(
                sock,
                dests,
                args.controller_id,
                args.session_id,
                args.rate,
                args.duration,
                log,
            )
        elif args.static:
            ctrls = AbstractControls(
                pan16=clamp16(args.pan),
                tilt16=clamp16(args.tilt),
                intensity=clamp8(args.intensity),
                iris=clamp8(args.iris),
                zoom=clamp8(args.zoom),
            )
            run_static(
                sock,
                dests,
                args.controller_id,
                args.session_id,
                args.rate,
                args.count,
                ctrls,
                log,
            )
        elif args.script:
            run_script(sock, dests, args.controller_id, args.session_id, args.script, log)
        else:
            run_interactive(sock, dests, args.controller_id, args.session_id, log)
    finally:
        sock.close()
        log.close()


if __name__ == "__main__":
    main()
