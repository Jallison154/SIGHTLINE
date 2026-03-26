#pragma once

#include <Arduino.h>

namespace sightline_v2 {

struct NodeConfig {
  String nodeId = "fixture-node-001";
  String friendlyName = "Fixture Node";
  bool dhcp = true;
  String staticIp = "192.168.1.200";
  String subnet = "255.255.255.0";
  String gateway = "192.168.1.1";
  uint16_t discoveryPort = 5568;
  uint16_t controlPort = 5570;
  uint16_t dmxStartAddress = 1;
};

}  // namespace sightline_v2
