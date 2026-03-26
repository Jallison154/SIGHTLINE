# Fixture Node Data Flow

## Runtime Loop

Fixture node `loop()` delegates to `App::tick(nowMs)`:

1. Poll Art-Net UDP socket (`ArtNetReceiver`)
2. Validate/parse ArtDMX and filter by configured universe
3. Update shared 512-channel DMX buffer (`DmxBuffer`)
4. Emit DMX512 frame at fixed cadence (`DmxOutput`)
5. Serve web UI/API requests (`WebUiServer`)
6. Publish runtime metrics (`StatusTracker`)

## Signal Handling

- Art-Net input is non-blocking (`parsePacket()` polling)
- Only accepted ArtDMX frames update output buffer
- Rejected frames are counted (ignored universe / malformed)
- Status includes:
  - signal present/absent
  - last frame interval
  - packet counters

## DMX Output Path

- Current output source is always the latest buffer snapshot
- Full 512 slots are transmitted continuously
- UART target mode:
  - `250000` baud
  - `8N2`
  - proper BREAK + MAB behavior required for final hardware

## Web UI Path

- `GET /api/status` exposes runtime health
- `GET /api/config` returns persisted/runtime config
- `POST /api/config` saves validated config
- `POST /api/config/apply` is the apply hook for runtime reconfiguration
