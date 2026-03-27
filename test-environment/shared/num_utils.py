"""
Shared numeric normalization helpers for public-facing control values.
"""

from __future__ import annotations


def clamp8(value: int | float | str) -> int:
    return max(0, min(255, int(value)))


def clamp16(value: int | float | str) -> int:
    return max(0, min(65535, int(value)))


def norm_to_u16(value: int | float | str) -> int:
    n = max(0.0, min(1.0, float(value)))
    return int(n * 65535.0)


def int_text(value: int | float | str | None, fallback: int = 0) -> str:
    if value is None:
        return str(fallback)
    return str(int(value))
