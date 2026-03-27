# Ground Controller (NextGen)

Scaffolded modular controller architecture for SIGHTLINE v2.

## Modules

- `ControllerNetworkService` - Ethernet/network state
- `DiscoveryService` - advertise controller and discover nodes/controllers
- `TargetSelector` - active target node selection logic
- `OwnershipClient` - claim/release/heartbeat client state
- `ControlTxService` - abstract control frame transmit
- `EncoderInputService` - pan/tilt encoder hardware input
- `PanTiltMotionEngine` - velocity-based pan/tilt motion logic
- `FaderInputService` - analog fader/knob input layer
- `ButtonInputService` - button input layer
- `ControllerConfigStore` - persisted config load/save/validate
- `ControllerStatus` - runtime status for future UI/API
- `ControllerRuntimeState` - shared source-of-truth control + mode state
- `ControllerWebService` - controller-side test/service web UI + APIs

## Control Loop Model

`tick(nowMs)` is a cooperative non-blocking pipeline:

1. network/discovery ticks
2. read input services (encoders/faders/buttons)
3. motion engine update from encoder deltas
4. target selection update
5. ownership state update
6. abstract control frame transmit at fixed interval

This keeps hardware input separated from motion and protocol logic.

All hardware-specific assumptions are marked with `TODO(HW)`.

## Controller Test Web UI

Ground controller now serves a lightweight UI from `apps/ground-controller/data/` for:

- live test controls (pan/tilt/intensity/iris/zoom)
- target selection + claim/release scaffolding
- status and service actions (blackout, home placeholder)

Upload filesystem assets:

```bash
pio run -e ground_controller_dev -t uploadfs
```
