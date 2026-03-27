#pragma once

#include <Arduino.h>

#include "ConfigStore.h"
#include "NetworkManager.h"
#include "StatusTracker.h"

class WebUiServer {
 public:
  bool begin(NodeConfig& liveConfig, ConfigStore& configStore, const StatusTracker& status, const NetworkManager& network);
  void tick();

 private:
  NodeConfig* _config = nullptr;
  ConfigStore* _configStore = nullptr;
  const StatusTracker* _status = nullptr;
  const NetworkManager* _network = nullptr;
};
