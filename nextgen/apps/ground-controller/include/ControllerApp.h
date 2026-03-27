#pragma once

#include <Arduino.h>

#include "ButtonInputService.h"
#include "ControllerConfigStore.h"
#include "ControllerNetworkService.h"
#include "ControllerRuntimeState.h"
#include "ControllerWebService.h"
#include "ControllerStatus.h"
#include "ControlState.h"
#include "DiscoveryService.h"
#include "ControlTxService.h"
#include "EncoderInputService.h"
#include "FaderInputService.h"
#include "OwnershipClient.h"
#include "PanTiltMotionEngine.h"
#include "TargetSelector.h"

class ControllerApp {
 public:
  void begin();
  void tick(uint32_t nowMs);

 private:
  sightline_v2::ControllerConfigStore _configStore;
  sightline_v2::ControllerConfig _persistedConfig;
  sightline_v2::ControllerConfig _runtimeConfig;
  sightline_v2::ControllerStatus _status;
  sightline_v2::ControllerNetworkService _network;
  sightline_v2::DiscoveryService _discovery;
  sightline_v2::ControlTxService _controlTx;
  sightline_v2::OwnershipClient _ownershipClient;
  sightline_v2::EncoderInputService _encoders;
  sightline_v2::FaderInputService _faders;
  sightline_v2::ButtonInputService _buttons;
  sightline_v2::PanTiltMotionEngine _motion;
  sightline_v2::TargetSelector _targetSelector;
  sightline_v2::ControllerRuntimeState _runtimeState;
  sightline_v2::ControllerWebService _web;
  sightline_v2::ControlState _hardwareState;
  String _controllerId = "controller-001";
  String _targetNodeId;
  uint32_t _lastTickMs = 0;
  void readControls(uint32_t nowMs);
  void updateTargeting(uint32_t nowMs);
  void publishControlFrameIfDue(uint32_t nowMs);
};
