"""
512-channel DMX universe buffer (0-based indexing internally).

Each DMX channel stores one byte: 0..255.
Pan/tilt 16-bit support is achieved by splitting a uint16 axis value across
coarse+fine channel pairs (MSB/LSB), not by widening per-channel storage.
"""

from __future__ import annotations

from dataclasses import dataclass, field


@dataclass
class DmxBuffer:
    """DMX channels 1..512 map to indices 0..511."""

    data: bytearray = field(default_factory=lambda: bytearray(512))

    def clear(self) -> None:
        self.data[:] = b"\x00" * 512

    def set8(self, channel_1based: int, value: int) -> None:
        if channel_1based < 1 or channel_1based > 512:
            return
        self.data[channel_1based - 1] = value & 0xFF

    def set16_msb_first(self, coarse_1based: int, value16: int) -> None:
        """
        Match DmxUniverse::setChannel16: MSB at coarse, LSB at coarse+1.
        """
        if coarse_1based < 1 or coarse_1based >= 512:
            return
        v = value16 & 0xFFFF
        self.data[coarse_1based - 1] = (v >> 8) & 0xFF
        self.data[coarse_1based] = v & 0xFF

    def snapshot(self) -> bytes:
        return bytes(self.data)

    def nonzero_summary(self, max_ch: int = 32) -> list[tuple[int, int]]:
        out: list[tuple[int, int]] = []
        for i in range(min(512, max_ch)):
            if self.data[i]:
                out.append((i + 1, self.data[i]))
        return out
