#include "ConfigStore.h"

#include <Preferences.h>

namespace {
constexpr const char* kNs = "sightline-node";
constexpr const char* kSchemaVersionKey = "schemaVer";

uint8_t toNetworkModeValue(NodeConfig::NetworkMode mode) {
  return static_cast<uint8_t>(mode);
}

NodeConfig::NetworkMode networkModeFromValue(uint8_t value) {
  return value == static_cast<uint8_t>(NodeConfig::NetworkMode::WiFiStation) ? NodeConfig::NetworkMode::WiFiStation
                                                                              : NodeConfig::NetworkMode::Ethernet;
}
}

bool ConfigStore::begin() {
  // NVS namespace is opened per operation, no long-lived handle needed here.
  return true;
}

NodeConfig ConfigStore::defaults() const { return NodeConfig{}; }

bool ConfigStore::validate(const NodeConfig& config, String& outError) {
  if (config.nodeName.length() == 0 || config.nodeName.length() > 32) {
    outError = "nodeName must be 1..32 chars";
    return false;
  }
  if (config.fixtureLabel.length() > 64) {
    outError = "fixtureLabel max length is 64";
    return false;
  }
  if (config.universe > 32767) {
    outError = "universe must be 0..32767";
    return false;
  }
  if (config.dmxStartAddress == 0 || config.dmxStartAddress > 512) {
    outError = "dmxStartAddress must be 1..512";
    return false;
  }
  if (config.networkMode == NodeConfig::NetworkMode::WiFiStation && config.wifiSsid.length() == 0) {
    outError = "wifiSsid required in Wi-Fi Station mode";
    return false;
  }
  if (config.setupApSsid.length() == 0 || config.setupApSsid.length() > 32) {
    outError = "setupApSsid must be 1..32 chars";
    return false;
  }
  return true;
}

bool ConfigStore::migrateIfNeeded(uint16_t storedVersion) {
  if (storedVersion == 0) {
    // Legacy config without version marker: treat as schema v1.
    return true;
  }
  if (storedVersion == 1 && kCurrentSchemaVersion == 2) {
    // v1 -> v2: new network fields get safe defaults from NodeConfig{}.
    return true;
  }
  if (storedVersion == kCurrentSchemaVersion) {
    return true;
  }

  // TODO: Add explicit migrations when schema changes.
  return false;
}

bool ConfigStore::loadPersisted(NodeConfig& outPersistedConfig, bool& outUsedDefaults) {
  outUsedDefaults = false;
  Preferences prefs;
  if (!prefs.begin(kNs, true)) {
    outPersistedConfig = defaults();
    _cachedConfig = outPersistedConfig;
    outUsedDefaults = true;
    return false;
  }

  NodeConfig loaded = defaults();
  loaded.nodeName = prefs.getString("nodeName", loaded.nodeName);
  loaded.fixtureLabel = prefs.getString("fixtureLabel", loaded.fixtureLabel);
  loaded.networkMode = networkModeFromValue(prefs.getUChar("netMode", toNetworkModeValue(loaded.networkMode)));
  loaded.wifiSsid = prefs.getString("wifiSsid", loaded.wifiSsid);
  loaded.wifiPassword = prefs.getString("wifiPass", loaded.wifiPassword);
  loaded.fallbackToSetupAp = prefs.getBool("fallbackAp", loaded.fallbackToSetupAp);
  loaded.setupApSsid = prefs.getString("apSsid", loaded.setupApSsid);
  loaded.setupApPassword = prefs.getString("apPass", loaded.setupApPassword);
  loaded.universe = prefs.getUShort("universe", loaded.universe);
  loaded.dmxStartAddress = prefs.getUShort("dmxStart", loaded.dmxStartAddress);
  loaded.dhcp = prefs.getBool("dhcp", loaded.dhcp);
  loaded.staticIp = prefs.getString("staticIp", loaded.staticIp);
  loaded.subnetMask = prefs.getString("subnet", loaded.subnetMask);
  loaded.gateway = prefs.getString("gateway", loaded.gateway);
  const uint16_t storedVersion = prefs.getUShort(kSchemaVersionKey, 0);
  prefs.end();

  String err;
  if (storedVersion > 0) {
    const bool migrateOk = migrateIfNeeded(storedVersion);
    if (!migrateOk) {
      outPersistedConfig = defaults();
      _cachedConfig = outPersistedConfig;
      outUsedDefaults = true;
      return false;
    }
  }

  if (!validate(loaded, err)) {
    outPersistedConfig = defaults();
    _cachedConfig = outPersistedConfig;
    outUsedDefaults = true;
    return false;
  }

  outPersistedConfig = loaded;
  _cachedConfig = loaded;

  if (storedVersion == 0) {
    Preferences writePrefs;
    if (writePrefs.begin(kNs, false)) {
      writePrefs.putUShort(kSchemaVersionKey, kCurrentSchemaVersion);
      writePrefs.end();
    }
  }
  return true;
}

bool ConfigStore::savePersisted(const NodeConfig& config) {
  String err;
  if (!validate(config, err)) {
    return false;
  }

  Preferences prefs;
  if (!prefs.begin(kNs, false)) {
    return false;
  }
  prefs.putUShort(kSchemaVersionKey, kCurrentSchemaVersion);
  prefs.putString("nodeName", config.nodeName);
  prefs.putString("fixtureLabel", config.fixtureLabel);
  prefs.putUChar("netMode", toNetworkModeValue(config.networkMode));
  prefs.putString("wifiSsid", config.wifiSsid);
  prefs.putString("wifiPass", config.wifiPassword);
  prefs.putBool("fallbackAp", config.fallbackToSetupAp);
  prefs.putString("apSsid", config.setupApSsid);
  prefs.putString("apPass", config.setupApPassword);
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

bool ConfigStore::resetToDefaults(NodeConfig& outPersistedConfig) {
  outPersistedConfig = defaults();
  return savePersisted(outPersistedConfig);
}

void ConfigStore::applyToRuntime(const NodeConfig& persistedConfig, NodeConfig& runtimeConfig) const {
  runtimeConfig = persistedConfig;
}

bool ConfigStore::load(NodeConfig& outConfig) {
  bool usedDefaults = false;
  return loadPersisted(outConfig, usedDefaults);
}

bool ConfigStore::save(const NodeConfig& config) { return savePersisted(config); }
