#include "ControllerNetworkService.h"

namespace sightline_v2 {

bool ControllerNetworkService::begin(const ControllerConfig& config) {
  (void)config;
  // TODO(HW): Initialize ESP32 Ethernet PHY and apply DHCP/static settings.
  _ready = true;
  return true;
}

void ControllerNetworkService::tick(uint32_t nowMs) { (void)nowMs; }

String ControllerNetworkService::localIpString() const {
  // TODO(HW): Return actual local IP string from Ethernet interface.
  return "0.0.0.0";
}

}  // namespace sightline_v2
