#include "EncoderInputService.h"

namespace sightline_v2 {

bool EncoderInputService::begin() {
  // TODO(HW): Assign and initialize pan/tilt encoder pins/PCNT units.
  _simulate = true;
  return true;
}

void EncoderInputService::tick(uint32_t nowMs) {
  (void)nowMs;
  if (_simulate) {
    _simPan = (_simPan + 1) % 400;
    _simTilt = (_simTilt + 1) % 600;
    _pending.panDelta = (_simPan < 200) ? 1 : -1;
    _pending.tiltDelta = (_simTilt < 300) ? 1 : -1;
  } else {
    // TODO(HW): Read encoder PCNT deltas without blocking.
  }
}

EncoderDeltas EncoderInputService::readAndResetDeltas() {
  EncoderDeltas out = _pending;
  _pending = EncoderDeltas{};
  return out;
}

}  // namespace sightline_v2
