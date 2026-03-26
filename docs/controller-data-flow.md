# Controller Data Flow

## Runtime Loop

Controller `loop()` delegates to `App::tick(nowMs)`:

1. Read input deltas from pan/tilt encoders (`ControlInput`)
2. Convert deltas to velocity/position (`PanTiltEngine`)
3. Map control state to DMX buffer (`DmxMapper`)
4. Push full universe to Art-Net transmit module (`ArtNetSender`)
5. Send frame on fixed interval (`txPeriodMs`)

## Data Path

- Encoder movement -> delta counts per tick
- Delta counts -> smoothed velocity with acceleration
- Velocity -> virtual normalized position (`0.0..1.0`)
- Position -> 16-bit pan/tilt values (`0..65535`)
- 16-bit + other controls -> profile-based DMX channel mapping
- DMX universe -> ArtDMX UDP packet

## Timing Targets

- Controller tick loop: as fast as possible, non-blocking
- Art-Net frame cadence: typically `25ms` (`~40Hz`)
- Clamp large `dtMs` spikes to avoid unstable jumps after stalls

## Practical Tuning Points

- Motion tuning (`PanTiltTuning`)
  - sensitivity
  - acceleration threshold/gain/curve
  - smoothing factor
  - max velocity clamp
- Profile defaults
  - pan/tilt speed scale
  - deadband
- Output timing
  - transmit period per universe
