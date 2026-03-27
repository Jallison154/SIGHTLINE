# SIGHTLINE Local Simulation Architecture

## Overview

The local simulation environment runs in one Python process and models:

- one virtual controller backend
- multiple virtual fixture nodes
- shared profile loading/validation
- discovery + selection + claim state
- control routing and DMX mapping

The runtime model is separated from HTTP transport for easier reuse in firmware-aligned modules.

## Components

- `shared/sim_runtime.py`
  - `SimRuntime`: system coordinator
  - `ControllerState`: single source of truth for controller state
  - `FixtureNodeSim`: node identity/config/claim/profile/DMX state
- `dev_server.py`
  - `ThreadingHTTPServer` host
  - serves static UI files from `/data`
  - exposes REST endpoints consumed by controller and node UIs

## Flow

1. `SimRuntime.start()` bootstraps node registry from profiles.
2. Background loop simulates physical input drift (`pan`/`tilt`) in `live_mixed`.
3. Routing loop applies controller control state to active target node.
4. Node maps abstract controls -> 512-channel DMX using profile mapper.
5. Browser UIs poll backend for live state and send adjustments via REST.

## Pan/Tilt numeric assumptions

- Internal canonical values are `pan16` / `tilt16` in `0..65535`.
- This range has `65536` total discrete steps (`2^16`).
- DMX output remains byte-based:
  - coarse (MSB) byte `0..255`
  - fine (LSB) byte `0..255`
- Most other abstract controls remain uint8 (`0..255`).

## DMX buffer and high channel-count fixtures

- The node always maintains a full 512-channel DMX buffer (`bytearray(512)`).
- Every channel value is an 8-bit byte (`0..255`).
- Fixture profiles can map only a sparse subset of channels and still declare large `channel_count` values (e.g. 56 channels).
- Optional `defaults.channel_values` allows pre-seeding many channels before dynamic control mappings are applied.

## API Groups

Controller-facing:

- `GET /api/state`
- `POST /api/state`
- `GET /api/nodes`
- `POST /api/select-node`
- `GET /api/target-fixture`
- `GET /api/status`

Node-facing (query arg `nodeId`):

- `GET /api/config?nodeId=...`
- `POST /api/config?nodeId=...`
- `GET /api/status?nodeId=...`
- `GET /api/profiles?nodeId=...`
- `POST /api/profile?nodeId=...`
- `POST /api/profile/import?nodeId=...`
- `GET /api/profile/export?nodeId=...&profileId=...`

## Extension points

- TODO(HW): replace simulated physical input loop with hardware bridge.
- TODO(PROTOCOL): align claim/discovery/status envelopes with firmware codecs.
- TODO(DMX): add optional Art-Net output bus in server mode for desk visualizers.
- TODO(GROUPS): add multi-target/group routing map in controller state.
