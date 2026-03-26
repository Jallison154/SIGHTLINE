#include "DiscoveryCodec.h"

#include <ArduinoJson.h>

namespace sightline_v2 {

namespace {
const char* typeToStr(DiscoveryDeviceType t) { return (t == DiscoveryDeviceType::kController) ? "controller" : "fixture_node"; }
DiscoveryDeviceType strToType(const String& s) {
  return (s == "controller") ? DiscoveryDeviceType::kController : DiscoveryDeviceType::kFixtureNode;
}
const char* healthToStr(DiscoveryHealth h) {
  if (h == DiscoveryHealth::kDegraded) return "degraded";
  if (h == DiscoveryHealth::kError) return "error";
  return "ok";
}
DiscoveryHealth strToHealth(const String& s) {
  if (s == "degraded") return DiscoveryHealth::kDegraded;
  if (s == "error") return DiscoveryHealth::kError;
  return DiscoveryHealth::kOk;
}
const char* claimToStr(ClaimState c) { return (c == ClaimState::kClaimed) ? "claimed" : "available"; }
ClaimState strToClaim(const String& s) { return (s == "claimed") ? ClaimState::kClaimed : ClaimState::kAvailable; }
}  // namespace

bool DiscoveryCodec::encode(const DiscoveryMessageV1& m, String& outJson) {
  StaticJsonDocument<768> doc;
  doc["schema_version"] = m.schemaVersion;
  doc["msg_type"] = m.msgType;
  doc["device_id"] = m.deviceId;
  doc["device_type"] = typeToStr(m.deviceType);
  doc["friendly_name"] = m.friendlyName;
  doc["ip"] = m.ip;
  doc["fw_version"] = m.firmwareVersion;
  doc["status"] = healthToStr(m.health);
  doc["uptime_ms"] = m.uptimeMs;
  doc["sent_at_ms"] = m.sentAtMs;
  doc["fixture_profile_name"] = m.fixtureProfileName;
  doc["claim_state"] = claimToStr(m.claimState);
  doc["assigned_controller_id"] = m.assignedControllerId;
  doc["target_node_id"] = m.targetNodeId;
  outJson.clear();
  serializeJson(doc, outJson);
  return !outJson.isEmpty();
}

bool DiscoveryCodec::decode(const String& json, DiscoveryMessageV1& outMessage, String& outError) {
  StaticJsonDocument<768> doc;
  const DeserializationError err = deserializeJson(doc, json);
  if (err) {
    outError = String("decode failed: ") + err.c_str();
    return false;
  }

  outMessage.schemaVersion = doc["schema_version"] | 0;
  outMessage.msgType = doc["msg_type"] | "";
  outMessage.deviceId = doc["device_id"] | "";
  outMessage.deviceType = strToType(String(doc["device_type"] | ""));
  outMessage.friendlyName = doc["friendly_name"] | "";
  outMessage.ip = doc["ip"] | "";
  outMessage.firmwareVersion = doc["fw_version"] | "";
  outMessage.health = strToHealth(String(doc["status"] | "ok"));
  outMessage.uptimeMs = doc["uptime_ms"] | 0;
  outMessage.sentAtMs = doc["sent_at_ms"] | 0;
  outMessage.fixtureProfileName = doc["fixture_profile_name"] | "";
  outMessage.claimState = strToClaim(String(doc["claim_state"] | "available"));
  outMessage.assignedControllerId = doc["assigned_controller_id"] | "";
  outMessage.targetNodeId = doc["target_node_id"] | "";

  if (outMessage.schemaVersion != 1 || outMessage.msgType != "announce" || outMessage.deviceId.isEmpty()) {
    outError = "invalid discovery envelope";
    return false;
  }
  return true;
}

}  // namespace sightline_v2
