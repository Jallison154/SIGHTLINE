#pragma once

#include <Arduino.h>

namespace sightline_v2 {

struct AbstractControls {
  bool hasPan = false;
  float pan = 0.5f;
  bool hasTilt = false;
  float tilt = 0.5f;
  bool hasIntensity = false;
  uint8_t intensity = 0;
  bool hasIris = false;
  uint8_t iris = 0;
  bool hasZoom = false;
  uint8_t zoom = 0;
  bool hasFocus = false;
  uint8_t focus = 0;
  bool hasShutter = false;
  uint8_t shutter = 0;
  bool hasColor = false;
  uint8_t color = 0;
  String extraJson;  // Optional extension controls blob.
};

struct ControlFrameV1 {
  uint16_t schemaVersion = 1;
  String msgType = "control_frame";
  String sessionId;
  String controllerId;
  String targetNodeId;
  uint32_t frameSeq = 0;
  uint32_t sentAtMs = 0;
  AbstractControls controls;
};

struct ControlStreamStatus {
  bool receiving = false;
  uint32_t framesSeen = 0;
  uint32_t framesAccepted = 0;
  uint32_t framesRejected = 0;
  uint32_t lastAcceptedAtMs = 0;
  uint32_t lastFrameIntervalMs = 0;
};

}  // namespace sightline_v2
