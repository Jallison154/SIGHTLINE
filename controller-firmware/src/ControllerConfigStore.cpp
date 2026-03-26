#include "ControllerConfigStore.h"

#include <Preferences.h>

namespace {
constexpr const char* kNs = "sightline-ctl";
constexpr const char* kSchemaVersionKey = "schemaVer";
}  // namespace

bool ControllerConfigStore::begin() { return true; }

ControllerConfig ControllerConfigStore::defaults() const { return ControllerConfig{}; }

bool ControllerConfigStore::validate(const ControllerConfig& c, String& outError) const {
  if (c.nodeName.isEmpty() || c.nodeName.length() > 32) {
    outError = "nodeName must be 1..32 chars";
    return false;
  }
  if (c.artNetUniverse > 32767) {
    outError = "artNetUniverse must be 0..32767";
    return false;
  }
  if (c.txPeriodMs < 10 || c.txPeriodMs > 100) {
    outError = "txPeriodMs must be 10..100";
    return false;
  }
  if (c.sensitivity <= 0.0f || c.sensitivity > 0.05f) {
    outError = "sensitivity out of range";
    return false;
  }
  if (c.velocitySmoothing < 0.0f || c.velocitySmoothing > 1.0f) {
    outError = "velocitySmoothing must be 0..1";
    return false;
  }
  return true;
}

bool ControllerConfigStore::migrateIfNeeded(uint16_t storedVersion) const {
  if (storedVersion == 0 || storedVersion == kCurrentSchemaVersion) {
    return true;
  }
  // TODO: Add explicit migrations for future schema versions.
  return false;
}

bool ControllerConfigStore::loadPersisted(ControllerConfig& outPersistedConfig, bool& outUsedDefaults) {
  outUsedDefaults = false;
  Preferences prefs;
  if (!prefs.begin(kNs, true)) {
    outPersistedConfig = defaults();
    outUsedDefaults = true;
    return false;
  }

  ControllerConfig loaded = defaults();
  loaded.nodeName = prefs.getString("nodeName", loaded.nodeName);
  loaded.artNetUniverse = prefs.getUShort("universe", loaded.artNetUniverse);
  loaded.txPeriodMs = prefs.getUInt("txPeriod", loaded.txPeriodMs);
  loaded.useBroadcast = prefs.getBool("broadcast", loaded.useBroadcast);
  loaded.targetIp = prefs.getString("targetIp", loaded.targetIp);
  loaded.fixtureProfileId = prefs.getString("profileId", loaded.fixtureProfileId);
  loaded.sensitivity = prefs.getFloat("sens", loaded.sensitivity);
  loaded.accelerationThresholdCountsPerSec = prefs.getFloat("accThr", loaded.accelerationThresholdCountsPerSec);
  loaded.accelerationGain = prefs.getFloat("accGain", loaded.accelerationGain);
  loaded.accelerationCurve = prefs.getFloat("accCurve", loaded.accelerationCurve);
  loaded.velocitySmoothing = prefs.getFloat("smooth", loaded.velocitySmoothing);
  loaded.maxVelocityNormalizedPerSec = prefs.getFloat("maxVel", loaded.maxVelocityNormalizedPerSec);
  const uint16_t storedVersion = prefs.getUShort(kSchemaVersionKey, 0);
  prefs.end();

  String err;
  if (!migrateIfNeeded(storedVersion) || !validate(loaded, err)) {
    outPersistedConfig = defaults();
    outUsedDefaults = true;
    return false;
  }

  outPersistedConfig = loaded;

  if (storedVersion == 0) {
    Preferences writePrefs;
    if (writePrefs.begin(kNs, false)) {
      writePrefs.putUShort(kSchemaVersionKey, kCurrentSchemaVersion);
      writePrefs.end();
    }
  }
  return true;
}

bool ControllerConfigStore::savePersisted(const ControllerConfig& config) {
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
  prefs.putUShort("universe", config.artNetUniverse);
  prefs.putUInt("txPeriod", config.txPeriodMs);
  prefs.putBool("broadcast", config.useBroadcast);
  prefs.putString("targetIp", config.targetIp);
  prefs.putString("profileId", config.fixtureProfileId);
  prefs.putFloat("sens", config.sensitivity);
  prefs.putFloat("accThr", config.accelerationThresholdCountsPerSec);
  prefs.putFloat("accGain", config.accelerationGain);
  prefs.putFloat("accCurve", config.accelerationCurve);
  prefs.putFloat("smooth", config.velocitySmoothing);
  prefs.putFloat("maxVel", config.maxVelocityNormalizedPerSec);
  prefs.end();
  return true;
}

bool ControllerConfigStore::resetToDefaults(ControllerConfig& outPersistedConfig) {
  outPersistedConfig = defaults();
  return savePersisted(outPersistedConfig);
}

void ControllerConfigStore::applyToRuntime(const ControllerConfig& persistedConfig,
                                           ControllerConfig& runtimeConfig) const {
  runtimeConfig = persistedConfig;
}
