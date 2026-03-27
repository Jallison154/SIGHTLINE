"""
Abstract controls → DMX — mirrors controller-firmware/src/DmxMapper.cpp semantics.

Pan/tilt canonical: uint16 0..65535 (65536 total steps).
- 16-bit profile mode (coarse + fine): write MSB to coarse, LSB to fine.
- 8-bit profile mode (coarse only): downscale by using the high byte.

Legacy normalized pan/tilt (0..1) is converted to uint16 for compatibility.
8-bit params: intensity maps to dimmer channel.
"""

from __future__ import annotations

from typing import TYPE_CHECKING

from .control_protocol import AbstractControls
from .dmx_buffer import DmxBuffer
from .num_utils import clamp8, clamp16, norm_to_u16

if TYPE_CHECKING:
    from .fixture_profile import FixtureProfile


def _maybe_invert16(v: int, invert: bool) -> int:
    v &= 0xFFFF
    if not invert:
        return v
    return (65535 - v) & 0xFFFF


def _clamp16(v: int, lo: int, hi: int) -> int:
    return max(lo, min(hi, v))


def _clamp8(v: int, lo: int, hi: int) -> int:
    return max(lo, min(hi, v)) & 0xFF


def apply_abstract_to_dmx(profile: "FixtureProfile", controls: AbstractControls, buf: DmxBuffer) -> None:
    """Fill buffer from abstract controls + profile (last-frame-wins; clears first)."""
    buf.clear()

    p = profile
    c = controls

    # Seed sparse/high-channel fixtures with profile-level defaults first.
    for channel_1based, value in p.default_channel_values.items():
        buf.set8(channel_1based, value)

    if c.pan16 is not None:
        pan16 = clamp16(c.pan16)
    else:
        pan16 = norm_to_u16(c.pan if c.pan is not None else 0.5)

    if c.tilt16 is not None:
        tilt16 = clamp16(c.tilt16)
    else:
        tilt16 = norm_to_u16(c.tilt if c.tilt is not None else 0.5)

    mp = _maybe_invert16(pan16, p.pan.invert)
    if p.pan.has_limits:
        mp = _clamp16(mp, p.pan.min_value, p.pan.max_value)
    if p.pan.coarse > 0 and p.pan.fine > 0:
        buf.set16_msb_first(p.pan.coarse, mp)
    elif p.pan.coarse > 0:
        buf.set8(p.pan.coarse, (mp >> 8) & 0xFF)

    mt = _maybe_invert16(tilt16, p.tilt.invert)
    if p.tilt.has_limits:
        mt = _clamp16(mt, p.tilt.min_value, p.tilt.max_value)
    if p.tilt.coarse > 0 and p.tilt.fine > 0:
        buf.set16_msb_first(p.tilt.coarse, mt)
    elif p.tilt.coarse > 0:
        buf.set8(p.tilt.coarse, (mt >> 8) & 0xFF)

    dim = clamp8(c.intensity if c.intensity is not None else p.default_dimmer)
    if p.dimmer.channel > 0:
        if p.dimmer.has_limits:
            dim = _clamp8(dim, p.dimmer.min_value, p.dimmer.max_value)
        if p.dimmer.invert:
            dim = (255 - dim) & 0xFF
        buf.set8(p.dimmer.channel, dim)

    # Match DmxMapper.cpp: shutter follows profile default, not live control in v1 firmware.
    if p.shutter.channel > 0:
        v = 255 if p.default_shutter_open else 0
        buf.set8(p.shutter.channel, v)

    if p.zoom.channel > 0 and c.zoom is not None:
        z = clamp8(c.zoom)
        if p.zoom.has_limits:
            z = _clamp8(z, p.zoom.min_value, p.zoom.max_value)
        buf.set8(p.zoom.channel, z)

    if p.iris.channel > 0 and c.iris is not None:
        ir = clamp8(c.iris)
        if p.iris.has_limits:
            ir = _clamp8(ir, p.iris.min_value, p.iris.max_value)
        buf.set8(p.iris.channel, ir)

    if p.focus.channel > 0 and c.focus is not None:
        buf.set8(p.focus.channel, clamp8(c.focus))

    if p.color.channel > 0 and c.color is not None:
        buf.set8(p.color.channel, clamp8(c.color))
