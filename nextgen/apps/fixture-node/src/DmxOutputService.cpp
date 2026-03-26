#include "DmxOutputService.h"

namespace sightline_v2 {

bool DmxOutputService::begin() {
  // TODO(HW): Initialize UART + RS-485 DMX transmit path on selected board.
  return true;
}

void DmxOutputService::tick(uint32_t nowMs, const uint8_t* universe512) {
  (void)universe512;
  constexpr uint32_t kFramePeriodMs = 25;
  if ((nowMs - _lastTxMs) < kFramePeriodMs) return;
  _lastTxMs = nowMs;
  // TODO(HW): Output full DMX512 frame with proper BREAK/MAB timings.
  _framesOutput++;
}

}  // namespace sightline_v2
