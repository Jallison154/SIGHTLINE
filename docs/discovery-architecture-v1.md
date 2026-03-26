# SIGHTLINE Discovery Architecture v1

## 1) Discovery Method Comparison

### UDP Broadcast Heartbeats

- Pros:
  - Very simple on ESP32
  - No central server dependency
  - Low latency on local event LANs
  - Easy to evolve schema/version
- Cons:
  - Broadcast traffic must be rate-limited
  - No guaranteed delivery

### mDNS

- Pros:
  - Standardized service discovery
  - Human-friendly hostnames
- Cons:
  - Can be inconsistent on constrained/segmented show networks
  - More implementation complexity on embedded targets
  - Less flexible for rich runtime state payloads

### REST Registration (central registry)

- Pros:
  - Strong consistency and query model
  - Easy global coordination when server exists
- Cons:
  - Requires always-on registration service
  - Single point of failure
  - Added ops overhead for DIY local deployments

## 2) Recommended v1 Approach

Use **UDP broadcast heartbeats** on a fixed discovery port for v1.

Why:

- Best fit for ESP32 simplicity and event LAN reliability
- Zero central infrastructure
- Works for controller and fixture node peer visibility
- Future-compatible with multi-controller coordination

Add optional mDNS later only as convenience naming layer, not core control dependency.

## 3) v1 Discovery Message Schema

Common fields:

- `schema_version` (uint16)
- `msg_type` (`announce`)
- `device_id` (stable unique ID)
- `device_type` (`controller` | `fixture_node`)
- `friendly_name`
- `ip`
- `fw_version`
- `status` (`ok` | `degraded` | `error`)
- `uptime_ms`
- `sent_at_ms`

Fixture-node extensions:

- `fixture_profile_name`
- `claim_state` (`available` | `claimed`)
- `assigned_controller_id` (empty if available)

Controller extensions:

- `target_node_id` (optional active target)

## 4) Discovered Device State Model

For each discovered device:

- identity: ID, type, friendly name
- network: IP
- software: firmware version
- runtime: status, last seen, heartbeat age
- role metadata:
  - fixture node -> profile + claim info
  - controller -> active target

State transitions:

- `new` -> first announcement received
- `online` -> heartbeat age <= timeout
- `stale` -> heartbeat age > timeout
- `expired` -> removed after stale retention window

Recommended timing:

- heartbeat every 1000 ms
- stale timeout 3000 ms
- expire/remove timeout 10000 ms

## 5) ESP32 Implementation Notes

- Single UDP socket bound to discovery port
- Non-blocking poll in main loop
- Bounded registry size to control memory
- Ignore malformed/oversized packets safely

## 6) Board/Network TODO Areas

- Ethernet initialization and link-state gating before announce
- Broadcast IP policy per local network (`255.255.255.255` vs subnet broadcast)
- Socket buffer sizing and packet rate tuning on final hardware
