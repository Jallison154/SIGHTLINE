#include "ConfigStore.h"

bool ConfigStore::begin() {
  // TODO: Initialize NVS namespace.
  return true;
}

bool ConfigStore::load(NodeConfig& outConfig) {
  // TODO: Load and validate settings from NVS.
  outConfig = NodeConfig{};
  return true;
}

bool ConfigStore::save(const NodeConfig& config) {
  // TODO: Persist settings atomically.
  (void)config;
  return true;
}
