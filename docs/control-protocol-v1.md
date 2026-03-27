# SIGHTLINE Control Protocol v1

## 1) v1 Transport Method

Use **UDP unicast** from controller to claimed fixture node for control frames.

Why:

- low overhead and low latency on local event Ethernet
- simple for ESP32 implementation
- no broker/service dependency
- easy to inspect in Wireshark during development

Notes:

- Keep discovery and ownership on separate message families/ports.
- Control receiver should only accept frames from current owner controller.

## 2) Control Packet / Message Format

v1 payload: JSON envelope with compact keys and optional controls.

Required fields:

- `schema_version`
- `msg_type` = `control_frame`
- `session_id`
- `controller_id`
- `target_node_id`
- `frame_seq`
- `sent_at_ms`
- `controls` object

`controls` values:

- `pan16` (uint16 `0..65535`)
- `tilt16` (uint16 `0..65535`)
- `intensity` (uint8 `0..255`)
- `iris` (uint8 `0..255`)
- `zoom` (uint8 `0..255`)
- `focus` (uint8 `0..255`)
- `shutter` (uint8 `0..255`)
- `color` (uint8 `0..255`)
- `extra` (optional object for future controls)

Notes:

- `0..65535` is the **valid value range** for each 16-bit axis value.
- That range contains **65536 total steps** (`2^16` discrete positions).
- Fixture DMX output still uses two 8-bit bytes:
  - coarse (MSB) `0..255`
  - fine (LSB) `0..255`

## 3) Update Rate Expectations

Recommended control frame rate:

- nominal: `30-40 Hz`
- high-motion ceiling: `50 Hz` if network and node load allow

Behavior expectations:

- Node applies latest valid frame only (last-frame-wins)
- If stream stalls, node follows configured control-loss policy

## 4) Handling Missing Controls

Controls are optional in message payload.

Node mapping rules:

- If fixture profile lacks a parameter channel: skip it.
- If message omits a control:
  - keep last value for short hold window, or
  - apply profile default/safe fallback (policy configurable)

This avoids hard dependencies on fixture personalities.
