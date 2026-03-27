"""
Fixture profile JSON — load, validate, and structured access.

Aligned with shared/profiles/fixture-profile.schema.json and FixtureProfileParser.cpp.
"""

from __future__ import annotations

import json
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Optional


@dataclass
class Channel16:
    coarse: int = 0
    fine: int = 0
    invert: bool = False
    has_limits: bool = False
    min_value: int = 0
    max_value: int = 65535


@dataclass
class Channel8Opt:
    channel: int = 0
    invert: bool = False
    has_limits: bool = False
    min_value: int = 0
    max_value: int = 255


@dataclass
class FixtureProfile:
    id: str = ""
    fixture_name: str = ""
    manufacturer: str = ""
    mode_name: str = ""
    channel_count: int = 0
    pan: Channel16 = field(default_factory=Channel16)
    tilt: Channel16 = field(default_factory=Channel16)
    dimmer: Channel8Opt = field(default_factory=Channel8Opt)
    shutter: Channel8Opt = field(default_factory=Channel8Opt)
    zoom: Channel8Opt = field(default_factory=Channel8Opt)
    focus: Channel8Opt = field(default_factory=Channel8Opt)
    iris: Channel8Opt = field(default_factory=Channel8Opt)
    color: Channel8Opt = field(default_factory=Channel8Opt)
    default_shutter_open: bool = True
    default_dimmer: int = 255
    # Optional defaults for sparse/high-channel fixtures: channel -> 0..255
    default_channel_values: dict[int, int] = field(default_factory=dict)
    # Optional explicit unsupported control names (UI/service hinting).
    unsupported_controls: list[str] = field(default_factory=list)


def _u16(v: Any, default: int = 0) -> int:
    if isinstance(v, int):
        return v
    if isinstance(v, float):
        return int(v)
    return default


def _parse_ch8_opt(ch: dict[str, Any]) -> Channel8Opt:
    lim = ch.get("limits") or {}
    has_lim = isinstance(lim.get("min"), int) and isinstance(lim.get("max"), int)
    return Channel8Opt(
        channel=_u16(ch.get("channel")),
        invert=bool(ch.get("invert", False)),
        has_limits=has_lim,
        min_value=int(lim["min"]) if has_lim else 0,
        max_value=int(lim["max"]) if has_lim else 255,
    )


def _parse_ch16_req(ch: dict[str, Any]) -> Channel16:
    lim = ch.get("limits") or {}
    has_lim = isinstance(lim.get("min"), int) and isinstance(lim.get("max"), int)
    return Channel16(
        coarse=_u16(ch.get("coarse")),
        fine=_u16(ch.get("fine")),
        invert=bool(ch.get("invert", False)),
        has_limits=has_lim,
        min_value=int(lim["min"]) if has_lim else 0,
        max_value=int(lim["max"]) if has_lim else 65535,
    )


def load_profile(path: str | Path) -> tuple[Optional[FixtureProfile], Optional[str]]:
    p = Path(path)
    try:
        raw = p.read_text(encoding="utf-8")
    except OSError as e:
        return None, str(e)
    try:
        doc = json.loads(raw)
    except json.JSONDecodeError as e:
        return None, str(e)
    return parse_profile_dict(doc)


def parse_profile_dict(doc: dict[str, Any]) -> tuple[Optional[FixtureProfile], Optional[str]]:
    ch = doc.get("channels") or {}
    if not isinstance(ch, dict):
        return None, "channels must be object"
    if "pan" not in ch or "tilt" not in ch:
        return None, "channels.pan and channels.tilt required"

    # Optional flexible logical mappings can supplement/override channel keys.
    logical = doc.get("logical_mappings") or {}
    if isinstance(logical, dict):
        merged = dict(ch)
        for k, v in logical.items():
            if isinstance(v, dict):
                merged[k] = v
        ch = merged

    prof = FixtureProfile(
        id=str(doc.get("id", "")),
        fixture_name=str(doc.get("fixture_name", "")),
        manufacturer=str(doc.get("manufacturer", "")),
        mode_name=str(doc.get("mode_name", "")),
        channel_count=_u16(doc.get("channel_count")),
        pan=_parse_ch16_req(ch["pan"]),
        tilt=_parse_ch16_req(ch["tilt"]),
    )

    defaults = doc.get("defaults") or {}
    if isinstance(defaults, dict):
        prof.default_shutter_open = bool(defaults.get("shutter_open", True))
        prof.default_dimmer = int(defaults.get("dimmer", 255)) & 0xFF
        chvals = defaults.get("channel_values") or {}
        if isinstance(chvals, dict):
            for key, val in chvals.items():
                try:
                    chan = int(key)
                except (TypeError, ValueError):
                    continue
                if 1 <= chan <= 512 and isinstance(val, int):
                    prof.default_channel_values[chan] = val & 0xFF

    u = doc.get("unsupported_controls") or []
    if isinstance(u, list):
        prof.unsupported_controls = [str(x) for x in u if isinstance(x, str) and x.strip()]

    for key, attr in (
        ("dimmer", "dimmer"),
        ("shutter", "shutter"),
        ("zoom", "zoom"),
        ("focus", "focus"),
        ("iris", "iris"),
        ("color", "color"),
    ):
        if key in ch and isinstance(ch[key], dict):
            setattr(prof, attr, _parse_ch8_opt(ch[key]))

    err = validate_profile(prof)
    if err:
        return None, err
    return prof, None


def validate_profile(p: FixtureProfile) -> Optional[str]:
    if not p.id or not p.fixture_name or not p.manufacturer or not p.mode_name:
        return "id, fixture_name, manufacturer, mode_name required"
    if p.channel_count < 1 or p.channel_count > 512:
        return "channel_count must be 1..512"
    for name, c in (("pan", p.pan), ("tilt", p.tilt)):
        if c.coarse < 1 or c.coarse > 512:
            return f"{name} coarse must be 1..512"
        if c.fine != 0 and (c.fine < 1 or c.fine > 512):
            return f"{name} fine must be 1..512 when provided"
        if c.coarse > p.channel_count:
            return f"{name} coarse exceeds channel_count"
        if c.fine != 0 and c.fine > p.channel_count:
            return f"{name} fine exceeds channel_count"
        if c.has_limits and c.min_value > c.max_value:
            return f"{name} limits invalid"
    for label, c in (
        ("dimmer", p.dimmer),
        ("shutter", p.shutter),
        ("zoom", p.zoom),
        ("focus", p.focus),
        ("iris", p.iris),
        ("color", p.color),
    ):
        if c.channel == 0:
            continue
        if c.channel > p.channel_count:
            return f"{label} channel exceeds channel_count"
        if c.has_limits and c.min_value > c.max_value:
            return f"{label} limits invalid"
    for chan, val in p.default_channel_values.items():
        if chan < 1 or chan > p.channel_count:
            return f"default channel value index {chan} exceeds channel_count"
        if val < 0 or val > 255:
            return f"default channel value at {chan} must be 0..255"
    return None
