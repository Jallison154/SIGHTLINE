# Fixture Profile Format

Fixture profiles are JSON files used by both firmware targets and validated by:

- `shared/profiles/fixture-profile.schema.json` (schema-level checks)
- `controller-firmware/src/FixtureProfileParser.cpp` (runtime validation checks)

## Required fields

- `id`
- `fixture_name`
- `manufacturer`
- `mode_name`
- `channel_count`
- `channels.pan.coarse`
- `channels.tilt.coarse`

## Optional channels

- `channels.dimmer`
- `channels.shutter`
- `channels.zoom`
- `channels.focus`
- `channels.iris`
- `channels.color`

If an optional channel is missing, firmware stores it as channel `0` and mapping logic skips it cleanly.

## Optional high-density / sparse-mode support

- `logical_mappings` (optional): extra named mappings for fixture-specific channels.
- `unsupported_controls` (optional): controls intentionally unavailable in this mode.
- `defaults.channel_values` (optional): static per-channel defaults (channel index -> `0..255`) for sparse/high-channel personalities.

## Optional behavior blocks

- `invert` flags on channels
- `limits` objects (`min`, `max`)
- `defaults` values (`dimmer`, `shutter_open`, motion parameters)

## Runtime handling rules

- Channels are 1-based DMX addresses.
- Optional channels use `0` internally to mean "not mapped".
- Required pan/tilt channels must be valid and within `channel_count`.
- Limits are only applied when both `min` and `max` are present.
- Pan/tilt are treated as canonical 16-bit runtime values (`0..65535`).
- If the profile provides both `coarse` and `fine`, SIGHTLINE outputs full 16-bit pan/tilt:
  - coarse byte (`MSB`) in `0..255`
  - fine byte (`LSB`) in `0..255`
- If the profile provides only `coarse` (no `fine`), SIGHTLINE outputs 8-bit pan/tilt by downscaling from 16-bit using the high byte.
- `0..65535` is the valid 16-bit range; it represents `65536` discrete steps.
- Fixtures with many channels (including `50+`) are supported because mapping only touches defined channels while the node maintains the full 512-channel DMX buffer.
