# Known Risks

## 1) DMX Timing Compliance

- Risk: fallback BREAK/MAB generation may not be deterministic enough across boards.
- Impact: some fixtures may flicker, ignore frames, or behave inconsistently.
- Mitigation:
  - move to explicit UART break control on final hardware
  - verify with DMX analyzer and multiple fixture types

## 2) Encoder Signal Quality

- Risk: noisy/bouncy encoder signals produce unstable movement.
- Impact: jittery pan/tilt response.
- Mitigation:
  - PCNT glitch filtering
  - clean wiring and pull configuration
  - motion deadband/smoothing tuning

## 3) Network Variability

- Risk: packet loss/jitter on busy networks.
- Impact: uneven fixture response or stale output.
- Mitigation:
  - fixed transmit cadence
  - accepted/bad packet metrics
  - status visibility in web UI

## 4) Config Drift and Schema Changes

- Risk: persisted config becomes incompatible after updates.
- Impact: boot-time config failure or unintended defaults.
- Mitigation:
  - `schemaVer` per store
  - validation gate before apply
  - migration hooks in config stores

## 5) Runtime Reconfiguration Hazards

- Risk: applying network/universe changes mid-show causes output disruption.
- Impact: temporary loss of control.
- Mitigation:
  - separate save vs apply endpoints
  - controlled apply sequence with operator feedback
  - optional restart-required policy for critical changes
