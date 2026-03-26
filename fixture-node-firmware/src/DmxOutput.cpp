#include "DmxOutput.h"

bool DmxOutput::begin() {
  // TODO(HW): Select UART/peripheral and configure RS-485 transceiver pins.
  // TODO(HW): Set DMX TX pin, DE/RE direction pin, and break timing for chosen board.
  _lastOutputMs = 0;
  _framesOutput = 0;
  return true;
}

void DmxOutput::tick(uint32_t nowMs, const DmxBuffer& source) {
  constexpr uint32_t kDmxFramePeriodMs = 25;  // ~40 Hz refresh
  if ((nowMs - _lastOutputMs) < kDmxFramePeriodMs) {
    return;
  }
  _lastOutputMs = nowMs;

  const uint8_t* dmx = source.data();
  (void)dmx;
  // TODO(HW): Write full DMX frame including break/MAB and 512 slots.
  _framesOutput++;
}
