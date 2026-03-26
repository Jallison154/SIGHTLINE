#pragma once

#include <Arduino.h>

namespace sightline_v2 {

struct ControllerStatus {
  bool networkReady = false;
  bool discoveryReady = false;
  bool ownershipReady = false;
  bool controlTxReady = false;
  String targetNodeId;
  bool ownsTarget = false;
  uint32_t controlFramesSent = 0;
  uint32_t uptimeMs = 0;
};

}  // namespace sightline_v2
