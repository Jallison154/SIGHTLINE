#pragma once

#include <Arduino.h>

#include "DiscoveryProtocol.h"

namespace sightline_v2 {

class DiscoveryRegistry {
 public:
  static constexpr size_t kMaxDevices = 32;

  void upsertFromAnnouncement(const DiscoveryMessageV1& message, uint32_t nowMs);
  void expireStale(uint32_t nowMs, uint32_t staleTimeoutMs, uint32_t expireTimeoutMs);
  size_t size() const { return _count; }
  bool get(size_t idx, DiscoveredDeviceState& out) const;

 private:
  int findIndexById(const String& deviceId) const;
  DiscoveredDeviceState _devices[kMaxDevices];
  size_t _count = 0;
};

}  // namespace sightline_v2
