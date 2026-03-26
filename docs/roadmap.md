# Future Roadmap

## Near Term

- Finalize board-specific Ethernet and DMX pin mappings
- Replace DMX BREAK/MAB fallback with deterministic UART implementation
- Add controller profile selection from persisted config
- Add fixture node live apply logic for network/universe changes
- Add serial diagnostics pages for field setup

## Mid Term

- Multi-universe controller transmit support
- Fixture node input fail-safe behavior (hold/blackout policy)
- Profile package tooling (validation CLI + import/export)
- Web UI auth option for unmanaged networks
- Better runtime health metrics and event logs

## Longer Term

- Preset/cue support for controller
- RDM-aware fixture node experiments (if hardware allows)
- Auto-discovery and node inventory view
- Optional OTA update pipeline for both device classes

## Definition of Done Bias

For live-event safety, prioritize:

- deterministic timing
- graceful behavior on bad/missing data
- clear operator feedback
- low surprise during reboots/network changes
