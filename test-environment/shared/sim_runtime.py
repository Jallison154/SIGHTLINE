"""
SIGHTLINE local runtime simulation core.

Keeps business logic separate from HTTP transport so the model can be reused later.
"""

from __future__ import annotations

import math
import threading
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Dict, List, Optional

from .control_protocol import AbstractControls
from .dmx_buffer import DmxBuffer
from .dmx_mapper import apply_abstract_to_dmx
from .fixture_profile import FixtureProfile, load_profile
from .logging_utils import SimLogger
from .num_utils import clamp8, clamp16, norm_to_u16


def _now_ms() -> int:
    return int(time.time() * 1000)


def _parse_int_or_none(value: Any) -> Optional[int]:
    try:
        if value is None:
            return None
        return int(value)
    except (TypeError, ValueError):
        return None


@dataclass
class FixtureNodeSim:
    node_id: str
    friendly_name: str
    fixture_label: str
    ip: str
    firmware_version: str
    profile_dir: Path
    active_profile_id: str
    profiles: Dict[str, FixtureProfile] = field(default_factory=dict)
    profile_sources: Dict[str, Dict[str, Any]] = field(default_factory=dict)
    claimed: bool = False
    assigned_controller_id: str = ""
    online: bool = True
    dmx: DmxBuffer = field(default_factory=DmxBuffer)
    last_control_update_ms: int = 0
    signal_status: str = "No signal"
    _last_controls: AbstractControls = field(default_factory=lambda: AbstractControls(pan16=32768, tilt16=32768))

    def load_profiles(self) -> None:
        self.profiles.clear()
        self.profile_sources.clear()
        for p in sorted(self.profile_dir.glob("*.json")):
            prof, err = load_profile(p)
            if prof:
                self.profiles[prof.id] = prof
                self.profile_sources[prof.id] = {
                    "id": prof.id,
                    "fixtureName": prof.fixture_name,
                    "modeName": prof.mode_name,
                    "channels": {
                        "pan": {"coarse": prof.pan.coarse, "fine": prof.pan.fine},
                        "tilt": {"coarse": prof.tilt.coarse, "fine": prof.tilt.fine},
                        "intensity": {"channel": prof.dimmer.channel},
                        "iris": {"channel": prof.iris.channel},
                        "zoom": {"channel": prof.zoom.channel},
                        "focus": {"channel": prof.focus.channel},
                        "shutter": {"channel": prof.shutter.channel},
                        "color": {"channel": prof.color.channel},
                    },
                }
            elif err:
                # Skip invalid profiles but keep simulation running.
                continue
        if self.active_profile_id not in self.profiles and self.profiles:
            self.active_profile_id = next(iter(self.profiles.keys()))

    def active_profile(self) -> Optional[FixtureProfile]:
        return self.profiles.get(self.active_profile_id)

    def supported_controls(self) -> List[str]:
        p = self.active_profile()
        if not p:
            return []
        supported = ["pan", "tilt"]
        if p.dimmer.channel:
            supported.append("intensity")
        if p.iris.channel:
            supported.append("iris")
        if p.zoom.channel:
            supported.append("zoom")
        if p.focus.channel:
            supported.append("focus")
        if p.shutter.channel:
            supported.append("shutter")
        if p.color.channel:
            supported.append("color")
        return supported

    def apply_controls(self, incoming: AbstractControls) -> None:
        self._last_controls = self._last_controls.merge_from(incoming)
        p = self.active_profile()
        if not p:
            return
        apply_abstract_to_dmx(p, self._last_controls, self.dmx)
        self.last_control_update_ms = _now_ms()
        self.signal_status = "Receiving"

    def status_payload(self) -> Dict[str, Any]:
        pan = clamp16(self._last_controls.pan16 if self._last_controls.pan16 is not None else 32768)
        tilt = clamp16(self._last_controls.tilt16 if self._last_controls.tilt16 is not None else 32768)
        intensity = clamp8(self._last_controls.intensity if self._last_controls.intensity is not None else 0)
        return {
            "nodeId": self.node_id,
            "networkReady": self.online,
            "ip": self.ip,
            "firmwareVersion": self.firmware_version,
            "assignedControllerId": self.assigned_controller_id,
            "claimed": self.claimed,
            "controlRxReady": self.signal_status == "Receiving",
            "lastControlUpdateMs": self.last_control_update_ms,
            "signalStatus": self.signal_status,
            "pan": pan,
            "tilt": tilt,
            "intensity": intensity,
            "dmxPreview": list(self.dmx.snapshot()[:16]),
        }


@dataclass
class ControllerState:
    controller_id: str = "sim-controller-1"
    input_mode: str = "live_mixed"
    active_target_node_id: str = "fixture-1"
    claim: bool = False
    sensitivity: int = 100
    blackout: bool = False
    controls: Dict[str, Any] = field(
        default_factory=lambda: {
            "pan16": 32768,
            "tilt16": 32768,
            "intensity": 255,
            "iris": 128,
            "zoom": 200,
            "focus": 120,
            "shutter": 255,
            "color": 0,
        }
    )
    per_control_source: Dict[str, str] = field(
        default_factory=lambda: {k: "init" for k in ["pan16", "tilt16", "intensity", "iris", "zoom", "focus", "shutter", "color"]}
    )
    last_update_source: str = "init"
    last_update_ms: int = field(default_factory=_now_ms)


class SimRuntime:
    """Owns nodes, controller state, profile registry, discovery and routing."""

    def __init__(self, profile_dir: Path, log_dir: Path, logger_name: str = "sim-runtime"):
        self._lock = threading.RLock()
        self.profile_dir = profile_dir
        self.log = SimLogger(logger_name, log_dir)
        self.controller = ControllerState()
        self.nodes: Dict[str, FixtureNodeSim] = {}
        self._stop = threading.Event()
        self._physical_thread: Optional[threading.Thread] = None
        self._route_thread: Optional[threading.Thread] = None
        self._last_routed_ms = 0

    def bootstrap_default_nodes(self) -> None:
        with self._lock:
            specs = [
                ("fixture-1", "Fixture Node A", "Front Truss A", "127.0.0.1", "sim-v1.0"),
                ("fixture-2", "Fixture Node B", "Front Truss B", "127.0.0.1", "sim-v1.0"),
            ]
            for node_id, name, label, ip, fw in specs:
                n = FixtureNodeSim(
                    node_id=node_id,
                    friendly_name=name,
                    fixture_label=label,
                    ip=ip,
                    firmware_version=fw,
                    profile_dir=self.profile_dir,
                    active_profile_id="sightline_demo_mh_16ch",
                )
                n.load_profiles()
                self.nodes[node_id] = n
            if self.controller.active_target_node_id not in self.nodes:
                self.controller.active_target_node_id = next(iter(self.nodes.keys()))
            self.log.log(f"discovery bootstrap: {len(self.nodes)} nodes registered")

    def start(self) -> None:
        self.bootstrap_default_nodes()
        self._physical_thread = threading.Thread(target=self._physical_input_loop, daemon=True)
        self._route_thread = threading.Thread(target=self._routing_loop, daemon=True)
        self._physical_thread.start()
        self._route_thread.start()
        self.log.log("sim runtime started")

    def stop(self) -> None:
        self._stop.set()
        self.log.log("sim runtime stopping")
        self.log.close()

    def _physical_input_loop(self) -> None:
        # TODO(HW): Replace with real hardware input service bridge when controller firmware matures.
        t0 = time.monotonic()
        while not self._stop.is_set():
            with self._lock:
                if self.controller.input_mode == "live_mixed":
                    t = time.monotonic() - t0
                    pan = 0.5 + 0.45 * math.sin(t * 0.17 * 2.0 * math.pi)
                    tilt = 0.5 + 0.35 * math.cos(t * 0.11 * 2.0 * math.pi)
                    self._set_control("pan16", norm_to_u16(pan), "encoder")
                    self._set_control("tilt16", norm_to_u16(tilt), "encoder")
            time.sleep(0.1)

    def _routing_loop(self) -> None:
        while not self._stop.is_set():
            self.route_controls_once()
            time.sleep(0.04)  # ~25Hz control routing

    def _set_control(self, key: str, value: Any, source: str) -> None:
        if key in ("pan16", "tilt16"):
            value = clamp16(value)
        elif key in ("intensity", "iris", "zoom", "focus", "shutter", "color"):
            value = clamp8(value)
        self.controller.controls[key] = value
        self.controller.per_control_source[key] = source
        self.controller.last_update_source = source
        self.controller.last_update_ms = _now_ms()

    def route_controls_once(self) -> None:
        with self._lock:
            node = self.nodes.get(self.controller.active_target_node_id)
            if not node or not node.online:
                return
            if self.controller.claim and node.assigned_controller_id != self.controller.controller_id:
                node.claimed = True
                node.assigned_controller_id = self.controller.controller_id
            if node.assigned_controller_id and node.assigned_controller_id != self.controller.controller_id:
                return

            c = self.controller.controls
            incoming = AbstractControls(
                pan16=int(c["pan16"]),
                tilt16=int(c["tilt16"]),
                intensity=int(c["intensity"]),
                iris=int(c["iris"]),
                zoom=int(c["zoom"]),
                focus=int(c["focus"]),
                shutter=int(c["shutter"]),
                color=int(c["color"]),
            )
            if self.controller.blackout:
                incoming.intensity = 0

            node.apply_controls(incoming)
            self._last_routed_ms = _now_ms()

    # ---------- Controller endpoints ----------
    def get_state(self) -> Dict[str, Any]:
        with self._lock:
            d = {
                **self.controller.controls,
                # Expose pan/tilt as canonical 16-bit integers for UI/API consumers.
                "pan": clamp16(self.controller.controls["pan16"]),
                "tilt": clamp16(self.controller.controls["tilt16"]),
                "activeTargetNodeId": self.controller.active_target_node_id,
                "inputMode": self.controller.input_mode,
                "blackout": self.controller.blackout,
                "sensitivity": self.controller.sensitivity,
                "lastUpdateMs": self.controller.last_update_ms,
                "lastUpdateSource": self.controller.last_update_source,
                "perControlSource": {
                    **self.controller.per_control_source,
                    "pan": self.controller.per_control_source.get("pan16", "init"),
                    "tilt": self.controller.per_control_source.get("tilt16", "init"),
                },
            }
            return d

    def patch_state(self, patch: Dict[str, Any]) -> Dict[str, Any]:
        with self._lock:
            if "inputMode" in patch:
                self.controller.input_mode = str(patch["inputMode"])
                self.controller.last_update_source = "web-mode"
                self.controller.last_update_ms = _now_ms()
            if "blackout" in patch:
                self.controller.blackout = bool(patch["blackout"])
                self.controller.last_update_source = "web-blackout"
                self.controller.last_update_ms = _now_ms()
            if "sensitivity" in patch:
                self.controller.sensitivity = clamp8(patch["sensitivity"])
                self.controller.last_update_source = "web-sensitivity"
                self.controller.last_update_ms = _now_ms()
            if "claim" in patch:
                self.controller.claim = bool(patch["claim"])
                node = self.nodes.get(self.controller.active_target_node_id)
                if node:
                    if self.controller.claim:
                        node.claimed = True
                        node.assigned_controller_id = self.controller.controller_id
                    elif node.assigned_controller_id == self.controller.controller_id:
                        node.claimed = False
                        node.assigned_controller_id = ""
                self.controller.last_update_source = "web-claim"
                self.controller.last_update_ms = _now_ms()

            # Accept both pan16/tilt16 and legacy normalized pan/tilt.
            if "pan16" in patch:
                parsed = _parse_int_or_none(patch["pan16"])
                if parsed is not None:
                    self._set_control("pan16", parsed, "web")
            elif "pan" in patch:
                parsed = _parse_int_or_none(patch["pan"])
                if parsed is not None:
                    self._set_control("pan16", clamp16(parsed), "web")

            if "tilt16" in patch:
                parsed = _parse_int_or_none(patch["tilt16"])
                if parsed is not None:
                    self._set_control("tilt16", parsed, "web")
            elif "tilt" in patch:
                parsed = _parse_int_or_none(patch["tilt"])
                if parsed is not None:
                    self._set_control("tilt16", clamp16(parsed), "web")

            for key in ("intensity", "iris", "zoom", "focus", "shutter", "color"):
                if key in patch:
                    parsed = _parse_int_or_none(patch[key])
                    if parsed is not None:
                        self._set_control(key, parsed, "web")
            return {"ok": True}

    def get_nodes(self) -> Dict[str, Any]:
        with self._lock:
            return {
                "nodes": [
                    {
                        "nodeId": n.node_id,
                        "friendlyName": n.friendly_name,
                        "fixtureLabel": n.fixture_label,
                        "online": n.online,
                        "claimed": n.claimed,
                        "assignedControllerId": n.assigned_controller_id,
                    }
                    for n in self.nodes.values()
                ]
            }

    def select_node(self, node_id: str) -> Dict[str, Any]:
        with self._lock:
            if node_id not in self.nodes:
                return {"ok": False, "error": "node-not-found"}
            self.controller.active_target_node_id = node_id
            self.controller.last_update_source = "web-target"
            self.controller.last_update_ms = _now_ms()
            self.log.log(f"target switch -> {node_id}")
            return {"ok": True, "activeTargetNodeId": node_id}

    def get_target_fixture(self) -> Dict[str, Any]:
        with self._lock:
            n = self.nodes.get(self.controller.active_target_node_id)
            if not n:
                return {"supportedControls": [], "fixtureLabel": "", "profileName": ""}
            return {
                "fixtureLabel": n.fixture_label,
                "profileName": n.active_profile_id,
                "supportedControls": n.supported_controls(),
            }

    def get_controller_status(self) -> Dict[str, Any]:
        with self._lock:
            n = self.nodes.get(self.controller.active_target_node_id)
            return {
                "controllerId": self.controller.controller_id,
                "networkReady": True,
                "discoveryReady": True,
                "ownershipReady": True,
                "controlTxReady": True,
                "controllerStatus": "running",
                "ownsTarget": bool(n and n.assigned_controller_id == self.controller.controller_id),
                "targetNodeId": self.controller.active_target_node_id,
                "signalHealth": "ok",
                "lastUpdateMs": self.controller.last_update_ms,
            }

    # ---------- Node endpoints ----------
    def _node_or_none(self, node_id: str) -> Optional[FixtureNodeSim]:
        return self.nodes.get(node_id)

    def get_node_config(self, node_id: str) -> Dict[str, Any]:
        with self._lock:
            n = self._node_or_none(node_id)
            if not n:
                return {"error": "node-not-found"}
            return {
                "nodeName": n.friendly_name,
                "fixtureLabel": n.fixture_label,
                "universe": 0,
                "dmxStartAddress": 1,
                "dhcp": True,
                "staticIp": n.ip,
                "subnetMask": "255.255.255.0",
                "gateway": "127.0.0.1",
            }

    def save_node_config(self, node_id: str, payload: Dict[str, Any]) -> Dict[str, Any]:
        with self._lock:
            n = self._node_or_none(node_id)
            if not n:
                return {"ok": False, "error": "node-not-found"}
            n.friendly_name = str(payload.get("nodeName") or n.friendly_name)
            n.fixture_label = str(payload.get("fixtureLabel") or n.fixture_label)
            self.log.log(f"node config update: {node_id} name={n.friendly_name} fixture={n.fixture_label}")
            return {"ok": True}

    def get_node_status(self, node_id: str) -> Dict[str, Any]:
        with self._lock:
            n = self._node_or_none(node_id)
            if not n:
                return {"error": "node-not-found"}
            return n.status_payload()

    def get_node_profiles(self, node_id: str) -> Dict[str, Any]:
        with self._lock:
            n = self._node_or_none(node_id)
            if not n:
                return {"profiles": [], "activeProfileId": ""}
            profiles = list(n.profile_sources.values())
            return {"profiles": profiles, "activeProfileId": n.active_profile_id}

    def node_profile_action(self, node_id: str, payload: Dict[str, Any]) -> Dict[str, Any]:
        with self._lock:
            n = self._node_or_none(node_id)
            if not n:
                return {"ok": False, "error": "node-not-found"}
            action = payload.get("action", "")
            if action == "apply":
                pid = str(payload.get("profileId", ""))
                if pid in n.profiles:
                    n.active_profile_id = pid
                    self.log.log(f"node {node_id} applied profile {pid}")
                    return {"ok": True}
                return {"ok": False, "error": "profile-not-found"}
            if action == "save":
                profile = payload.get("profile") or {}
                profile_id = str(profile.get("id") or "edited_profile")
                # TODO(PROFILE): Add full schema + collision checks matching firmware import pipeline.
                n.profile_sources[profile_id] = profile
                prof, _ = load_profile(self.profile_dir / "example_moving_head.json")
                if prof:
                    n.profiles[profile_id] = prof
                return {"ok": True, "profileId": profile_id}
            return {"ok": False, "error": "unsupported-action"}

    def node_profile_import(self, node_id: str, payload: Dict[str, Any]) -> Dict[str, Any]:
        with self._lock:
            n = self._node_or_none(node_id)
            if not n:
                return {"ok": False, "error": "node-not-found"}
            profile_json = payload.get("profileJson") or ""
            file_name = str(payload.get("fileName") or f"import-{_now_ms()}.json")
            temp = self.profile_dir / f".import-{file_name}"
            temp.write_text(profile_json, encoding="utf-8")
            prof, err = load_profile(temp)
            temp.unlink(missing_ok=True)
            if not prof:
                return {"ok": False, "error": err or "invalid-profile"}
            n.profiles[prof.id] = prof
            n.profile_sources[prof.id] = {
                "id": prof.id,
                "fixtureName": prof.fixture_name,
                "modeName": prof.mode_name,
                "channels": {
                    "pan": {"coarse": prof.pan.coarse, "fine": prof.pan.fine},
                    "tilt": {"coarse": prof.tilt.coarse, "fine": prof.tilt.fine},
                },
            }
            self.log.log(f"node {node_id} imported profile {prof.id}")
            return {"ok": True, "profileId": prof.id}

    def node_profile_export(self, node_id: str, profile_id: str) -> Dict[str, Any]:
        with self._lock:
            n = self._node_or_none(node_id)
            if not n:
                return {"ok": False, "error": "node-not-found"}
            src = n.profile_sources.get(profile_id)
            if not src:
                return {"ok": False, "error": "profile-not-found"}
            return src
