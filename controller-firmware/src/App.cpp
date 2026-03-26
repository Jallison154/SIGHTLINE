#include "App.h"
#include <IPAddress.h>

void App::begin() {
  _lastTickMs = millis();
  _lastTxMs = 0;

  _configStore.begin();
  bool usedDefaults = false;
  if (!_configStore.loadPersisted(_persistedConfig, usedDefaults)) {
    Serial.println("Controller config invalid/unavailable, using defaults");
  }
  _configStore.applyToRuntime(_persistedConfig, _runtimeConfig);

  _txPeriodMs = _runtimeConfig.txPeriodMs;
  _artNetUniverse = _runtimeConfig.artNetUniverse;

  _controlInput.begin();
  _profileStore.begin();
  _artNetSender.begin();
  _artNetSender.setUniverse(_artNetUniverse);
  if (!_runtimeConfig.useBroadcast) {
    // TODO(HW): Add strict IPv4 parsing/validation.
    IPAddress ip;
    if (ip.fromString(_runtimeConfig.targetIp)) {
      _artNetSender.setTargetIp(ip);
    }
  }

  String profileError;
  if (!_profileStore.loadSelectedProfile(_activeProfile, profileError)) {
    Serial.printf("Profile load failed: %s\n", profileError.c_str());
  } else {
    Serial.printf("Loaded profile: %s %s [%s]\n", _activeProfile.manufacturer.c_str(),
                  _activeProfile.fixtureName.c_str(), _activeProfile.modeName.c_str());
    PanTiltTuning tuning;
    tuning.sensitivity = _runtimeConfig.sensitivity;
    tuning.accelerationThresholdCountsPerSec = _runtimeConfig.accelerationThresholdCountsPerSec;
    tuning.accelerationGain = _runtimeConfig.accelerationGain;
    tuning.accelerationCurve = _runtimeConfig.accelerationCurve;
    tuning.velocitySmoothing = _runtimeConfig.velocitySmoothing;
    tuning.maxVelocityNormalizedPerSec = _runtimeConfig.maxVelocityNormalizedPerSec;
    _panTiltEngine.configure(_activeProfile.deadband, _activeProfile.panSpeedScale, _activeProfile.tiltSpeedScale,
                             tuning);
  }

  _universe.clear();
}

void App::tick(uint32_t nowMs) {
  uint32_t dtMs = nowMs - _lastTickMs;
  if (dtMs > 100) {
    dtMs = 100;
  }
  _lastTickMs = nowMs;

  updateControlState(nowMs, dtMs);
  updateDmxFrame();
  transmitIfDue(nowMs);
}

void App::updateControlState(uint32_t nowMs, uint32_t dtMs) {
  _controlInput.update(nowMs, dtMs, _controlState);
  _panTiltEngine.updateFromEncoderDelta(_controlState.panEncoderDelta, _controlState.tiltEncoderDelta, dtMs);
}

void App::updateDmxFrame() {
  _mapper.apply(_activeProfile, _controlState, _panTiltEngine.pan16(), _panTiltEngine.tilt16(), _universe);
}

void App::transmitIfDue(uint32_t nowMs) {
  if ((nowMs - _lastTxMs) < _txPeriodMs) {
    return;
  }
  _lastTxMs = nowMs;

  // Universe is sent as full 512-byte frame each transmit cycle.
  _artNetSender.setBuffer(_universe.data(), _universe.size());
  _artNetSender.sendFrame();
}
