#pragma once

#include <Arduino.h>

#include "ConfigStore.h"
#include "StatusTracker.h"

class WebUiServer {
 public:
  bool begin(NodeConfig& liveConfig, ConfigStore& configStore, const StatusTracker& status);
  void tick();

 private:
  NodeConfig* _config = nullptr;
  ConfigStore* _configStore = nullptr;
  const StatusTracker* _status = nullptr;
};
