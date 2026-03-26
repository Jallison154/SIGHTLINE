#pragma once

#include <Arduino.h>

struct NodeConfig {
  String nodeName = "SIGHTLINE-Node";
  uint16_t universe = 0;
  uint16_t dmxStartAddress = 1;
  bool dhcp = true;
  String staticIp = "192.168.1.200";
  String subnetMask = "255.255.255.0";
  String gateway = "192.168.1.1";
};

class ConfigStore {
 public:
  bool begin();
  bool load(NodeConfig& outConfig);
  bool save(const NodeConfig& config);

 private:
  bool validate(const NodeConfig& config, String& outError);
  NodeConfig _cachedConfig;
};
