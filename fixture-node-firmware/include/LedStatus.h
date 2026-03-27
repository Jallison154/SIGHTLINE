#pragma once

#include <Arduino.h>

class LedStatus {
 public:
  void begin();
  void tick(uint32_t nowMs, bool networkConnected);
  void notifyActivity(uint32_t nowMs);

 private:
  void writeRaw(bool on);

  int _pin = -1;
  bool _activeLow = false;
  bool _initialized = false;
  uint32_t _activityUntilMs = 0;
  uint32_t _lastActivityMs = 0;
  uint32_t _blinkPhaseStartMs = 0;
  bool _lastOutput = false;
};
