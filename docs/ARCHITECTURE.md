# SIGHTLINE Architecture

## 1) Firmware Architecture (High-Level)

Both devices use the same non-blocking pattern:

- `setup()` performs one-time initialization.
- `loop()` runs a fast cooperative scheduler.
- Each module exposes `begin()` and `tick(nowMs)` methods.
- No long `delay()` calls in production paths.

## 2) Ground Controller Flow

1. Read physical controls (encoders, faders, buttons).
2. Filter/shape pan-tilt input (deadband, smoothing, speed scaling).
3. Resolve fixture profile channel mapping.
4. Encode 16-bit pan/tilt into coarse/fine DMX bytes.
5. Fill a full 512-byte DMX universe buffer.
6. Send Art-Net DMX packets at a fixed output rate.

## 3) Fixture Node Flow

1. Bring up Ethernet and network stack.
2. Receive Art-Net packets and validate universe/opcode.
3. Update a full 512-byte DMX output buffer.
4. Stream DMX512 continuously at valid timing.
5. Host lightweight web UI for configuration changes.
6. Persist settings to NVS and apply updates safely.

## 4) Shared Fixture Profile JSON Format

Each profile should include:

- Identity (`id`, `display_name`, `manufacturer`, `model`)
- Footprint (`channel_count`)
- Attribute map (`pan`, `tilt`, `dimmer`, `zoom`, `iris`, `focus`, `color`)
- 16-bit definitions (`coarse`, `fine` channels)
- Optional behavior hints (`invert`, `default_speed_scale`, limits)

The firmware parser should validate required fields and reject malformed profiles.

## 5) Key Risks and Mitigations

1. Network and DMX timing conflicts:
   - Use task separation and queue/buffer handoff.
2. Blocking web handlers:
   - Keep handlers short and async; no heavy synchronous parsing in request thread.
3. Encoder jitter / operator feel:
   - Apply deadband + smoothing + curve scaling, tune empirically.
4. Fixture variability:
   - Strict profile validation and safe defaults.
5. Live-event reliability:
   - Add watchdog strategy, heartbeat status, graceful fallback states.
