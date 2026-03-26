#include "PanTiltEngine.h"

#include <algorithm>

void PanTiltEngine::configure(float deadband, float smoothing, float panScale, float tiltScale) {
  _deadband = deadband;
  _smoothing = smoothing;
  _panScale = panScale;
  _tiltScale = tiltScale;
}

void PanTiltEngine::update(float panVelocity, float tiltVelocity, uint32_t dtMs) {
  (void)dtMs;
  (void)panVelocity;
  (void)tiltVelocity;
  // TODO: Implement deadband, smoothing, and speed scaling.
}

uint16_t PanTiltEngine::pan16() const { return static_cast<uint16_t>(std::clamp(_pan, 0.0f, 1.0f) * 65535.0f); }

uint16_t PanTiltEngine::tilt16() const { return static_cast<uint16_t>(std::clamp(_tilt, 0.0f, 1.0f) * 65535.0f); }
