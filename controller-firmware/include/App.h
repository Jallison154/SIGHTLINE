#pragma once

#include <Arduino.h>

#include "ArtNetSender.h"
#include "ControlInput.h"
#include "DmxMapper.h"
#include "DmxUniverse.h"
#include "FixtureProfileStore.h"
#include "PanTiltEngine.h"

class App {
 public:
  void begin();
  void tick(uint32_t nowMs);

 private:
  void updateControlState(uint32_t nowMs, uint32_t dtMs);
  void updateDmxFrame();
  void transmitIfDue(uint32_t nowMs);

  ControlInput _controlInput;
  PanTiltEngine _panTiltEngine;
  FixtureProfileStore _profileStore;
  DmxMapper _mapper;
  DmxUniverse _universe;
  ArtNetSender _artNetSender;

  FixtureProfile _activeProfile;
  ControlState _controlState;

  uint32_t _lastTickMs = 0;
  uint32_t _lastTxMs = 0;
  uint32_t _txPeriodMs = 25;  // 40Hz default
  uint16_t _artNetUniverse = 0;
};
