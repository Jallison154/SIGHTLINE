#pragma once

#include <stdint.h>

struct PanTiltTuning {
  // Sensitivity converts encoder counts/sec into normalized velocity units/sec.
  float sensitivity = 0.0010f;
  // Any speed above this starts adding acceleration gain.
  float accelerationThresholdCountsPerSec = 80.0f;
  // How much extra gain to add at high spin rates.
  float accelerationGain = 2.2f;
  // Curve shape for acceleration (1.0 linear, >1 stronger at high speed).
  float accelerationCurve = 1.35f;
  // Low-pass factor for velocity smoothing (0..1). Higher = snappier.
  float velocitySmoothing = 0.25f;
  // Optional cap so runaway values never overflow position integration.
  float maxVelocityNormalizedPerSec = 2.0f;
};

class PanTiltEngine {
 public:
  void configure(float deadband, float panScale, float tiltScale, const PanTiltTuning& tuning);
  void updateFromEncoderDelta(int32_t panDeltaCounts, int32_t tiltDeltaCounts, uint32_t dtMs);
  uint16_t pan16() const;
  uint16_t tilt16() const;
  float panNormalized() const { return _pan; }
  float tiltNormalized() const { return _tilt; }
  float panVelocity() const { return _panVelocity; }
  float tiltVelocity() const { return _tiltVelocity; }

 private:
  float applyAcceleration(float countsPerSecond) const;
  float clamp01(float value) const;

  // Deadband in counts/sec for encoder jitter suppression.
  float _deadbandCountsPerSec = 1.0f;
  float _panScale = 1.0f;
  float _tiltScale = 1.0f;
  PanTiltTuning _tuning;

  float _pan = 0.5f;
  float _tilt = 0.5f;
  float _panVelocity = 0.0f;
  float _tiltVelocity = 0.0f;
};
