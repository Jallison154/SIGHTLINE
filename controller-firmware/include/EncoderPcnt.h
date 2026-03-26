#pragma once

#include <Arduino.h>

#include <driver/pcnt.h>

struct EncoderPcntConfig {
  int pinA = -1;
  int pinB = -1;
  pcnt_unit_t unit = PCNT_UNIT_0;
  uint16_t glitchFilterCycles = 100;  // Reject very short pulses/noise.
  bool invertDirection = false;
};

class EncoderPcnt {
 public:
  bool begin(const EncoderPcntConfig& config);
  int32_t readAndResetDelta();
  bool isReady() const { return _ready; }

 private:
  bool _ready = false;
  bool _invertDirection = false;
  pcnt_unit_t _unit = PCNT_UNIT_0;
};
