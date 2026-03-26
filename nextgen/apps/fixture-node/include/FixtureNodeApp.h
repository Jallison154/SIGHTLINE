#pragma once

#include <Arduino.h>

#include "AbstractToDmxMapper.h"
#include "DiscoveryService.h"
#include "ControlRxService.h"
#include "DmxOutputService.h"
#include "FixtureProfileManager.h"
#include "FixtureProfileWebApi.h"
#include "NetworkService.h"
#include "NodeConfigStore.h"
#include "NodeStatus.h"
#include "NodeWebService.h"
#include "OwnershipManager.h"

class FixtureNodeApp {
 public:
  void begin();
  void tick(uint32_t nowMs);

 private:
  sightline_v2::NodeConfigStore _configStore;
  sightline_v2::NodeConfig _persistedConfig;
  sightline_v2::NodeConfig _runtimeConfig;
  sightline_v2::NodeStatus _status;
  sightline_v2::NetworkService _network;
  sightline_v2::DiscoveryService _discovery;
  sightline_v2::ControlRxService _controlRx;
  sightline_v2::FixtureProfileManager _profileManager;
  sightline_v2::FixtureProfileWebApi _profileWebApi;
  sightline_v2::NodeWebService _web;
  sightline_v2::AbstractToDmxMapper _mapper;
  sightline_v2::DmxOutputService _dmx;
  sightline_v2::OwnershipManager _ownership;
  uint8_t _dmxUniverse[512] = {0};
  sightline_v2::AbstractControls _currentControls;
  String _nodeId = "fixture-node-001";
  void serviceDiscovery(uint32_t nowMs);
  void serviceClaimState(uint32_t nowMs);
  void serviceControlRx(uint32_t nowMs);
  void renderDmx(uint32_t nowMs);
  void serviceWeb(uint32_t nowMs);
};
