#pragma once

#include <Arduino.h>

#include "ControlState.h"

namespace sightline_v2 {

enum class InputMode : uint8_t {
  kLiveMixed = 0,
  kWebOverride = 1,
  kMonitorOnly = 2
};

struct ControlPatch {
  bool hasPan = false;
  float panNorm = 0.5f;
  bool hasTilt = false;
  float tiltNorm = 0.5f;
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
};

struct RuntimeStatusFields {
  uint32_t lastUpdateMs = 0;
  String lastUpdateSource = "init";
  bool blackout = false;
  bool homeRequested = false;
  uint8_t sensitivity = 100;
  bool claimRequested = false;
  bool releaseRequested = false;
};

struct PerControlSourceInfo {
  String pan = "init";
  String tilt = "init";
  String intensity = "init";
  String iris = "init";
  String zoom = "init";
  String focus = "init";
  String shutter = "init";
  String color = "init";
};

class ControllerRuntimeState {
 public:
  void setInputMode(InputMode mode, uint32_t nowMs);
  InputMode inputMode() const { return _mode; }

  // Hardware writes in live-mixed mode only.
  void applyHardwareState(const ControlState& hardwareState, uint32_t nowMs);

  // Web writes in live-mixed and web-override modes.
  bool applyWebPatch(const ControlPatch& patch, uint32_t nowMs);

  void setActiveTargetNodeId(const String& nodeId, const String& source, uint32_t nowMs);
  const String& activeTargetNodeId() const { return _activeTargetNodeId; }

  void setBlackout(bool enabled, uint32_t nowMs);
  void requestHome(uint32_t nowMs);
  void setSensitivity(uint8_t value, uint32_t nowMs);
  void setClaimRequested(bool requested, uint32_t nowMs);
  void setReleaseRequested(bool requested, uint32_t nowMs);

  const ControlState& controls() const { return _controls; }
  const RuntimeStatusFields& status() const { return _status; }
  const PerControlSourceInfo& perControlSource() const { return _perControlSource; }

 private:
  static float clampNorm(float v);

  InputMode _mode = InputMode::kLiveMixed;
  ControlState _controls;
  RuntimeStatusFields _status;
  PerControlSourceInfo _perControlSource;
  String _activeTargetNodeId;
};

}  // namespace sightline_v2
