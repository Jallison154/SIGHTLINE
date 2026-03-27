#pragma once

#include <Arduino.h>

#include "ConfigStore.h"
#include "NetworkManager.h"
#include "StatusTracker.h"

class WebUiServer {
 public:
  bool begin(NodeConfig& liveConfig, ConfigStore& configStore, const StatusTracker& status, NetworkManager& network);
  void tick();

 private:
  void scheduleReboot(uint32_t delayMs);

  NodeConfig* _config = nullptr;
  ConfigStore* _configStore = nullptr;
  const StatusTracker* _status = nullptr;
  NetworkManager* _network = nullptr;
  bool _rebootScheduled = false;
  uint32_t _rebootAtMs = 0;
};
