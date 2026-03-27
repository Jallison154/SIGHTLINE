#include "LedStatus.h"

namespace {
constexpr uint32_t kBlinkPeriodMs = 1000;
constexpr uint32_t kBlinkOnMs = 140;
constexpr uint32_t kActivityFlashMs = 90;
constexpr uint32_t kMinActivityIntervalMs = 140;
}

void LedStatus::begin() {
#ifdef LED_BUILTIN
  _pin = LED_BUILTIN;
#else
  // TODO(HW): Confirm onboard status LED GPIO for the final fixture-node board.
  _pin = 2;
#endif

  // TODO(HW): Some ESP32 boards wire onboard LED as active-low; expose this in config if needed.
  _activeLow = false;
  pinMode(_pin, OUTPUT);
  _initialized = true;
  _blinkPhaseStartMs = millis();
  writeRaw(false);
}

void LedStatus::notifyActivity(uint32_t nowMs) {
  if (!_initialized) {
    return;
  }
  if (_lastActivityMs > 0 && (nowMs - _lastActivityMs) < kMinActivityIntervalMs) {
    return;
  }
  _lastActivityMs = nowMs;
  _activityUntilMs = nowMs + kActivityFlashMs;
}

void LedStatus::tick(uint32_t nowMs, bool networkConnected) {
  if (!_initialized) {
    return;
  }

  bool targetOn = false;
  if (nowMs < _activityUntilMs) {
    // Activity flash has priority over base pattern.
    targetOn = true;
  } else if (networkConnected) {
    targetOn = true;
  } else {
    // Slow blink while connecting/disconnected.
    uint32_t phase = nowMs - _blinkPhaseStartMs;
    if (phase >= kBlinkPeriodMs) {
      _blinkPhaseStartMs = nowMs;
      phase = 0;
    }
    targetOn = phase < kBlinkOnMs;
  }

  if (targetOn != _lastOutput) {
    writeRaw(targetOn);
    _lastOutput = targetOn;
  }
}

void LedStatus::writeRaw(bool on) {
  if (!_initialized) {
    return;
  }
  digitalWrite(_pin, (_activeLow ? !on : on) ? HIGH : LOW);
}
