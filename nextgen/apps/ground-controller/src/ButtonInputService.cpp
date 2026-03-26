#include "ButtonInputService.h"

namespace sightline_v2 {

bool ButtonInputService::begin() {
  // TODO(HW): Assign digital pins for shutter/color buttons and debounce strategy.
  return true;
}

void ButtonInputService::tick(uint32_t nowMs) {
  (void)nowMs;
  // TODO(HW): Read button state changes and latch output values.
}

}  // namespace sightline_v2
