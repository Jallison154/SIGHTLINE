# Controller Firmware

ESP32-based ground controller firmware for SIGHTLINE.

## Responsibilities

- Read panel controls (encoders, faders, buttons)
- Process pan/tilt movement with deadband, smoothing, and speed scaling
- Build full 512-channel DMX data
- Transmit Art-Net over Ethernet

## Structure

- `platformio.ini`: PlatformIO project config
- `include/`: module headers
- `src/`: application entrypoint and modules
- `lib/`: private reusable components for this target
- `config/`: local config placeholders and defaults
