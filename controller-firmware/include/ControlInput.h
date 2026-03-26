#pragma once

#include <Arduino.h>

struct ControlState {
  // Raw encoder movement since last update tick (detents or counts).
  int32_t panEncoderDelta = 0;
  int32_t tiltEncoderDelta = 0;
  uint8_t dimmer = 0;
  uint8_t zoom = 0;
  uint8_t iris = 0;
  uint8_t focus = 0;
  uint8_t color = 0;
};

class ControlInput {
 public:
  bool begin();
  void update(uint32_t nowMs, uint32_t dtMs, ControlState& outState);

 private:
  int32_t _simPanAccumulator = 0;
  int32_t _simTiltAccumulator = 0;
};
