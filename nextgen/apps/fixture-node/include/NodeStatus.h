#pragma once

#include <Arduino.h>

namespace sightline_v2 {

struct NodeStatus {
  bool networkReady = false;
  bool discoveryReady = false;
  bool controlRxReady = false;
  bool webReady = false;
  bool dmxReady = false;
  bool profileActive = false;
  bool claimed = false;
  String assignedControllerId;
  String activeProfileId;
  uint32_t uptimeMs = 0;
  uint32_t controlFramesAccepted = 0;
  uint32_t dmxFramesOutput = 0;
};

}  // namespace sightline_v2
