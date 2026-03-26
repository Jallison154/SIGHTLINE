#include "ControllerConfigStore.h"

#include <Preferences.h>

namespace sightline_v2 {

namespace {
constexpr const char* kNs = "sl-v2-ctrl";
}

bool ControllerConfigStore::begin() { return true; }
ControllerConfig ControllerConfigStore::defaults() const { return ControllerConfig{}; }

bool ControllerConfigStore::validate(const ControllerConfig& c, String& outError) const {
  if (c.controllerId.isEmpty()) {
    outError = "controllerId required";
    return false;
  }
  if (c.friendlyName.isEmpty() || c.friendlyName.length() > 32) {
    outError = "friendlyName must be 1..32 chars";
    return false;
  }
  if (c.discoveryPort == 0 || c.controlPort == 0) {
    outError = "ports must be non-zero";
    return false;
  }
  if (c.controlTxPeriodMs < 10 || c.controlTxPeriodMs > 100) {
    outError = "controlTxPeriodMs must be 10..100";
    return false;
  }
  return true;
}

bool ControllerConfigStore::loadPersisted(ControllerConfig& outConfig, bool& outUsedDefaults) {
  outUsedDefaults = false;
  Preferences p;
  if (!p.begin(kNs, true)) {
    outConfig = defaults();
    outUsedDefaults = true;
    return false;
  }
  ControllerConfig c = defaults();
  c.controllerId = p.getString("id", c.controllerId);
  c.friendlyName = p.getString("name", c.friendlyName);
  c.dhcp = p.getBool("dhcp", c.dhcp);
  c.staticIp = p.getString("ip", c.staticIp);
  c.subnet = p.getString("subnet", c.subnet);
  c.gateway = p.getString("gw", c.gateway);
  c.discoveryPort = p.getUShort("discPort", c.discoveryPort);
  c.controlPort = p.getUShort("ctrlPort", c.controlPort);
  c.controlTxPeriodMs = p.getUInt("txPeriod", c.controlTxPeriodMs);
  p.end();

  String err;
  if (!validate(c, err)) {
    outConfig = defaults();
    outUsedDefaults = true;
    return false;
  }
  outConfig = c;
  return true;
}

bool ControllerConfigStore::savePersisted(const ControllerConfig& config, String& outError) {
  if (!validate(config, outError)) return false;
  Preferences p;
  if (!p.begin(kNs, false)) {
    outError = "nvs open failed";
    return false;
  }
  p.putString("id", config.controllerId);
  p.putString("name", config.friendlyName);
  p.putBool("dhcp", config.dhcp);
  p.putString("ip", config.staticIp);
  p.putString("subnet", config.subnet);
  p.putString("gw", config.gateway);
  p.putUShort("discPort", config.discoveryPort);
  p.putUShort("ctrlPort", config.controlPort);
  p.putUInt("txPeriod", config.controlTxPeriodMs);
  p.end();
  return true;
}

bool ControllerConfigStore::applyToRuntime(const ControllerConfig& persisted, ControllerConfig& runtime,
                                           String& outError) const {
  if (!validate(persisted, outError)) return false;
  runtime = persisted;
  return true;
}

}  // namespace sightline_v2
