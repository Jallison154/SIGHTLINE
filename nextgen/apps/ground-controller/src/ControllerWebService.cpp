#include "ControllerWebService.h"

#include <ArduinoJson.h>
#include <LittleFS.h>
#include <WebServer.h>

namespace {
WebServer g_webServer(80);

bool sendFileIfExists(const char* path, const char* contentType) {
  if (!LittleFS.exists(path)) return false;
  File f = LittleFS.open(path, "r");
  if (!f) return false;
  g_webServer.streamFile(f, contentType);
  f.close();
  return true;
}
}  // namespace

namespace sightline_v2 {

bool ControllerWebService::begin(ControllerRuntimeState& runtimeState, ControllerStatus& status,
                                 TargetSelector& targetSelector, OwnershipClient& ownershipClient,
                                 const String& controllerId) {
  _runtimeState = &runtimeState;
  _status = &status;
  _targetSelector = &targetSelector;
  _ownershipClient = &ownershipClient;
  _controllerId = controllerId;

  if (!LittleFS.begin(true)) {
    _ready = false;
    return false;
  }

  g_webServer.on("/", HTTP_GET, []() {
    if (!sendFileIfExists("/index.html", "text/html")) {
      g_webServer.send(200, "text/plain", "controller UI missing; upload data partition");
    }
  });
  g_webServer.on("/style.css", HTTP_GET, []() {
    if (!sendFileIfExists("/style.css", "text/css")) g_webServer.send(404, "text/plain", "missing style.css");
  });
  g_webServer.on("/app.js", HTTP_GET, []() {
    if (!sendFileIfExists("/app.js", "application/javascript")) g_webServer.send(404, "text/plain", "missing app.js");
  });

  g_webServer.on("/api/control-state", HTTP_GET, [this]() { handleGetState(); });
  g_webServer.on("/api/control-state", HTTP_POST, [this]() { handlePatchState(); });
  g_webServer.on("/api/targets", HTTP_GET, [this]() { handleGetTargets(); });
  g_webServer.on("/api/targets/select", HTTP_POST, [this]() { handleSelectTarget(); });
  g_webServer.on("/api/ownership", HTTP_POST, [this]() { handleOwnershipAction(); });
  g_webServer.on("/api/action", HTTP_POST, [this]() { handleAction(); });
  g_webServer.on("/api/status", HTTP_GET, [this]() { handleGetStatus(); });

  g_webServer.begin();
  _ready = true;
  return true;
}

void ControllerWebService::tick() { g_webServer.handleClient(); }

void ControllerWebService::handleGetState() {
  StaticJsonDocument<1024> doc;
  const ControlState& c = _runtimeState->controls();
  const RuntimeStatusFields& s = _runtimeState->status();
  const PerControlSourceInfo& src = _runtimeState->perControlSource();

  doc["pan"] = c.panNorm;
  doc["tilt"] = c.tiltNorm;
  doc["pan16"] = static_cast<uint16_t>(c.panNorm * 65535.0f);
  doc["tilt16"] = static_cast<uint16_t>(c.tiltNorm * 65535.0f);
  doc["intensity"] = c.intensity;
  doc["iris"] = c.iris;
  doc["zoom"] = c.zoom;
  doc["focus"] = c.focus;
  doc["shutter"] = c.shutter;
  doc["color"] = c.color;
  doc["activeTargetNodeId"] = _runtimeState->activeTargetNodeId();
  doc["inputMode"] = inputModeToString(_runtimeState->inputMode());
  doc["blackout"] = s.blackout;
  doc["homeRequested"] = s.homeRequested;
  doc["sensitivity"] = s.sensitivity;
  doc["lastUpdateMs"] = s.lastUpdateMs;
  doc["lastUpdateSource"] = s.lastUpdateSource;
  JsonObject per = doc.createNestedObject("perControlSource");
  per["pan"] = src.pan;
  per["tilt"] = src.tilt;
  per["intensity"] = src.intensity;
  per["iris"] = src.iris;
  per["zoom"] = src.zoom;
  per["focus"] = src.focus;
  per["shutter"] = src.shutter;
  per["color"] = src.color;

  String payload;
  serializeJson(doc, payload);
  g_webServer.send(200, "application/json", payload);
}

void ControllerWebService::handlePatchState() {
  StaticJsonDocument<512> req;
  const DeserializationError err = deserializeJson(req, g_webServer.arg("plain"));
  if (err) {
    g_webServer.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid-json\"}");
    return;
  }

  const uint32_t nowMs = millis();
  ControlPatch patch;
  if (req.containsKey("pan")) {
    patch.hasPan = true;
    patch.panNorm = req["pan"];
  }
  if (req.containsKey("tilt")) {
    patch.hasTilt = true;
    patch.tiltNorm = req["tilt"];
  }
  if (req.containsKey("intensity")) {
    patch.hasIntensity = true;
    patch.intensity = req["intensity"];
  }
  if (req.containsKey("iris")) {
    patch.hasIris = true;
    patch.iris = req["iris"];
  }
  if (req.containsKey("zoom")) {
    patch.hasZoom = true;
    patch.zoom = req["zoom"];
  }
  if (req.containsKey("focus")) {
    patch.hasFocus = true;
    patch.focus = req["focus"];
  }
  if (req.containsKey("shutter")) {
    patch.hasShutter = true;
    patch.shutter = req["shutter"];
  }
  if (req.containsKey("color")) {
    patch.hasColor = true;
    patch.color = req["color"];
  }
  if (req.containsKey("inputMode")) {
    _runtimeState->setInputMode(inputModeFromString(static_cast<const char*>(req["inputMode"])), nowMs);
  }
  if (req.containsKey("sensitivity")) {
    _runtimeState->setSensitivity(req["sensitivity"], nowMs);
  }
  if (req.containsKey("blackout")) {
    _runtimeState->setBlackout(req["blackout"], nowMs);
  }

  const bool accepted = _runtimeState->applyWebPatch(patch, nowMs);
  g_webServer.send(200, "application/json", accepted ? "{\"ok\":true}" : "{\"ok\":false,\"error\":\"monitor-only\"}");
}

void ControllerWebService::handleGetTargets() {
  StaticJsonDocument<2048> doc;
  doc["activeTargetNodeId"] = _runtimeState->activeTargetNodeId();
  JsonArray arr = doc.createNestedArray("targets");
  for (size_t i = 0; i < _targetSelector->candidateCount(); ++i) {
    TargetInfo t;
    if (!_targetSelector->getCandidate(i, t)) continue;
    JsonObject o = arr.createNestedObject();
    o["nodeId"] = t.nodeId;
    o["friendlyName"] = t.friendlyName;
    o["ip"] = t.ip;
    o["status"] = targetStatusToString(t.status);
  }
  String payload;
  serializeJson(doc, payload);
  g_webServer.send(200, "application/json", payload);
}

void ControllerWebService::handleSelectTarget() {
  StaticJsonDocument<256> req;
  const DeserializationError err = deserializeJson(req, g_webServer.arg("plain"));
  if (err) {
    g_webServer.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid-json\"}");
    return;
  }
  const char* nodeId = req["nodeId"] | "";
  const bool ok = _targetSelector->selectByNodeId(nodeId);
  if (ok) {
    _runtimeState->setActiveTargetNodeId(nodeId, "web-target", millis());
  }
  g_webServer.send(200, "application/json", ok ? "{\"ok\":true}" : "{\"ok\":false,\"error\":\"not-found\"}");
}

void ControllerWebService::handleOwnershipAction() {
  StaticJsonDocument<256> req;
  const DeserializationError err = deserializeJson(req, g_webServer.arg("plain"));
  if (err) {
    g_webServer.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid-json\"}");
    return;
  }
  const String action = req["action"] | "";
  if (action == "claim") {
    _runtimeState->setClaimRequested(true, millis());
    _runtimeState->setReleaseRequested(false, millis());
    g_webServer.send(200, "application/json", "{\"ok\":true,\"note\":\"claim-requested\"}");
  } else if (action == "release") {
    _runtimeState->setReleaseRequested(true, millis());
    _runtimeState->setClaimRequested(false, millis());
    g_webServer.send(200, "application/json", "{\"ok\":true,\"note\":\"release-requested\"}");
  } else {
    g_webServer.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid-action\"}");
  }
}

void ControllerWebService::handleAction() {
  StaticJsonDocument<256> req;
  const DeserializationError err = deserializeJson(req, g_webServer.arg("plain"));
  if (err) {
    g_webServer.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid-json\"}");
    return;
  }
  const String action = req["action"] | "";
  if (action == "blackout_on") {
    _runtimeState->setBlackout(true, millis());
    g_webServer.send(200, "application/json", "{\"ok\":true}");
  } else if (action == "blackout_off") {
    _runtimeState->setBlackout(false, millis());
    g_webServer.send(200, "application/json", "{\"ok\":true}");
  } else if (action == "home") {
    _runtimeState->requestHome(millis());
    g_webServer.send(200, "application/json", "{\"ok\":true,\"note\":\"home-placeholder\"}");
  } else {
    g_webServer.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid-action\"}");
  }
}

void ControllerWebService::handleGetStatus() {
  StaticJsonDocument<768> doc;
  const RuntimeStatusFields& rs = _runtimeState->status();
  doc["networkReady"] = _status->networkReady;
  doc["discoveryReady"] = _status->discoveryReady;
  doc["ownershipReady"] = _status->ownershipReady;
  doc["controlTxReady"] = _status->controlTxReady;
  doc["webReady"] = _status->webReady;
  doc["targetNodeId"] = _status->targetNodeId;
  doc["ownsTarget"] = _status->ownsTarget;
  doc["controlFramesSent"] = _status->controlFramesSent;
  doc["uptimeMs"] = _status->uptimeMs;
  doc["controllerId"] = _controllerId;
  doc["inputMode"] = inputModeToString(_runtimeState->inputMode());
  doc["blackout"] = rs.blackout;
  doc["lastUpdateSource"] = rs.lastUpdateSource;
  String payload;
  serializeJson(doc, payload);
  g_webServer.send(200, "application/json", payload);
}

const char* ControllerWebService::inputModeToString(InputMode mode) {
  switch (mode) {
    case InputMode::kWebOverride:
      return "web_override";
    case InputMode::kMonitorOnly:
      return "monitor_only";
    case InputMode::kLiveMixed:
    default:
      return "live_mixed";
  }
}

InputMode ControllerWebService::inputModeFromString(const String& mode) {
  if (mode == "web_override") return InputMode::kWebOverride;
  if (mode == "monitor_only") return InputMode::kMonitorOnly;
  return InputMode::kLiveMixed;
}

const char* ControllerWebService::targetStatusToString(TargetStatus status) {
  switch (status) {
    case TargetStatus::kAvailable:
      return "available";
    case TargetStatus::kClaimedByUs:
      return "claimed_by_us";
    case TargetStatus::kClaimedByOther:
      return "claimed_by_other";
    case TargetStatus::kOffline:
      return "offline";
    case TargetStatus::kUnknown:
    default:
      return "unknown";
  }
}

}  // namespace sightline_v2
