#include "DiscoveryRegistry.h"

namespace sightline_v2 {

int DiscoveryRegistry::findIndexById(const String& deviceId) const {
  for (size_t i = 0; i < _count; ++i) {
    if (_devices[i].deviceId == deviceId) return static_cast<int>(i);
  }
  return -1;
}

void DiscoveryRegistry::upsertFromAnnouncement(const DiscoveryMessageV1& m, uint32_t nowMs) {
  const int idx = findIndexById(m.deviceId);
  if (idx >= 0) {
    DiscoveredDeviceState& d = _devices[idx];
    d.deviceType = m.deviceType;
    d.friendlyName = m.friendlyName;
    d.ip = m.ip;
    d.firmwareVersion = m.firmwareVersion;
    d.health = m.health;
    d.fixtureProfileName = m.fixtureProfileName;
    d.claimState = m.claimState;
    d.assignedControllerId = m.assignedControllerId;
    d.targetNodeId = m.targetNodeId;
    d.lastAnnounceAtMs = m.sentAtMs;
    d.lastSeenMs = nowMs;
    return;
  }

  if (_count >= kMaxDevices) return;  // bounded memory behavior
  DiscoveredDeviceState& d = _devices[_count++];
  d.deviceId = m.deviceId;
  d.deviceType = m.deviceType;
  d.friendlyName = m.friendlyName;
  d.ip = m.ip;
  d.firmwareVersion = m.firmwareVersion;
  d.health = m.health;
  d.fixtureProfileName = m.fixtureProfileName;
  d.claimState = m.claimState;
  d.assignedControllerId = m.assignedControllerId;
  d.targetNodeId = m.targetNodeId;
  d.lastAnnounceAtMs = m.sentAtMs;
  d.lastSeenMs = nowMs;
}

void DiscoveryRegistry::expireStale(uint32_t nowMs, uint32_t staleTimeoutMs, uint32_t expireTimeoutMs) {
  (void)staleTimeoutMs;  // TODO: expose stale/online state flags in registry.
  size_t w = 0;
  for (size_t r = 0; r < _count; ++r) {
    const bool expired = (nowMs - _devices[r].lastSeenMs) > expireTimeoutMs;
    if (!expired) {
      if (w != r) _devices[w] = _devices[r];
      ++w;
    }
  }
  _count = w;
}

bool DiscoveryRegistry::get(size_t idx, DiscoveredDeviceState& out) const {
  if (idx >= _count) return false;
  out = _devices[idx];
  return true;
}

}  // namespace sightline_v2
