#pragma once

#include <Arduino.h>

namespace sightline_v2 {

struct ControlState {
  float panNorm = 0.5f;
  float tiltNorm = 0.5f;
  uint8_t intensity = 0;
  uint8_t iris = 0;
  uint8_t zoom = 0;
  uint8_t focus = 0;
  uint8_t shutter = 0;
  uint8_t color = 0;
};

}  // namespace sightline_v2
