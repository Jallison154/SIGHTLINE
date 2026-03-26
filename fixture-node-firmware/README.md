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

## DMX512 Output Notes

`DmxOutput` uses ESP32 UART in `250000` baud, `SERIAL_8N2`, with a full universe frame:

- start code (`0x00`)
- 512 channel bytes
- continuous refresh (default `~40Hz`, `25ms`)

Board-specific tuning remains marked as `TODO(HW)`:

- final UART index and TX pin
- RS-485 DE/RE direction control pin behavior
- deterministic BREAK/MAB generation using ESP-IDF UART primitives
