#include "FaderInputService.h"

namespace sightline_v2 {

bool FaderInputService::begin() {
  // TODO(HW): Assign ADC pins and scaling for physical faders/knobs.
  return true;
}

void FaderInputService::tick(uint32_t nowMs) {
  (void)nowMs;
  // TODO(HW): Read analog controls and map to 0..255.
  _values.intensity = 255;
  _values.iris = 0;
  _values.zoom = 0;
  _values.focus = 0;
}

}  // namespace sightline_v2
