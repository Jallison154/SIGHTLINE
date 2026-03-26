#pragma once

#include <Arduino.h>

namespace sightline_v2 {

enum class DeviceType : uint8_t {
  kGroundController = 1,
  kFixtureNode = 2
};

struct DeviceIdentity {
  String deviceId;      // Unique stable ID
  String friendlyName;  // Operator-visible label
  DeviceType type;
};

struct AbstractControlFrame {
  String sessionId;
  String controllerId;
  String targetNodeId;
  uint32_t frameSeq = 0;
  uint32_t sentAtMs = 0;

  float panNorm = 0.5f;
  float tiltNorm = 0.5f;
  uint8_t intensity = 0;
  uint8_t iris = 0;
  uint8_t zoom = 0;
  uint8_t focus = 0;
  uint8_t shutter = 0;
  uint8_t color = 0;
};

}  // namespace sightline_v2
