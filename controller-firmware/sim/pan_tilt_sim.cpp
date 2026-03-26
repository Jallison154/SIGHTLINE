#include <cmath>
#include <cstdio>

#include "../include/PanTiltEngine.h"

int main() {
  PanTiltEngine engine;
  PanTiltTuning tuning;
  tuning.sensitivity = 0.0012f;
  tuning.accelerationThresholdCountsPerSec = 90.0f;
  tuning.accelerationGain = 2.0f;
  tuning.accelerationCurve = 1.3f;
  tuning.velocitySmoothing = 0.25f;
  tuning.maxVelocityNormalizedPerSec = 2.4f;

  engine.configure(0.02f, 1.0f, 1.0f, tuning);

  const uint32_t dtMs = 20;
  for (int i = 0; i < 200; ++i) {
    int32_t panDelta = 1;
    int32_t tiltDelta = 1;

    // Simulate fast hand-turn bursts every few cycles.
    if ((i % 50) > 35) {
      panDelta = 5;
      tiltDelta = 3;
    }
    if ((i % 80) > 70) {
      panDelta = -4;
      tiltDelta = -2;
    }

    engine.updateFromEncoderDelta(panDelta, tiltDelta, dtMs);

    if ((i % 10) == 0) {
      std::printf("step=%3d pan=%.4f tilt=%.4f pan16=%5u tilt16=%5u vPan=%.3f vTilt=%.3f\n", i,
                  engine.panNormalized(), engine.tiltNormalized(), engine.pan16(), engine.tilt16(),
                  engine.panVelocity(), engine.tiltVelocity());
    }
  }

  return 0;
}
