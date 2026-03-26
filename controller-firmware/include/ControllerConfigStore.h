#pragma once

#include <Arduino.h>

struct ControllerConfig {
  String nodeName = "SIGHTLINE-Controller";
  uint16_t artNetUniverse = 0;
  uint32_t txPeriodMs = 25;
  bool useBroadcast = true;
  String targetIp = "255.255.255.255";
  String fixtureProfileId = "generic-moving-head-16bit-20ch";
  float sensitivity = 0.0012f;
  float accelerationThresholdCountsPerSec = 90.0f;
  float accelerationGain = 2.0f;
  float accelerationCurve = 1.3f;
  float velocitySmoothing = 0.25f;
  float maxVelocityNormalizedPerSec = 2.4f;
};

class ControllerConfigStore {
 public:
  static constexpr uint16_t kCurrentSchemaVersion = 1;

  bool begin();
  ControllerConfig defaults() const;
  bool loadPersisted(ControllerConfig& outPersistedConfig, bool& outUsedDefaults);
  bool savePersisted(const ControllerConfig& config);
  bool resetToDefaults(ControllerConfig& outPersistedConfig);
  void applyToRuntime(const ControllerConfig& persistedConfig, ControllerConfig& runtimeConfig) const;

 private:
  bool validate(const ControllerConfig& config, String& outError) const;
  bool migrateIfNeeded(uint16_t storedVersion) const;
};
