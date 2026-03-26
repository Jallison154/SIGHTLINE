# Fixture Node Firmware

ESP32-based fixture node firmware for SIGHTLINE.

## Responsibilities

- Receive Art-Net over Ethernet
- Output DMX512 to moving head fixture
- Serve lightweight web UI for configuration
- Persist settings in non-volatile memory

## Structure

- `platformio.ini`: PlatformIO project config
- `include/`: module headers
- `src/`: application entrypoint and modules
- `lib/`: private reusable components for this target
- `data/`: static web assets
- `config/`: local config placeholders and defaults
