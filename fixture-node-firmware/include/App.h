#pragma once

#include <Arduino.h>

#include "ArtNetReceiver.h"
#include "ConfigStore.h"
#include "DmxBuffer.h"
#include "DmxOutput.h"
#include "StatusTracker.h"
#include "WebUiServer.h"

class App {
 public:
  void begin();
  void tick(uint32_t nowMs);

 private:
  void serviceArtNet(uint32_t nowMs);

  NodeConfig _config;
  ConfigStore _configStore;
  DmxBuffer _dmxBuffer;
  ArtNetReceiver _artNetReceiver;
  DmxOutput _dmxOutput;
  StatusTracker _status;
  WebUiServer _webUi;
};
