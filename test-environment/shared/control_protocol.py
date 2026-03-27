"""
SIGHTLINE Control Protocol v1 — JSON over UDP.

Aligned with docs/control-protocol-v1.md and ControlCodec (nextgen).
"""

from __future__ import annotations

import json
import time
from dataclasses import dataclass, field
from typing import Any, Optional

from .num_utils import clamp8, clamp16, norm_to_u16


@dataclass
class AbstractControls:
    """
    Optional control fields; omitted keys mean 'not present' in the wire payload.

    Canonical pan/tilt representation is 16-bit:
    - pan16:  0..65535
    - tilt16: 0..65535

    Legacy normalized fields (pan/tilt 0..1) are accepted on input for compatibility.
    Outbound wire payloads remain integer-only.
    """

    pan16: Optional[int] = None
    tilt16: Optional[int] = None
    pan: Optional[float] = None
    tilt: Optional[float] = None
    intensity: Optional[int] = None
    iris: Optional[int] = None
    zoom: Optional[int] = None
    focus: Optional[int] = None
    shutter: Optional[int] = None
    color: Optional[int] = None
    extra: Optional[dict[str, Any]] = None

    def to_wire_dict(self) -> dict[str, Any]:
        c: dict[str, Any] = {}
        if self.pan16 is not None:
            c["pan16"] = clamp16(self.pan16)
        if self.tilt16 is not None:
            c["tilt16"] = clamp16(self.tilt16)
        if self.intensity is not None:
            c["intensity"] = clamp8(self.intensity)
        if self.iris is not None:
            c["iris"] = clamp8(self.iris)
        if self.zoom is not None:
            c["zoom"] = clamp8(self.zoom)
        if self.focus is not None:
            c["focus"] = clamp8(self.focus)
        if self.shutter is not None:
            c["shutter"] = clamp8(self.shutter)
        if self.color is not None:
            c["color"] = clamp8(self.color)
        if self.extra:
            c["extra"] = self.extra
        return c

    def merge_from(self, other: "AbstractControls") -> "AbstractControls":
        """Last-frame-wins for each field present in other."""
        return AbstractControls(
            pan16=other.pan16 if other.pan16 is not None else self.pan16,
            tilt16=other.tilt16 if other.tilt16 is not None else self.tilt16,
            pan=other.pan if other.pan is not None else self.pan,
            tilt=other.tilt if other.tilt is not None else self.tilt,
            intensity=other.intensity if other.intensity is not None else self.intensity,
            iris=other.iris if other.iris is not None else self.iris,
            zoom=other.zoom if other.zoom is not None else self.zoom,
            focus=other.focus if other.focus is not None else self.focus,
            shutter=other.shutter if other.shutter is not None else self.shutter,
            color=other.color if other.color is not None else self.color,
            extra=other.extra if other.extra is not None else self.extra,
        )

    @staticmethod
    def from_wire_dict(d: dict[str, Any]) -> "AbstractControls":
        ex = d.get("extra")
        pan16 = d.get("pan16")
        tilt16 = d.get("tilt16")
        if pan16 is None and d.get("pan") is not None:
            pan16 = norm_to_u16(d.get("pan"))
        if tilt16 is None and d.get("tilt") is not None:
            tilt16 = norm_to_u16(d.get("tilt"))
        return AbstractControls(
            pan16=clamp16(pan16) if pan16 is not None else None,
            tilt16=clamp16(tilt16) if tilt16 is not None else None,
            pan=None,
            tilt=None,
            intensity=clamp8(d.get("intensity")) if d.get("intensity") is not None else None,
            iris=clamp8(d.get("iris")) if d.get("iris") is not None else None,
            zoom=clamp8(d.get("zoom")) if d.get("zoom") is not None else None,
            focus=clamp8(d.get("focus")) if d.get("focus") is not None else None,
            shutter=clamp8(d.get("shutter")) if d.get("shutter") is not None else None,
            color=clamp8(d.get("color")) if d.get("color") is not None else None,
            extra=ex if isinstance(ex, dict) else None,
        )


@dataclass
class ControlFrameV1:
    schema_version: int = 1
    msg_type: str = "control_frame"
    session_id: str = "sim-session"
    controller_id: str = "sim-controller"
    target_node_id: str = "fixture-1"
    frame_seq: int = 0
    sent_at_ms: int = 0
    controls: AbstractControls = field(default_factory=AbstractControls)

    def to_json_bytes(self) -> bytes:
        self.sent_at_ms = int(time.time() * 1000) % (2**32)
        doc: dict[str, Any] = {
            "schema_version": self.schema_version,
            "msg_type": self.msg_type,
            "session_id": self.session_id,
            "controller_id": self.controller_id,
            "target_node_id": self.target_node_id,
            "frame_seq": self.frame_seq,
            "sent_at_ms": self.sent_at_ms,
            "controls": self.controls.to_wire_dict(),
        }
        return json.dumps(doc, separators=(",", ":")).encode("utf-8")

    @staticmethod
    def from_json_bytes(data: bytes) -> tuple[Optional["ControlFrameV1"], Optional[str]]:
        try:
            doc = json.loads(data.decode("utf-8"))
        except (UnicodeDecodeError, json.JSONDecodeError) as e:
            return None, str(e)

        if doc.get("schema_version") != 1 or doc.get("msg_type") != "control_frame":
            return None, "invalid envelope"
        if not doc.get("controller_id") or not doc.get("target_node_id"):
            return None, "missing controller_id or target_node_id"

        c = doc.get("controls") or {}
        if not isinstance(c, dict):
            return None, "controls must be object"

        frame = ControlFrameV1(
            schema_version=1,
            msg_type="control_frame",
            session_id=str(doc.get("session_id", "")),
            controller_id=str(doc["controller_id"]),
            target_node_id=str(doc["target_node_id"]),
            frame_seq=int(doc.get("frame_seq", 0)),
            sent_at_ms=int(doc.get("sent_at_ms", 0)),
            controls=AbstractControls.from_wire_dict(c),
        )
        return frame, None
