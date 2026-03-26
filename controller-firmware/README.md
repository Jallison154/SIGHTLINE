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
- `sim/`: small host-side simulation helpers for tuning logic

## Pan/Tilt Tuning Notes

Pan/tilt motion uses velocity-based control from encoder deltas (not absolute position):

- `sensitivity`: counts/sec -> velocity conversion
- `accelerationThresholdCountsPerSec`: where acceleration boost begins
- `accelerationGain` and `accelerationCurve`: how strongly fast turns speed up motion
- `velocitySmoothing`: low-pass filter for natural movement
- `maxVelocityNormalizedPerSec`: safety clamp

Hardware-specific encoder pin setup remains marked as `TODO(HW)` in `src/ControlInput.cpp`.

## Art-Net Transmit Notes

`src/ArtNetSender.cpp` provides clean Art-Net transmit separation with API:

- `setChannel(channel, value)`
- `setBuffer(data, length)`
- `setUniverse(universe)`
- `sendFrame()`

Recommended output rates for moving lights:

- `30-44 Hz` for smooth follow-spot style control
- `40 Hz` (25ms period) is a practical default
- `>50 Hz` is usually unnecessary network load for most fixtures

Board-specific Ethernet bring-up is marked as `TODO(HW)` in `src/ArtNetSender.cpp`.
