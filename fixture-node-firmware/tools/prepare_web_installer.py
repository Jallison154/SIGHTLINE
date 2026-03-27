#!/usr/bin/env python3
"""
Prepare browser-installable binaries + ESP Web Tools manifest for SIGHTLINE.

Usage:
  python3 tools/prepare_web_installer.py --env esp32_eth_dev
"""

from __future__ import annotations

import argparse
import json
import shutil
from pathlib import Path


def copy_required(src: Path, dst: Path) -> None:
    if not src.exists():
        raise FileNotFoundError(
            f"Missing required build artifact: {src}\n"
            "Run firmware build first: python3 -m platformio run -e esp32_eth_dev"
        )
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, dst)


def main() -> None:
    ap = argparse.ArgumentParser(description="Prepare SIGHTLINE web installer assets")
    ap.add_argument("--env", default="esp32_eth_dev", help="PlatformIO env name")
    args = ap.parse_args()

    proj = Path(__file__).resolve().parents[1]
    build_dir = proj / ".pio" / "build" / args.env
    out_dir = proj.parent / "web-installer" / "fixture-node" / "assets" / args.env
    manifest_path = proj.parent / "web-installer" / "fixture-node" / "manifest.json"

    try:
        copy_required(build_dir / "bootloader.bin", out_dir / "bootloader.bin")
        copy_required(build_dir / "partitions.bin", out_dir / "partitions.bin")
        firmware_src = build_dir / "firmware.bin"
        copy_required(firmware_src, out_dir / "firmware.bin")
    except FileNotFoundError as e:
        print(str(e))
        print("If build failed, scroll up and fix compile errors first.")
        raise SystemExit(1) from e

    # buildfs produces littlefs.bin when board_build.filesystem=littlefs
    littlefs_src = build_dir / "littlefs.bin"
    littlefs_present = littlefs_src.exists()
    try:
        if littlefs_present:
            # Guard against stale littlefs.bin from a previous successful run.
            if littlefs_src.stat().st_mtime < firmware_src.stat().st_mtime:
                raise FileNotFoundError(
                    f"Stale littlefs.bin detected: {littlefs_src}\n"
                    "Run: python3 -m platformio run -e esp32_eth_dev -t buildfs"
                )
            copy_required(littlefs_src, out_dir / "littlefs.bin")
    except FileNotFoundError as e:
        print(str(e))
        raise SystemExit(1) from e

    manifest = {
        "name": "SIGHTLINE Fixture Node",
        "version": "dev",
        "home_assistant_domain": "sightline",
        "new_install_prompt_erase": True,
        "builds": [
            {
                "chipFamily": "ESP32",
                "parts": [
                    {"path": f"assets/{args.env}/bootloader.bin", "offset": 0x1000},
                    {"path": f"assets/{args.env}/partitions.bin", "offset": 0x8000},
                    {"path": f"assets/{args.env}/firmware.bin", "offset": 0x10000},
                ],
            }
        ],
    }
    if littlefs_present:
        # Must match fixture-node-firmware/partitions_sightline.csv
        manifest["builds"][0]["parts"].append({"path": f"assets/{args.env}/littlefs.bin", "offset": 0x190000})

    manifest_path.parent.mkdir(parents=True, exist_ok=True)
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"Prepared installer assets in: {out_dir}")
    print(f"Wrote manifest: {manifest_path}")
    if not littlefs_present:
        print("NOTE: littlefs.bin not found. Run `pio run -e <env> -t buildfs` first for full web UI install.")


if __name__ == "__main__":
    main()
