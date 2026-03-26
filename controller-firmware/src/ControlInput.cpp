#include "ControlInput.h"

bool ControlInput::begin() {
  // TODO(HW): Assign and configure pan encoder pins here.
  // TODO(HW): Assign and configure tilt encoder pins here.
  // TODO(HW): Add pull-ups/pull-downs and optional interrupt setup.
  return true;
}

void ControlInput::update(uint32_t nowMs, uint32_t dtMs, ControlState& outState) {
  (void)nowMs;
  (void)dtMs;

  // TODO(HW): Replace simulation with real non-blocking encoder reads.
  // For now this creates variable-speed movement so acceleration tuning can be tested.
  _simPanAccumulator = (_simPanAccumulator + 1) % 400;
  _simTiltAccumulator = (_simTiltAccumulator + 1) % 600;

  // Slow turns around center, faster bursts near phase boundaries.
  outState.panEncoderDelta = (_simPanAccumulator < 200) ? 1 : -1;
  if ((_simPanAccumulator % 80) < 10) {
    outState.panEncoderDelta *= 3;
  }

  outState.tiltEncoderDelta = (_simTiltAccumulator < 300) ? 1 : -1;
  if ((_simTiltAccumulator % 120) < 12) {
    outState.tiltEncoderDelta *= 2;
  }

  // TODO(HW): Wire physical knobs/buttons later.
  outState.dimmer = 255;
  outState.zoom = 0;
  outState.iris = 0;
  outState.focus = 0;
  outState.color = 0;
}
