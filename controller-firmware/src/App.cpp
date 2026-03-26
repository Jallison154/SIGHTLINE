#include "App.h"

void App::begin() {
  _lastTickMs = millis();
  _lastTxMs = 0;

  _controlInput.begin();
  _profileStore.begin();
  _artNetSender.begin();

  String profileError;
  if (!_profileStore.loadSelectedProfile(_activeProfile, profileError)) {
    Serial.printf("Profile load failed: %s\n", profileError.c_str());
  } else {
    Serial.printf("Loaded profile: %s %s [%s]\n", _activeProfile.manufacturer.c_str(),
                  _activeProfile.fixtureName.c_str(), _activeProfile.modeName.c_str());
    PanTiltTuning tuning;
    tuning.sensitivity = 0.0012f;
    tuning.accelerationThresholdCountsPerSec = 90.0f;
    tuning.accelerationGain = 2.0f;
    tuning.accelerationCurve = 1.3f;
    tuning.velocitySmoothing = _activeProfile.smoothingAlpha;
    tuning.maxVelocityNormalizedPerSec = 2.4f;
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
  _artNetSender.sendDmx(0, _universe.data(), _universe.size());
}
