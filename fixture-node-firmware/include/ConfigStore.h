#pragma once

#include <Arduino.h>

struct NodeConfig {
  String nodeName = "SIGHTLINE-Node";
  String fixtureLabel = "";
  uint16_t universe = 0;
  uint16_t dmxStartAddress = 1;
  bool dhcp = true;
  String staticIp = "192.168.1.200";
  String subnetMask = "255.255.255.0";
  String gateway = "192.168.1.1";
};

class ConfigStore {
 public:
  static constexpr uint16_t kCurrentSchemaVersion = 1;

  bool begin();
  NodeConfig defaults() const;
  bool loadPersisted(NodeConfig& outPersistedConfig, bool& outUsedDefaults);
  bool savePersisted(const NodeConfig& config);
  bool resetToDefaults(NodeConfig& outPersistedConfig);
  void applyToRuntime(const NodeConfig& persistedConfig, NodeConfig& runtimeConfig) const;

  // Legacy wrappers kept for current call sites.
  bool load(NodeConfig& outConfig);
  bool save(const NodeConfig& config);

 private:
  bool validate(const NodeConfig& config, String& outError);
  bool migrateIfNeeded(uint16_t storedVersion);
  NodeConfig _cachedConfig;
};
