#include "ControllerRuntimeState.h"

namespace sightline_v2 {

namespace {
uint8_t clampU8(int v) {
  if (v < 0) return 0;
  if (v > 255) return 255;
  return static_cast<uint8_t>(v);
}
}  // namespace

float ControllerRuntimeState::clampNorm(float v) {
  if (v < 0.0f) return 0.0f;
  if (v > 1.0f) return 1.0f;
  return v;
}

void ControllerRuntimeState::setInputMode(InputMode mode, uint32_t nowMs) {
  _mode = mode;
  _status.lastUpdateMs = nowMs;
  _status.lastUpdateSource = "mode";
}

void ControllerRuntimeState::applyHardwareState(const ControlState& hardwareState, uint32_t nowMs) {
  if (_mode != InputMode::kLiveMixed) {
    return;
  }
  _controls.panNorm = clampNorm(hardwareState.panNorm);
  _controls.tiltNorm = clampNorm(hardwareState.tiltNorm);
  _controls.intensity = hardwareState.intensity;
  _controls.iris = hardwareState.iris;
  _controls.zoom = hardwareState.zoom;
  _controls.focus = hardwareState.focus;
  _controls.shutter = hardwareState.shutter;
  _controls.color = hardwareState.color;
  _perControlSource.pan = "hardware";
  _perControlSource.tilt = "hardware";
  _perControlSource.intensity = "hardware";
  _perControlSource.iris = "hardware";
  _perControlSource.zoom = "hardware";
  _perControlSource.focus = "hardware";
  _perControlSource.shutter = "hardware";
  _perControlSource.color = "hardware";
  _status.lastUpdateMs = nowMs;
  _status.lastUpdateSource = "hardware";
}

bool ControllerRuntimeState::applyWebPatch(const ControlPatch& patch, uint32_t nowMs) {
  if (_mode == InputMode::kMonitorOnly) {
    return false;
  }

  if (patch.hasPan) {
    _controls.panNorm = clampNorm(patch.panNorm);
    _perControlSource.pan = "web";
  }
  if (patch.hasTilt) {
    _controls.tiltNorm = clampNorm(patch.tiltNorm);
    _perControlSource.tilt = "web";
  }
  if (patch.hasIntensity) {
    _controls.intensity = patch.intensity;
    _perControlSource.intensity = "web";
  }
  if (patch.hasIris) {
    _controls.iris = patch.iris;
    _perControlSource.iris = "web";
  }
  if (patch.hasZoom) {
    _controls.zoom = patch.zoom;
    _perControlSource.zoom = "web";
  }
  if (patch.hasFocus) {
    _controls.focus = patch.focus;
    _perControlSource.focus = "web";
  }
  if (patch.hasShutter) {
    _controls.shutter = patch.shutter;
    _perControlSource.shutter = "web";
  }
  if (patch.hasColor) {
    _controls.color = patch.color;
    _perControlSource.color = "web";
  }

  _status.lastUpdateMs = nowMs;
  _status.lastUpdateSource = "web";
  return true;
}

void ControllerRuntimeState::setActiveTargetNodeId(const String& nodeId, const String& source, uint32_t nowMs) {
  _activeTargetNodeId = nodeId;
  _status.lastUpdateMs = nowMs;
  _status.lastUpdateSource = source;
}

void ControllerRuntimeState::setBlackout(bool enabled, uint32_t nowMs) {
  _status.blackout = enabled;
  if (_status.blackout) {
    _controls.intensity = 0;
    _perControlSource.intensity = "web-blackout";
  }
  _status.lastUpdateMs = nowMs;
  _status.lastUpdateSource = "web";
}

void ControllerRuntimeState::requestHome(uint32_t nowMs) {
  _status.homeRequested = true;
  _status.lastUpdateMs = nowMs;
  _status.lastUpdateSource = "web";
  // TODO(HW): Convert this edge-triggered flag into homing motion actions.
}

void ControllerRuntimeState::setSensitivity(uint8_t value, uint32_t nowMs) {
  _status.sensitivity = clampU8(value);
  _status.lastUpdateMs = nowMs;
  _status.lastUpdateSource = "web";
}

void ControllerRuntimeState::setClaimRequested(bool requested, uint32_t nowMs) {
  _status.claimRequested = requested;
  _status.lastUpdateMs = nowMs;
  _status.lastUpdateSource = "web";
}

void ControllerRuntimeState::setReleaseRequested(bool requested, uint32_t nowMs) {
  _status.releaseRequested = requested;
  _status.lastUpdateMs = nowMs;
  _status.lastUpdateSource = "web";
}

}  // namespace sightline_v2
