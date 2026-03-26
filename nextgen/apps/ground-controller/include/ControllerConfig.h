#pragma once

#include <Arduino.h>

namespace sightline_v2 {

struct ControllerConfig {
  String controllerId = "controller-001";
  String friendlyName = "Ground Controller";
  bool dhcp = true;
  String staticIp = "192.168.1.100";
  String subnet = "255.255.255.0";
  String gateway = "192.168.1.1";
  uint16_t discoveryPort = 5568;
  uint16_t controlPort = 5570;
  uint32_t controlTxPeriodMs = 25;  // ~40Hz
};

}  // namespace sightline_v2
