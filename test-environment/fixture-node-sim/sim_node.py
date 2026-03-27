#!/usr/bin/env python3
"""
Virtual fixture node: UDP JSON control frames → fixture profile → 512-channel DMX.

Filters by --node-id (must match target_node_id in frames).
Optional Art-Net output to QLC+ (localhost:6454).
"""

from __future__ import annotations

import argparse
import socket
import sys
import time
from pathlib import Path
from typing import Optional

_ROOT = Path(__file__).resolve().parents[1]
if str(_ROOT) not in sys.path:
    sys.path.insert(0, str(_ROOT))

from shared.artnet_out import send_artdmx
from shared.control_protocol import AbstractControls, ControlFrameV1
from shared.dmx_buffer import DmxBuffer
from shared.dmx_mapper import apply_abstract_to_dmx
from shared.fixture_profile import load_profile
from shared.logging_utils import SimLogger
from shared.num_utils import int_text


def merge_abstract(prev: AbstractControls, inc: AbstractControls) -> AbstractControls:
    return prev.merge_from(inc)


def format_dmx_preview(buf: DmxBuffer, max_ch: int = 24) -> str:
    parts = []
    for i in range(min(max_ch, 512)):
        v = buf.data[i]
        if v:
            parts.append(f"{i + 1}:{v}")
    return " ".join(parts) if parts else "(all zero)"


def main() -> None:
    ap = argparse.ArgumentParser(description="SIGHTLINE virtual fixture node")
    ap.add_argument("--bind", default="0.0.0.0", help="Listen address")
    ap.add_argument("--port", type=int, default=6470, help="UDP port for control frames")
    ap.add_argument("--node-id", default="fixture-1", dest="node_id", help="Must match controller target_node_id")
    ap.add_argument("--profile", type=Path, required=True, help="Fixture profile JSON")
    ap.add_argument("--log-dir", type=Path, default=None)
    ap.add_argument("--verbose", action="store_true", help="Log every accepted frame")
    ap.add_argument("--quiet", action="store_true", help="Only log DMX changes / periodic summary")
    ap.add_argument("--summary-interval", type=float, default=1.0, help="Seconds between summaries when not verbose")
    ap.add_argument("--artnet", action="store_true", help="Mirror DMX to Art-Net (127.0.0.1:6454)")
    ap.add_argument("--artnet-host", default="127.0.0.1")
    ap.add_argument("--artnet-universe", type=int, default=0)
    args = ap.parse_args()

    prof, err = load_profile(args.profile)
    if not prof:
        print(f"Profile error: {err}", file=sys.stderr)
        sys.exit(1)

    log = SimLogger(f"node-{args.node_id}", args.log_dir)
    log.log(
        f"start node_id={args.node_id} profile={args.profile} id={prof.id} "
        f"{prof.fixture_name} ({prof.mode_name}) bind={args.bind}:{args.port}",
        also_stderr=True,
    )

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind((args.bind, args.port))

    state = AbstractControls(pan16=32768, tilt16=32768)
    buf = DmxBuffer()
    last_snapshot: Optional[bytes] = None
    artnet_seq = 0
    last_summary = time.monotonic()
    frames_ok = 0
    frames_ignored = 0

    try:
        while True:
            data, addr = sock.recvfrom(4096)
            frame, perr = ControlFrameV1.from_json_bytes(data)
            if not frame:
                frames_ignored += 1
                log.log(f"bad packet from {addr}: {perr}", also_stderr=not args.quiet)
                continue
            if frame.target_node_id != args.node_id:
                frames_ignored += 1
                if args.verbose:
                    log.log(f"ignore target={frame.target_node_id!r} (want {args.node_id!r})", also_stderr=True)
                continue

            state = merge_abstract(state, frame.controls)
            apply_abstract_to_dmx(prof, state, buf)
            snap = buf.snapshot()
            frames_ok += 1

            pan_v = int(state.pan16) if state.pan16 is not None else 32768
            tilt_v = int(state.tilt16) if state.tilt16 is not None else 32768
            line = (
                f"seq={frame.frame_seq} pan16={pan_v} tilt16={tilt_v} "
                f"intensity={int_text(state.intensity)} dmx={format_dmx_preview(buf)}"
            )

            if args.verbose:
                log.log(line, also_stderr=True)
            elif snap != last_snapshot:
                log.log(f"DMX change: {line}", also_stderr=True)
                last_snapshot = snap
            else:
                now = time.monotonic()
                if now - last_summary >= args.summary_interval:
                    log.log(
                        f"summary ok={frames_ok} ignored={frames_ignored} last_seq={frame.frame_seq} "
                        f"pan16={pan_v} tilt16={tilt_v} {format_dmx_preview(buf)}",
                        also_stderr=True,
                    )
                    last_summary = now

            if args.artnet:
                send_artdmx(
                    snap,
                    host=args.artnet_host,
                    universe=args.artnet_universe,
                    sequence=artnet_seq & 0xFF,
                )
                artnet_seq += 1
    except KeyboardInterrupt:
        log.log("shutdown", also_stderr=True)
    finally:
        sock.close()
        log.close()


if __name__ == "__main__":
    main()
