#include "PanTiltMotionEngine.h"

#include <algorithm>

namespace sightline_v2 {

void PanTiltMotionEngine::configure(float sensitivity, float smoothing, float maxVel) {
  _sensitivity = sensitivity;
  _smoothing = smoothing;
  _maxVel = maxVel;
}

void PanTiltMotionEngine::tick(const EncoderDeltas& d, uint32_t dtMs) {
  if (dtMs == 0) return;
  const float dt = static_cast<float>(dtMs) / 1000.0f;
  const float panTarget = static_cast<float>(d.panDelta) / dt * _sensitivity;
  const float tiltTarget = static_cast<float>(d.tiltDelta) / dt * _sensitivity;

  _panVel = (_smoothing * panTarget) + ((1.0f - _smoothing) * _panVel);
  _tiltVel = (_smoothing * tiltTarget) + ((1.0f - _smoothing) * _tiltVel);

  _panVel = std::clamp(_panVel, -_maxVel, _maxVel);
  _tiltVel = std::clamp(_tiltVel, -_maxVel, _maxVel);

  _pan = std::clamp(_pan + (_panVel * dt), 0.0f, 1.0f);
  _tilt = std::clamp(_tilt + (_tiltVel * dt), 0.0f, 1.0f);
}

}  // namespace sightline_v2
