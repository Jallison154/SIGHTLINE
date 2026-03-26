#include "NetworkService.h"

namespace sightline_v2 {

bool NetworkService::begin(const NodeConfig& config) {
  (void)config;
  // TODO(HW): Initialize ESP32 Ethernet PHY for selected board.
  // TODO(HW): Apply DHCP/static IP from config and wait for link state.
  _ready = true;
  return true;
}

void NetworkService::tick(uint32_t nowMs) { (void)nowMs; }

String NetworkService::localIpString() const {
  // TODO(HW): Return actual Ethernet local IP string.
  return "0.0.0.0";
}

}  // namespace sightline_v2
