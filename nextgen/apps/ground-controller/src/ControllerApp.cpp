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

  _controlState.panNorm = _motion.panNorm();
  _controlState.tiltNorm = _motion.tiltNorm();
  _controlState.intensity = f.intensity;
  _controlState.iris = f.iris;
  _controlState.zoom = f.zoom;
  _controlState.focus = f.focus;
  _controlState.shutter = b.shutterOpen ? 255 : 0;
  _controlState.color = b.color;
}

void ControllerApp::updateTargeting(uint32_t nowMs) {
  _targetSelector.updateFromDiscovery(nowMs, _discovery, _controllerId);
  if (_targetSelector.hasTarget()) {
    _targetNodeId = _targetSelector.targetNodeId();
    _ownershipClient.setTargetNodeId(_targetNodeId);
    _controlTx.setTargetNodeId(_targetNodeId);
    _controlTx.setTargetIp(_targetSelector.targetIp());
    _status.targetNodeId = _targetNodeId;
  }

  // TODO: Send claim/release/heartbeat messages and process responses.
  _status.ownsTarget = _ownershipClient.isOwner();
}

void ControllerApp::publishControlFrameIfDue(uint32_t nowMs) {
  static uint32_t lastTxMs = 0;
  const uint32_t kTxPeriodMs = _runtimeConfig.controlTxPeriodMs;
  if ((nowMs - lastTxMs) < kTxPeriodMs) return;
  lastTxMs = nowMs;

  if (!_ownershipClient.isOwner()) return;

  sightline_v2::ControlFrameV1 frame;
  frame.sessionId = "session-a";  // TODO: derive session from controller runtime.
  frame.frameSeq = nowMs;
  frame.sentAtMs = nowMs;
  frame.controls.hasPan = true;
  frame.controls.pan = _controlState.panNorm;
  frame.controls.hasTilt = true;
  frame.controls.tilt = _controlState.tiltNorm;
  frame.controls.hasIntensity = true;
  frame.controls.intensity = _controlState.intensity;
  frame.controls.hasIris = true;
  frame.controls.iris = _controlState.iris;
  frame.controls.hasZoom = true;
  frame.controls.zoom = _controlState.zoom;
  frame.controls.hasFocus = true;
  frame.controls.focus = _controlState.focus;
  frame.controls.hasShutter = true;
  frame.controls.shutter = _controlState.shutter;
  frame.controls.hasColor = true;
  frame.controls.color = _controlState.color;
  if (_controlTx.sendFrame(frame)) {
    _status.controlFramesSent++;
  }
}
