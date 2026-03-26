# SIGHTLINE NextGen (Scalable Architecture Scaffold)

This folder contains a revised multi-device network architecture scaffold for SIGHTLINE.

## Goals

- Multiple fixture nodes on one network
- Multiple controllers later (coordination-ready)
- Abstract control protocol (not Art-Net between controller and node)
- Node discovery + claim/release + status
- Profile-driven DMX rendering at fixture node

## Layout

- `platformio.ini` - separate build environments
- `apps/ground-controller` - control source + targeting client
- `apps/fixture-node` - control sink + DMX renderer + web/profile API
- `shared/protocol` - protocol envelopes/types
- `shared/profiles` - JSON schemas and examples

## Notes

- Existing firmware remains unchanged in root folders.
- This scaffold is intentionally modular with TODO markers for hardware/network specifics.
