# SIGHTLINE Fixture Profile System v2

## 1) JSON Schema Design

Profile schema lives at:

- `nextgen/shared/profiles/schema/fixture-profile.schema.json`

Profile describes abstract-control-to-DMX mapping:

- identity: `id`, `fixture_name`, `manufacturer`, `mode_name`
- footprint: `channel_count`
- required: `channels.pan.{coarse,fine}`, `channels.tilt.{coarse,fine}`
- optional: `intensity`, `iris`, `zoom`, `focus`, `shutter`, `color`
- optional per-channel behavior:
  - `invert`
  - `limits`
- optional `defaults` for abstract controls

## 2) Validation Rules

Core runtime rules:

- required identity fields must be present
- `channel_count` in `1..512`
- pan/tilt coarse+fine required and within channel range
- optional channels may be absent
- limit ranges must have `min <= max`
- invalid profiles are rejected before activation

## 3) Example Profiles

- `nextgen/shared/profiles/examples/generic-moving-head-20ch.json`
- `nextgen/shared/profiles/examples/beam-compact-14ch.json`

## 4) Profile Storage + Management Modules

Fixture-node modules:

- `FixtureProfileCodec` - JSON encode/decode
- `FixtureProfileValidator` - runtime validation
- `FixtureProfileStore` - SPIFFS persistence (multi-profile + active marker)
- `FixtureProfileManager` - runtime activation/safe switching
- `FixtureProfileWebApi` - profile CRUD/import/export/activate API surface

## 5) Runtime Switching and Failure Handling

Switch sequence:

1. Load requested profile JSON from storage
2. Decode + validate into temporary object
3. Swap active profile only if step 2 succeeds
4. Persist active-profile marker

Failure behavior:

- If new profile fails validation, keep prior active profile
- If no active profile available at boot, node remains in safe/unmapped mode
- Profile import failures do not modify active profile

Operational guidance:

- Prefer atomic profile file writes (temp + rename)
- Keep a known-good fallback profile on node
- Expose active-profile and last-error in web status for debugging
