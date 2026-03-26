# Fixture Profile Format (Draft)

Fixture profiles are JSON files used by both firmware targets.

## Required top-level fields

- `id` (string)
- `display_name` (string)
- `channel_count` (number)
- `attributes` (object)

## Attribute mapping

- 8-bit attribute: `{ "coarse": <channel> }`
- 16-bit attribute: `{ "coarse": <channel>, "fine": <channel>, "invert": <bool> }`

## Notes

- Channels are 1-based.
- Use stable IDs to avoid breaking saved configuration.
- Add optional defaults to tune pan/tilt handling.
