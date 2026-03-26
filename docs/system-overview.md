# System Overview

SIGHTLINE is a two-device follow-spot control system for moving head fixtures:

- Controller firmware (`controller-firmware`)
  - Reads operator inputs (pan/tilt encoders first, more controls later)
  - Maintains virtual pan/tilt position in software
  - Maps control values to fixture channels using selected profile
  - Transmits full-universe Art-Net over Ethernet
- Fixture node firmware (`fixture-node-firmware`)
  - Receives Art-Net for selected universe
  - Maintains latest 512-channel DMX buffer
  - Outputs DMX512 through UART + RS-485 transceiver
  - Hosts lightweight web UI for configuration/status

## Design Principles

- Non-blocking loops (`begin()` + `tick(nowMs)` modules)
- Full 512-channel universe buffers end-to-end
- Profile-driven channel mapping (fixture-agnostic control layer)
- Safe defaults + validation for configuration
- Embedded-friendly implementation (small memory footprint, no heavy frameworks)

## Repository Structure

- `controller-firmware/` - ESP32 controller
- `fixture-node-firmware/` - ESP32 Art-Net to DMX node
- `shared/profiles/` - JSON fixture profiles and schema
- `docs/` - architecture, flows, risks, roadmap
