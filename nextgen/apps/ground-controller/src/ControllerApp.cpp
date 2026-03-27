#include "ControllerApp.h"

void ControllerApp::begin() {
  _lastTickMs = millis();

  bool usedDefaults = false;
  _configStore.begin();
  _configStore.loadPersisted(_persistedConfig, usedDefaults);
  String cfgErr;
  _configStore.applyToRuntime(_persistedConfig, _runtimeConfig, cfgErr);

  _controllerId = _runtimeConfig.controllerId;
  _network.begin(_runtimeConfig);
  _status.networkReady = _network.ready();

  _encoders.begin();
  _faders.begin();
  _buttons.begin();
  _motion.configure(0.0012f, 0.25f, 2.4f);

  _discovery.begin(_runtimeConfig.discoveryPort);
  _status.discoveryReady = true;

  _ownershipClient.begin(_controllerId);
  _status.ownershipReady = true;
  _controlTx.begin(_runtimeConfig.controlPort);
  _controlTx.setControllerId(_controllerId);
  _status.controlTxReady = true;

  // Controller web UI for test-control + service workflows.
  // TODO(HW): secure access/auth when deployed on shared production networks.
  _web.begin(_runtimeState, _status, _targetSelector, _ownershipClient, _controllerId);
  _status.webReady = _web.ready();

  sightline_v2::DiscoveryMessageV1 self;
  self.deviceId = _controllerId;       // TODO(HW): derive from MAC or persisted unique ID.
  self.deviceType = sightline_v2::DiscoveryDeviceType::kController;
  self.friendlyName = _runtimeConfig.friendlyName;
  self.ip = _network.localIpString();
  self.firmwareVersion = "v0.1.0";
  self.health = sightline_v2::DiscoveryHealth::kOk;
  _discovery.setSelfAnnouncement(self);
}

void ControllerApp::tick(uint32_t nowMs) {
  _status.uptimeMs = nowMs;
  _network.tick(nowMs);
  _ownershipClient.tick(nowMs);
  _discovery.tick(nowMs);
  readControls(nowMs);
  updateTargeting(nowMs);
  _web.tick();
  _status.webReady = _web.ready();
  publishControlFrameIfDue(nowMs);
}

void ControllerApp::readControls(uint32_t nowMs) {
  uint32_t dtMs = nowMs - _lastTickMs;
  if (dtMs > 100) dtMs = 100;
  _lastTickMs = nowMs;

  _encoders.tick(nowMs);
  _faders.tick(nowMs);
  _buttons.tick(nowMs);

  const auto deltas = _encoders.readAndResetDeltas();
  _motion.tick(deltas, dtMs);

  const auto f = _faders.current();
  const auto b = _buttons.current();

  _hardwareState.panNorm = _motion.panNorm();
  _hardwareState.tiltNorm = _motion.tiltNorm();
  _hardwareState.intensity = f.intensity;
  _hardwareState.iris = f.iris;
  _hardwareState.zoom = f.zoom;
  _hardwareState.focus = f.focus;
  _hardwareState.shutter = b.shutterOpen ? 255 : 0;
  _hardwareState.color = b.color;
  _runtimeState.applyHardwareState(_hardwareState, nowMs);
}

void ControllerApp::updateTargeting(uint32_t nowMs) {
  _targetSelector.updateFromDiscovery(nowMs, _discovery, _controllerId);
  if (_targetSelector.hasTarget()) {
    _targetNodeId = _targetSelector.targetNodeId();
    _ownershipClient.setTargetNodeId(_targetNodeId);
    _controlTx.setTargetNodeId(_targetNodeId);
    _controlTx.setTargetIp(_targetSelector.targetIp());
    _status.targetNodeId = _targetNodeId;
    _runtimeState.setActiveTargetNodeId(_targetNodeId, "selector", nowMs);
  }

  // TODO(NET): Send claim/release/heartbeat messages and process responses.
  // For now this keeps web claim/release intent available for diagnostics/UI.
  if (_runtimeState.status().claimRequested) {
    // Placeholder until claim transport is hooked up.
  }
  if (_runtimeState.status().releaseRequested) {
    // Placeholder until release transport is hooked up.
  }
  _status.ownsTarget = _ownershipClient.isOwner();
}

void ControllerApp::publishControlFrameIfDue(uint32_t nowMs) {
  static uint32_t lastTxMs = 0;
  const uint32_t kTxPeriodMs = _runtimeConfig.controlTxPeriodMs;
  if ((nowMs - lastTxMs) < kTxPeriodMs) return;
  lastTxMs = nowMs;

  const bool allowTxFromWebTools = _runtimeState.status().claimRequested;
  if (!_ownershipClient.isOwner() && !allowTxFromWebTools) return;

  sightline_v2::ControlFrameV1 frame;
  frame.sessionId = "session-a";  // TODO: derive session from controller runtime.
  frame.frameSeq = nowMs;
  frame.sentAtMs = nowMs;
  frame.controls.hasPan = true;
  frame.controls.pan16 = static_cast<uint16_t>(_runtimeState.controls().panNorm * 65535.0f);
  frame.controls.pan = _runtimeState.controls().panNorm;
  frame.controls.hasTilt = true;
  frame.controls.tilt16 = static_cast<uint16_t>(_runtimeState.controls().tiltNorm * 65535.0f);
  frame.controls.tilt = _runtimeState.controls().tiltNorm;
  frame.controls.hasIntensity = true;
  frame.controls.intensity = _runtimeState.status().blackout ? 0 : _runtimeState.controls().intensity;
  frame.controls.hasIris = true;
  frame.controls.iris = _runtimeState.controls().iris;
  frame.controls.hasZoom = true;
  frame.controls.zoom = _runtimeState.controls().zoom;
  frame.controls.hasFocus = true;
  frame.controls.focus = _runtimeState.controls().focus;
  frame.controls.hasShutter = true;
  frame.controls.shutter = _runtimeState.controls().shutter;
  frame.controls.hasColor = true;
  frame.controls.color = _runtimeState.controls().color;
  if (_controlTx.sendFrame(frame)) {
    _status.controlFramesSent++;
  }
}
