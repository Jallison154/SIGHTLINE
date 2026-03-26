#include "App.h"

void App::begin() {
  // TODO: Initialize Ethernet, controls, Art-Net transmitter, and fixture profile loader.
}

void App::tick(uint32_t nowMs) {
  (void)nowMs;
  // TODO: Non-blocking scheduler:
  // 1) Read controls
  // 2) Process pan/tilt shaping
  // 3) Build DMX512 buffer
  // 4) Transmit Art-Net at fixed rate
}
