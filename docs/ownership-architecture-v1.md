# SIGHTLINE Ownership Architecture v1

## 1) Ownership State Model

Fixture node is authoritative for ownership and uses lease-based state:

- `available`
  - no active owner
  - accepts claim request
- `claimed(controller_id, lease_expiry_ms)`
  - only owner controller can send control frames
  - owner must refresh lease with heartbeat

State transitions:

- `available -> claimed` on accepted claim
- `claimed -> available` on owner release
- `claimed -> available` on lease timeout
- `claimed(owner A)` rejects claim from controller B (v1 single-owner protection)

## 2) Claim/Release/Heartbeat/Status Model

Transport (v1 recommendation):

- UDP request/response for low-latency embedded simplicity
- JSON envelope with schema version and request IDs

Messages:

- `OwnershipClaimRequest`
  - `request_id`, `controller_id`, `node_id`, `lease_ms`, `sent_at_ms`
- `OwnershipClaimResponse`
  - `request_id`, `result`, `node_id`, `controller_id`, `lease_ms`, `expires_at_ms`
- `OwnershipReleaseRequest`
  - `request_id`, `controller_id`, `node_id`, `sent_at_ms`
- `OwnershipHeartbeat`
  - `controller_id`, `node_id`, `sent_at_ms`

Status exposure:

- Discovery heartbeat includes `claim_state` + `assigned_controller_id`
- Web/API can expose ownership state and remaining lease

## 3) Timeout Behavior

Recommended v1 values:

- lease duration: `3000 ms`
- heartbeat period: `1000 ms`
- local controller lease timeout guard: `>= lease duration`

If controller disappears:

- fixture node lease expires
- node automatically transitions to `available`
- control path rejects stale-owner frames

## 4) Safe Recovery on Communication Loss

Node side:

- Keep last valid control frame for short hold window (optional)
- On prolonged loss, move to safe fallback output policy (e.g. hold or fade to safe state)
- Always clear ownership on lease expiry

Controller side:

- If claim responses/heartbeats fail, mark ownership lost
- Stop sending control-to-node commands that assume ownership
- Attempt re-claim if operator still targets node

## 5) Extensibility

v1 is single-owner by design, but fields and state are compatible with:

- priority claims
- negotiated handoff
- shared/group control
- controller arbitration service
