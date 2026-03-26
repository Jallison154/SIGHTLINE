#include "PanTiltEngine.h"

#include <algorithm>
#include <cmath>

namespace {
float clampf(float v, float lo, float hi) { return std::max(lo, std::min(v, hi)); }
}  // namespace

void PanTiltEngine::configure(float deadband, float panScale, float tiltScale, const PanTiltTuning& tuning) {
  // deadband profile value is normalized; map to counts/sec range for encoders.
  _deadbandCountsPerSec = std::max(0.0f, deadband * 100.0f);
  _panScale = panScale;
  _tiltScale = tiltScale;
  _tuning = tuning;
}

float PanTiltEngine::applyAcceleration(float countsPerSecond) const {
  const float absSpeed = std::fabs(countsPerSecond);
  const float threshold = std::max(1.0f, _tuning.accelerationThresholdCountsPerSec);
  const float normalized = absSpeed / threshold;
  const float curve = std::pow(normalized, _tuning.accelerationCurve);
  const float accelMultiplier = 1.0f + (_tuning.accelerationGain * curve);
  return accelMultiplier;
}

float PanTiltEngine::clamp01(float value) const { return clampf(value, 0.0f, 1.0f); }

void PanTiltEngine::updateFromEncoderDelta(int32_t panDeltaCounts, int32_t tiltDeltaCounts, uint32_t dtMs) {
  if (dtMs == 0) {
    return;
  }
  const float dtSeconds = static_cast<float>(dtMs) / 1000.0f;
  const float panCountsPerSec = static_cast<float>(panDeltaCounts) / dtSeconds;
  const float tiltCountsPerSec = static_cast<float>(tiltDeltaCounts) / dtSeconds;

  float panTargetVel = 0.0f;
  float tiltTargetVel = 0.0f;

  if (std::fabs(panCountsPerSec) >= _deadbandCountsPerSec) {
    panTargetVel = panCountsPerSec * _tuning.sensitivity * _panScale * applyAcceleration(panCountsPerSec);
  }
  if (std::fabs(tiltCountsPerSec) >= _deadbandCountsPerSec) {
    tiltTargetVel = tiltCountsPerSec * _tuning.sensitivity * _tiltScale * applyAcceleration(tiltCountsPerSec);
  }

  // Smooth velocity to avoid abrupt starts/stops from noisy encoder steps.
  _panVelocity = (_tuning.velocitySmoothing * panTargetVel) + ((1.0f - _tuning.velocitySmoothing) * _panVelocity);
  _tiltVelocity =
      (_tuning.velocitySmoothing * tiltTargetVel) + ((1.0f - _tuning.velocitySmoothing) * _tiltVelocity);

  _panVelocity = clampf(_panVelocity, -_tuning.maxVelocityNormalizedPerSec, _tuning.maxVelocityNormalizedPerSec);
  _tiltVelocity = clampf(_tiltVelocity, -_tuning.maxVelocityNormalizedPerSec, _tuning.maxVelocityNormalizedPerSec);

  _pan = clamp01(_pan + (_panVelocity * dtSeconds));
  _tilt = clamp01(_tilt + (_tiltVelocity * dtSeconds));
}

uint16_t PanTiltEngine::pan16() const { return static_cast<uint16_t>(clampf(_pan, 0.0f, 1.0f) * 65535.0f); }

uint16_t PanTiltEngine::tilt16() const { return static_cast<uint16_t>(clampf(_tilt, 0.0f, 1.0f) * 65535.0f); }
