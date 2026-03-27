#include "WebUiServer.h"

#include <ArduinoJson.h>
#include <LittleFS.h>
#include <WebServer.h>

// Static UI assets live under fixture-node-firmware/data/ and are baked into the LittleFS
// image at build/upload time. After changing files in data/, upload the filesystem:
//   pio run -e esp32_eth_dev -t uploadfs
// Use lowercase paths only: LittleFS is case-sensitive (matches data/ filenames).

namespace {
WebServer g_server(80);

bool sendFileIfExists(const char* path, const char* contentType) {
  if (!LittleFS.exists(path)) {
    return false;
  }
  File f = LittleFS.open(path, "r");
  if (!f) {
    return false;
  }
  g_server.streamFile(f, contentType);
  f.close();
  return true;
}

void sendStaticFallbackPage() {
  g_server.send(200, "text/html",
                "<!doctype html><html><body><h1>SIGHTLINE Fixture Node</h1>"
                "<p>UI assets missing. Upload LittleFS data partition (uploadfs).</p></body></html>");
}

bool parseAndSaveConfigPayload(NodeConfig& liveConfig, ConfigStore& store, bool applyNow) {
  StaticJsonDocument<768> doc;
  const String body = g_server.arg("plain");
  const DeserializationError err = deserializeJson(doc, body);
  if (err) {
    g_server.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid-json\"}");
    return false;
  }

  NodeConfig next = liveConfig;
  if (doc.containsKey("nodeName")) next.nodeName = static_cast<const char*>(doc["nodeName"]);
  if (doc.containsKey("fixtureLabel")) next.fixtureLabel = static_cast<const char*>(doc["fixtureLabel"]);
  if (doc.containsKey("networkMode")) {
    const String m = static_cast<const char*>(doc["networkMode"]);
    next.networkMode = m == "wifi-station" ? NodeConfig::NetworkMode::WiFiStation : NodeConfig::NetworkMode::Ethernet;
  }
  if (doc.containsKey("wifiSsid")) next.wifiSsid = static_cast<const char*>(doc["wifiSsid"]);
  if (doc.containsKey("wifiPassword")) next.wifiPassword = static_cast<const char*>(doc["wifiPassword"]);
  if (doc.containsKey("fallbackToSetupAp")) next.fallbackToSetupAp = doc["fallbackToSetupAp"];
  if (doc.containsKey("setupApSsid")) next.setupApSsid = static_cast<const char*>(doc["setupApSsid"]);
  if (doc.containsKey("setupApPassword")) next.setupApPassword = static_cast<const char*>(doc["setupApPassword"]);
  if (doc.containsKey("universe")) next.universe = doc["universe"];
  if (doc.containsKey("dmxStartAddress")) next.dmxStartAddress = doc["dmxStartAddress"];
  if (doc.containsKey("dhcp")) next.dhcp = doc["dhcp"];
  if (doc.containsKey("staticIp")) next.staticIp = static_cast<const char*>(doc["staticIp"]);
  if (doc.containsKey("subnetMask")) next.subnetMask = static_cast<const char*>(doc["subnetMask"]);
  if (doc.containsKey("gateway")) next.gateway = static_cast<const char*>(doc["gateway"]);

  if (!store.savePersisted(next)) {
    g_server.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid-config\"}");
    return false;
  }
  if (applyNow) {
    store.applyToRuntime(next, liveConfig);
  }
  return true;
}
}

bool WebUiServer::begin(NodeConfig& liveConfig, ConfigStore& configStore, const StatusTracker& status,
                        const NetworkManager& network) {
  _config = &liveConfig;
  _configStore = &configStore;
  _status = &status;
  _network = &network;
  const bool fsOk = LittleFS.begin(true);
  if (fsOk) {
    Serial.println("[fs] LittleFS mount: ok");
  } else {
    Serial.println("[fs] LittleFS mount: failed");
  }
  if (!fsOk) {
    return false;
  }

  g_server.on("/", HTTP_GET, []() {
    if (!sendFileIfExists("/index.html", "text/html")) {
      sendStaticFallbackPage();
    }
  });
  g_server.on("/style.css", HTTP_GET, []() {
    if (!sendFileIfExists("/style.css", "text/css")) {
      g_server.send(404, "text/plain", "missing style.css");
    }
  });
  g_server.on("/app.js", HTTP_GET, []() {
    if (!sendFileIfExists("/app.js", "application/javascript")) {
      g_server.send(404, "text/plain", "missing app.js");
    }
  });
  g_server.on("/sightline_logo.png", HTTP_GET, []() {
    if (!sendFileIfExists("/sightline_logo.png", "image/png")) {
      g_server.send(404, "text/plain", "missing sightline_logo.png");
    }
  });

  g_server.on("/api/status", HTTP_GET, [this]() {
    StaticJsonDocument<768> doc;
    const NodeStatus& s = _status->current();
    doc["ethernetReady"] = s.ethernetReady;
    doc["networkConnected"] = s.networkConnected;
    doc["setupMode"] = s.setupMode;
    doc["networkMode"] = s.networkMode;
    doc["networkState"] = s.networkState;
    doc["networkIp"] = s.networkIp;
    doc["networkSsid"] = s.networkSsid;
    doc["webUiReady"] = s.webUiReady;
    doc["configLoaded"] = s.configLoaded;
    doc["artNetHasSignal"] = s.artNetHasSignal;
    doc["artNetUniverse"] = s.artNetUniverse;
    doc["lastArtNetRxMs"] = s.lastArtNetRxMs;
    doc["artNetLastFrameIntervalMs"] = s.artNetLastFrameIntervalMs;
    doc["artNetPacketsSeen"] = s.artNetPacketsSeen;
    doc["artNetPacketsAccepted"] = s.artNetPacketsAccepted;
    doc["artNetPacketsIgnoredUniverse"] = s.artNetPacketsIgnoredUniverse;
    doc["artNetPacketsBad"] = s.artNetPacketsBad;
    doc["dmxFramesOutput"] = s.dmxFramesOutput;
    doc["uptimeMs"] = s.uptimeMs;

    String payload;
    serializeJson(doc, payload);
    g_server.send(200, "application/json", payload);
  });

  g_server.on("/api/config", HTTP_GET, [this]() {
    StaticJsonDocument<1024> doc;
    doc["nodeName"] = _config->nodeName;
    doc["fixtureLabel"] = _config->fixtureLabel;
    doc["networkMode"] = _config->networkMode == NodeConfig::NetworkMode::WiFiStation ? "wifi-station" : "ethernet";
    doc["wifiSsid"] = _config->wifiSsid;
    doc["wifiPassword"] = "";
    doc["fallbackToSetupAp"] = _config->fallbackToSetupAp;
    doc["setupApSsid"] = _config->setupApSsid;
    doc["setupApPassword"] = "";
    doc["universe"] = _config->universe;
    doc["dmxStartAddress"] = _config->dmxStartAddress;
    doc["dhcp"] = _config->dhcp;
    doc["staticIp"] = _config->staticIp;
    doc["subnetMask"] = _config->subnetMask;
    doc["gateway"] = _config->gateway;

    String payload;
    serializeJson(doc, payload);
    g_server.send(200, "application/json", payload);
  });

  g_server.on("/api/config", HTTP_POST, [this]() {
    if (!parseAndSaveConfigPayload(*_config, *_configStore, false)) {
      return;
    }
    g_server.send(200, "application/json", "{\"ok\":true,\"applied\":false}");
  });

  g_server.on("/api/config/apply", HTTP_POST, [this]() {
    if (!parseAndSaveConfigPayload(*_config, *_configStore, true)) {
      return;
    }
    // Apply semantics: configuration is persisted and copied to live state.
    // TODO(HW): Reconfigure Ethernet/Wi-Fi services immediately once service ownership is split.
    g_server.send(200, "application/json", "{\"ok\":true,\"applied\":true,\"note\":\"reboot-required\"}");
  });

  g_server.on("/api/setup-state", HTTP_GET, [this]() {
    StaticJsonDocument<192> doc;
    const NetworkState& ns = _network->state();
    doc["setupMode"] = ns.setupMode;
    doc["networkMode"] = ns.mode;
    doc["status"] = ns.statusText;
    doc["ip"] = ns.ip;
    doc["ssid"] = ns.ssid;
    String payload;
    serializeJson(doc, payload);
    g_server.send(200, "application/json", payload);
  });

  g_server.begin();
  Serial.println("[web] HTTP server listening on port 80");
  return true;
}

void WebUiServer::tick() { g_server.handleClient(); }
