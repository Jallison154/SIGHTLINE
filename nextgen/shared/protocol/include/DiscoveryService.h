#pragma once

#include <Arduino.h>

#include "DiscoveryCodec.h"
#include "DiscoveryRegistry.h"

namespace sightline_v2 {

class DiscoveryService {
 public:
  bool begin(uint16_t discoveryPort);
  void setSelfAnnouncement(const DiscoveryMessageV1& self);
  void tick(uint32_t nowMs);
  const DiscoveryRegistry& registry() const { return _registry; }

 private:
  void sendAnnouncementIfDue(uint32_t nowMs);
  void receiveAnnouncements(uint32_t nowMs);

  DiscoveryCodec _codec;
  DiscoveryRegistry _registry;
  DiscoveryMessageV1 _self;
  uint16_t _port = 5568;
  uint32_t _lastTxMs = 0;
  uint32_t _txPeriodMs = 1000;
};

}  // namespace sightline_v2
