#pragma once

#include <Arduino.h>

#include "NodeConfigStore.h"
#include "NodeStatus.h"

namespace sightline_v2 {

class NodeWebService {
 public:
  bool begin(NodeConfig& runtimeConfig, NodeConfigStore& configStore, const NodeStatus& status);
  void tick();

 private:
  NodeConfig* _runtimeConfig = nullptr;
  NodeConfigStore* _configStore = nullptr;
  const NodeStatus* _status = nullptr;
};

}  // namespace sightline_v2
