# Fixture Node (NextGen)

Scaffolded modular fixture-node architecture for SIGHTLINE v2.

## Modules

- `NetworkService` - Ethernet bring-up and link/IP state
- `DiscoveryService` - node auto-advertise and peer discovery
- `OwnershipManager` - claim/lease authority
- `ControlRxService` - abstract control frame receive/validation
- `NodeConfigStore` - persisted config load/save/validate
- `FixtureProfileManager` - active profile lifecycle
- `FixtureProfileStore` - profile persistence/import/export backbone
- `AbstractToDmxMapper` - abstract controls -> DMX universe
- `DmxOutputService` - DMX512 output path
- `NodeWebService` + `FixtureProfileWebApi` - UI and API surface

## Update Loop Model

`tick(nowMs)` performs short non-blocking service calls:

1. network tick
2. discovery tick
3. ownership tick
4. control receive tick
5. profile-based DMX render
6. DMX transmit tick
7. web/profile API tick

All hardware/network specific details are marked with `TODO(HW)`.
