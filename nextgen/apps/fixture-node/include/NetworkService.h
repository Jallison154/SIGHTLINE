#pragma once

#include <Arduino.h>

#include "NodeConfig.h"

namespace sightline_v2 {

class NetworkService {
 public:
  bool begin(const NodeConfig& config);
  void tick(uint32_t nowMs);
  bool ready() const { return _ready; }
  String localIpString() const;

 private:
  bool _ready = false;
};

}  // namespace sightline_v2
