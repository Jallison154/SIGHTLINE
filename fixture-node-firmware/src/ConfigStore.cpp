#include "ConfigStore.h"

#include <Preferences.h>

bool ConfigStore::begin() {
  // NVS namespace is opened per operation, no long-lived handle needed here.
  return true;
}

bool ConfigStore::validate(const NodeConfig& config, String& outError) {
  if (config.nodeName.length() == 0 || config.nodeName.length() > 32) {
    outError = "nodeName must be 1..32 chars";
    return false;
  }
  if (config.dmxStartAddress == 0 || config.dmxStartAddress > 512) {
    outError = "dmxStartAddress must be 1..512";
    return false;
  }
  return true;
}

bool ConfigStore::load(NodeConfig& outConfig) {
  Preferences prefs;
  if (!prefs.begin("sightline-node", true)) {
    outConfig = NodeConfig{};
    _cachedConfig = outConfig;
    return false;
  }

  NodeConfig loaded;
  loaded.nodeName = prefs.getString("nodeName", loaded.nodeName);
  loaded.universe = prefs.getUShort("universe", loaded.universe);
  loaded.dmxStartAddress = prefs.getUShort("dmxStart", loaded.dmxStartAddress);
  loaded.dhcp = prefs.getBool("dhcp", loaded.dhcp);
  loaded.staticIp = prefs.getString("staticIp", loaded.staticIp);
  loaded.subnetMask = prefs.getString("subnet", loaded.subnetMask);
  loaded.gateway = prefs.getString("gateway", loaded.gateway);
  prefs.end();

  String err;
  if (!validate(loaded, err)) {
    outConfig = NodeConfig{};
    _cachedConfig = outConfig;
    return false;
  }

  outConfig = loaded;
  _cachedConfig = loaded;
  return true;
}

bool ConfigStore::save(const NodeConfig& config) {
  String err;
  if (!validate(config, err)) {
    return false;
  }

  Preferences prefs;
  if (!prefs.begin("sightline-node", false)) {
    return false;
  }
  prefs.putString("nodeName", config.nodeName);
  prefs.putUShort("universe", config.universe);
  prefs.putUShort("dmxStart", config.dmxStartAddress);
  prefs.putBool("dhcp", config.dhcp);
  prefs.putString("staticIp", config.staticIp);
  prefs.putString("subnet", config.subnetMask);
  prefs.putString("gateway", config.gateway);
  prefs.end();

  _cachedConfig = config;
  return true;
}
