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
- `channels.pan.coarse`, `channels.pan.fine`
- `channels.tilt.coarse`, `channels.tilt.fine`

## Optional channels

- `channels.dimmer`
- `channels.shutter`
- `channels.zoom`
- `channels.focus`
- `channels.iris`
- `channels.color`

If an optional channel is missing, firmware stores it as channel `0` and mapping logic skips it cleanly.

## Optional behavior blocks

- `invert` flags on channels
- `limits` objects (`min`, `max`)
- `defaults` values (`dimmer`, `shutter_open`, motion parameters)

## Runtime handling rules

- Channels are 1-based DMX addresses.
- Optional channels use `0` internally to mean "not mapped".
- Required pan/tilt channels must be valid and within `channel_count`.
- Limits are only applied when both `min` and `max` are present.
