# SIGHTLINE local development test environment

Develop and test SIGHTLINE behavior locally before ESP32 hardware is complete.

This environment simulates:

- controller runtime state and routing
- multiple fixture nodes
- profile loading/selection/mapping
- discovery and claim/assignment behavior
- live UI synchronization for controller and node pages

## Stack choice

This environment uses **Python stdlib** (`ThreadingHTTPServer` + shared simulation modules).

Why:

- no heavy dependencies
- easy to debug with plain logs and JSON
- fast iteration for protocol/state experiments
- reusable pure-logic modules that can inform firmware implementations

## Folder structure

```
test-environment/
  controller-sim/      # legacy CLI sender tools (still available)
  fixture-node-sim/    # legacy CLI node tools (still available)
  shared/              # reusable protocol, profile, DMX, runtime simulation modules
  profiles/            # fixture profiles used by simulation
  logs/                # runtime logs
  docs/                # architecture notes
  dev_server.py        # full local simulation backend + static UI host
  run_local_sim.sh     # convenience launcher
```

## Architecture (new full local sim)

See `docs/local-sim-architecture.md` for detailed internals.

High-level flow:

1. `dev_server.py` starts `SimRuntime`.
2. Runtime bootstraps two fixture nodes and loads profiles.
3. Background loop simulates physical input drift (pan/tilt) in `live_mixed`.
4. Routing loop applies controller abstract controls to active node.
5. Node maps controls to DMX via `shared/dmx_mapper.py`.
6. Controller and node UIs poll APIs and render live state.

## Run locally

From `test-environment/`:

```bash
./run_local_sim.sh
```

Or:

```bash
python3 dev_server.py --host 127.0.0.1 --port 8088
```

Open:

- Controller UI: `http://127.0.0.1:8088/controller`
- Fixture node UI: `http://127.0.0.1:8088/node?nodeId=fixture-1`
- Second node UI: `http://127.0.0.1:8088/node?nodeId=fixture-2`

## Implemented capabilities

### 1) Controller simulation

- shared control state:
  - canonical pan/tilt: `pan16`, `tilt16` (`0..65535`)
  - UI aliases: `pan`, `tilt` (`0.0..1.0`) for slider ergonomics
  - most other controls: `0..255` (`intensity`, `iris`, `zoom`, `focus`, `shutter`, `color`)
- target selection and switching
- claim/release simulation
- input mode support (`monitor_only`, `live_mixed`, `web_override`)
- simulated physical input updates for pan/tilt
- controller status/discovery views

### 2) Fixture node simulation

Each node has:

- `nodeId`, friendly name, fixture label
- active profile
- online status
- claim status + assigned controller
- 512-channel DMX buffer
- status/config/profile APIs

### 3) Profile system

- loads JSON profiles from `profiles/`
- validates through `shared/fixture_profile.py`
- supports profile apply/save/import/export via API
- maps abstract controls → DMX channels using active profile
- pan/tilt mapping uses 16-bit values split to coarse/fine bytes (`0..255` each)
- supports sparse/high-channel fixtures with optional:
  - `logical_mappings`
  - `unsupported_controls`
  - `defaults.channel_values`

### 16-bit pan/tilt clarification

- valid 16-bit value range: `0..65535`
- total number of discrete steps: `65536`
- each DMX channel output remains `0..255` (always byte-sized)

### 50+ channel fixture handling

- Node keeps a full 512-byte DMX buffer.
- Profile `channel_count` may be large (e.g. 56).
- Sparse mappings are allowed; only mapped channels are actively driven.
- Optional `defaults.channel_values` preloads static channels for large personalities.

### 4) Multi-node testing

- default simulation starts with two nodes (`fixture-1`, `fixture-2`)
- controller selects one active target at a time
- architecture leaves room for future group targeting

### 5) Discovery simulation

- simulated node registry is exposed via `/api/nodes`
- controller can discover/select/claim nodes using same surface as web UI

### 6) Live UI behavior

- controller page works against local `/api/state`, `/api/nodes`, `/api/select-node`, `/api/target-fixture`, `/api/status`
- node page works against `/api/config`, `/api/status`, `/api/profiles`, `/api/profile`, `/api/profile/import`, `/api/profile/export` with `nodeId` query
- both pages stay synchronized through lightweight polling

## API summary

Controller-facing:

- `GET /api/state`
- `POST /api/state`
- `GET /api/nodes`
- `POST /api/select-node`
- `GET /api/target-fixture`
- `GET /api/status`

Node-facing (`?nodeId=fixture-1` etc):

- `GET /api/config`
- `POST /api/config`
- `GET /api/status`
- `GET /api/profiles`
- `POST /api/profile`
- `POST /api/profile/import`
- `GET /api/profile/export`

## Logging and visibility

- runtime logs under `logs/` (via `shared/logging_utils.py`)
- logs include discovery bootstrap, target switching, profile actions, config updates
- UIs expose live status + raw state blocks for debugging

## Add a new fixture profile

1. Add a JSON file in `test-environment/profiles/`.
2. Ensure required fields match existing profile schema style (`id`, `channels.pan`, `channels.tilt`, etc).
3. Restart `dev_server.py` (or add dynamic refresh later).
4. New profile appears in node profile list and can be applied.

Example high-channel sparse profile included:

- `profiles/example_56ch_sparse.json`

## Add another simulated fixture node

Edit `bootstrap_default_nodes()` in `shared/sim_runtime.py` and add another node tuple:

- unique `node_id`
- friendly name
- fixture label
- simulated IP/firmware values

Restart server; node appears in `/api/nodes` and can be targeted.

## Firmware alignment and TODOs

- TODO(HW): bridge simulated physical input loop to real input service.
- TODO(PROTOCOL): align claim/discovery/status envelopes to firmware codecs.
- TODO(PROFILE): stronger schema validation + profile save semantics.
- TODO(GROUPS): multi-target/group routing model.
- TODO(DMX): optional Art-Net mirror from full server mode.

## Legacy CLI simulators (still available)

The original CLI tools are kept for focused testing:

- `controller-sim/sim_controller.py`
- `fixture-node-sim/sim_node.py`

They remain useful for direct protocol/mapping tests and scripted traffic generation.
