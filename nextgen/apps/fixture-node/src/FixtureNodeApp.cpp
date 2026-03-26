#include "FixtureNodeApp.h"

void FixtureNodeApp::begin() {
  bool usedDefaults = false;
  _configStore.begin();
  _configStore.loadPersisted(_persistedConfig, usedDefaults);
  String configErr;
  _configStore.applyToRuntime(_persistedConfig, _runtimeConfig, configErr);
  _nodeId = _runtimeConfig.nodeId;

  _network.begin(_runtimeConfig);
  _status.networkReady = _network.ready();

  _discovery.begin(_runtimeConfig.discoveryPort);
  _ownership.begin(_nodeId);
  _controlRx.begin(_runtimeConfig.controlPort, _nodeId);
  _status.discoveryReady = true;
  _status.controlRxReady = true;

  _profileManager.begin();
  _profileWebApi.begin(_profileManager);
  _status.profileActive = _profileManager.hasActiveProfile();

  _dmx.begin();
  _status.dmxReady = true;
  _web.begin(_runtimeConfig, _configStore, _status);
  _status.webReady = true;

  sightline_v2::DiscoveryMessageV1 self;
  self.deviceId = _nodeId;             // TODO(HW): derive from MAC or persisted unique ID.
  self.deviceType = sightline_v2::DiscoveryDeviceType::kFixtureNode;
  self.friendlyName = "Fixture Node";
  self.ip = _network.localIpString();
  self.firmwareVersion = "v0.1.0";
  self.health = sightline_v2::DiscoveryHealth::kOk;
  self.fixtureProfileName = _profileManager.hasActiveProfile() ? _profileManager.activeProfileId() : "none";
  self.claimState = sightline_v2::ClaimState::kAvailable;
  _discovery.setSelfAnnouncement(self);
}

void FixtureNodeApp::tick(uint32_t nowMs) {
  _status.uptimeMs = nowMs;
  _network.tick(nowMs);
  serviceDiscovery(nowMs);
  serviceClaimState(nowMs);
  serviceControlRx(nowMs);
  renderDmx(nowMs);
  serviceWeb(nowMs);
}

void FixtureNodeApp::serviceDiscovery(uint32_t nowMs) {
  sightline_v2::DiscoveryMessageV1 self;
  self.deviceId = _nodeId;
  self.deviceType = sightline_v2::DiscoveryDeviceType::kFixtureNode;
  self.friendlyName = "Fixture Node";
  self.ip = _network.localIpString();
  self.firmwareVersion = "v0.1.0";
  self.health = sightline_v2::DiscoveryHealth::kOk;
  self.fixtureProfileName = _profileManager.hasActiveProfile() ? _profileManager.activeProfileId() : "none";
  self.claimState = _ownership.state().claimed ? sightline_v2::ClaimState::kClaimed : sightline_v2::ClaimState::kAvailable;
  self.assignedControllerId = _ownership.state().controllerId;
  _discovery.setSelfAnnouncement(self);
  _discovery.tick(nowMs);
}

void FixtureNodeApp::serviceClaimState(uint32_t nowMs) {
  _ownership.tick(nowMs);
  _status.claimed = _ownership.state().claimed;
  _status.assignedControllerId = _ownership.state().controllerId;
  // TODO: Receive ownership claim/release/heartbeat messages from network and route to OwnershipManager.
  // TODO: Publish ownership status responses over network transport.
}

void FixtureNodeApp::serviceControlRx(uint32_t nowMs) {
  const auto& own = _ownership.state();
  const bool accepted = _controlRx.poll(nowMs, own.controllerId, own.claimed);
  if (accepted && _controlRx.hasLatestFrame()) {
    _currentControls = _controlRx.latestFrame().controls;
  }
  _status.controlFramesAccepted = _controlRx.status().framesAccepted;
}

void FixtureNodeApp::renderDmx(uint32_t nowMs) {
  if (_profileManager.hasActiveProfile()) {
    _mapper.map(_currentControls, _profileManager.activeProfile(), _dmxUniverse, sizeof(_dmxUniverse));
  }
  _dmx.tick(nowMs, _dmxUniverse);
  _status.dmxFramesOutput = _dmx.framesOutput();
}

void FixtureNodeApp::serviceWeb(uint32_t nowMs) {
  (void)nowMs;
  _profileWebApi.tick();
  _web.tick();
}
