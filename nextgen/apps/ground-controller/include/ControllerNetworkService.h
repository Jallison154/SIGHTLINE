#pragma once

#include <Arduino.h>

#include "ControllerConfig.h"

namespace sightline_v2 {

class ControllerNetworkService {
 public:
  bool begin(const ControllerConfig& config);
  void tick(uint32_t nowMs);
  bool ready() const { return _ready; }
  String localIpString() const;

 private:
  bool _ready = false;
};

}  // namespace sightline_v2
