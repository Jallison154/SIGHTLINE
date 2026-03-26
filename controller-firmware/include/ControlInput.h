#pragma once

#include <Arduino.h>

struct ControlState {
  float panVelocity = 0.0f;
  float tiltVelocity = 0.0f;
  uint8_t dimmer = 0;
  uint8_t zoom = 0;
  uint8_t iris = 0;
  uint8_t focus = 0;
  uint8_t color = 0;
};

class ControlInput {
 public:
  bool begin();
  void update(uint32_t nowMs, ControlState& outState);
};
