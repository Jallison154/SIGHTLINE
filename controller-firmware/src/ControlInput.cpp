#include "ControlInput.h"

bool ControlInput::begin() {
  // TODO: Configure pins and encoder interfaces.
  return true;
}

void ControlInput::update(uint32_t nowMs, ControlState& outState) {
  (void)nowMs;
  // TODO: Read controls without blocking.
  outState = ControlState{};
}
