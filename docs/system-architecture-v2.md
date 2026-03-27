# SIGHTLINE System Architecture v2

## 1) Redesigned Architecture

SIGHTLINE moves to an abstract networked control model:

- Ground Controller publishes abstract control state (`pan`, `tilt`, `intensity`, `zoom`, etc.).
- Fixture Node receives abstract state, applies selected fixture profile, and renders DMX512.
- Discovery/claim layer coordinates which controller targets which fixture node.

This decouples operator input from fixture DMX personality and improves scalability.

Core DMX assumptions:

- Each DMX channel is always an 8-bit value (`0..255`).
- High channel-count fixtures (including `50+` channel modes) are supported by a full universe buffer.
- Fixture node owns translation from abstract control state -> mapped DMX channel bytes.

## 2) Network Communication Model

### Message Families

- `discovery/*`: announce, heartbeat, presence list
- `control/*`: abstract control frame and timing metadata
- `claim/*`: claim, release, keepalive, claim-state
- `profile/*`: list, select, import, export, validate
- `status/*`: health and telemetry

### Transport Recommendation

- UDP for discovery and high-rate control frames (low latency, non-blocking).
- HTTP/JSON for configuration/profile import-export.
- Optional reliable command channel later (e.g. lightweight ACK protocol).

### Control Frame Shape

- `session_id`, `controller_id`, `target_node_id`
- monotonic `frame_seq`, `sent_at_ms`
- abstract channels map where:
  - `pan16` / `tilt16` are canonical uint16 values (`0..65535`)
  - most other controls remain uint8 (`0..255`)

Pan/tilt output mapping:

- Node uses fixture-profile mapping mode per axis:
  - 16-bit mode (`coarse` + `fine`): coarse = high byte (`value >> 8`), fine = low byte (`value & 0xFF`)
  - 8-bit mode (`coarse` only): output high byte (`value >> 8`) as downscaled axis value

## 3) Discovery Model

- Each device advertises `device_id`, `friendly_name`, `device_type`, `capabilities`, `status`.
- Broadcast/multicast heartbeat at fixed interval.
- Controllers maintain a live registry with timeout-based stale removal.

## 4) Claiming / Assignment Model

- Fixture node has claim state:
  - `unclaimed`
  - `claimed(controller_id, lease_expiry)`
- Controller sends `claim` request and periodic `keepalive`.
- Node auto-releases on lease timeout.
- `release` explicitly clears claim.
- Future extension: priority-based claims and shared/group control modes.

## 5) Fixture Profile Architecture

- Profile schema remains JSON with abstract attributes mapped to DMX channels.
- Profile storage is local on fixture node.
- Runtime switchable active profile.
- Import/export endpoints with schema + runtime validation.
- Safe fallback profile if active profile invalid.
- Profiles support sparse mappings and optional unsupported controls.
- Profiles can include default values for arbitrary channels (useful for large/sparse personalities).
- Profiles support both 8-bit and 16-bit pan/tilt mappings while controller state remains 16-bit abstract pan/tilt.

## 6) Scalable Repo Structure

- `nextgen/apps/ground-controller`
- `nextgen/apps/fixture-node`
- `nextgen/shared/protocol`
- `nextgen/shared/profiles`
- `docs/system-architecture-v2.md`

## 7) Risks and Tradeoffs

- UDP control is low-latency but lossy -> mitigate with steady frame rate + last-good hold.
- Claim complexity rises with multiple controllers -> keep lease model simple first.
- Dynamic profile import risks invalid mappings -> strict validation + safe fallback.
- Web/config operations can disturb runtime if blocking -> keep APIs asynchronous/lightweight.
