#pragma once

#include <Arduino.h>

class PanTiltEngine {
 public:
  void configure(float deadband, float smoothing, float panScale, float tiltScale);
  void update(float panVelocity, float tiltVelocity, uint32_t dtMs);
  uint16_t pan16() const;
  uint16_t tilt16() const;

 private:
  float _deadband = 0.02f;
  float _smoothing = 0.25f;
  float _panScale = 1.0f;
  float _tiltScale = 1.0f;
  float _pan = 0.5f;
  float _tilt = 0.5f;
};
