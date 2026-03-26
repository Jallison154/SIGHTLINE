#pragma once

#include <Arduino.h>

#include "EncoderInputService.h"

namespace sightline_v2 {

class PanTiltMotionEngine {
 public:
  void configure(float sensitivity, float smoothing, float maxVel);
  void tick(const EncoderDeltas& deltas, uint32_t dtMs);
  float panNorm() const { return _pan; }
  float tiltNorm() const { return _tilt; }

 private:
  float _sensitivity = 0.001f;
  float _smoothing = 0.25f;
  float _maxVel = 2.0f;
  float _pan = 0.5f;
  float _tilt = 0.5f;
  float _panVel = 0.0f;
  float _tiltVel = 0.0f;
};

}  // namespace sightline_v2
