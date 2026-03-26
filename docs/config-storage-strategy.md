# Config Storage Strategy

SIGHTLINE firmware uses a consistent configuration pattern on both devices:

1. Read persisted config from NVS.
2. Validate values before use.
3. Fall back to safe defaults if invalid or missing.
4. Copy persisted config into runtime config.
5. Runtime config is used by live modules.

## Why persisted vs runtime split

- Persisted config reflects durable user intent.
- Runtime config can be applied safely, staged, or partially updated.
- This avoids corrupting stored state during temporary runtime operations.

## Versioning and migrations

- Each store writes a `schemaVer` key.
- `kCurrentSchemaVersion` defines expected layout.
- Unknown/newer schema versions are rejected and defaulted safely.
- Migration hooks (`migrateIfNeeded`) are included for future schema evolution.

## Current stores

- Fixture node: `fixture-node-firmware/src/ConfigStore.cpp`
- Controller: `controller-firmware/src/ControllerConfigStore.cpp`
