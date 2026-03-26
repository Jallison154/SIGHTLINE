#include "WebUiServer.h"

#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <WebServer.h>

namespace {
WebServer g_server(80);

bool sendFileIfExists(const char* path, const char* contentType) {
  if (!SPIFFS.exists(path)) {
    return false;
  }
  File f = SPIFFS.open(path, "r");
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
                "<p>UI assets missing. Upload SPIFFS data partition.</p></body></html>");
}

bool parseAndSaveConfigPayload(NodeConfig& liveConfig, ConfigStore& store) {
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
  if (doc.containsKey("universe")) next.universe = doc["universe"];
  if (doc.containsKey("dmxStartAddress")) next.dmxStartAddress = doc["dmxStartAddress"];
  if (doc.containsKey("dhcp")) next.dhcp = doc["dhcp"];
  if (doc.containsKey("staticIp")) next.staticIp = static_cast<const char*>(doc["staticIp"]);
  if (doc.containsKey("subnetMask")) next.subnetMask = static_cast<const char*>(doc["subnetMask"]);
  if (doc.containsKey("gateway")) next.gateway = static_cast<const char*>(doc["gateway"]);

  if (!store.save(next)) {
    g_server.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid-config\"}");
    return false;
  }
  liveConfig = next;
  return true;
}
}

bool WebUiServer::begin(NodeConfig& liveConfig, ConfigStore& configStore, const StatusTracker& status) {
  _config = &liveConfig;
  _configStore = &configStore;
  _status = &status;
  SPIFFS.begin(true);

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

  g_server.on("/api/status", HTTP_GET, [this]() {
    StaticJsonDocument<512> doc;
    const NodeStatus& s = _status->current();
    doc["ethernetReady"] = s.ethernetReady;
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
    StaticJsonDocument<768> doc;
    doc["nodeName"] = _config->nodeName;
    doc["fixtureLabel"] = _config->fixtureLabel;
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
    if (!parseAndSaveConfigPayload(*_config, *_configStore)) {
      return;
    }
    g_server.send(200, "application/json", "{\"ok\":true,\"applied\":false}");
  });

  g_server.on("/api/config/apply", HTTP_POST, [this]() {
    if (!parseAndSaveConfigPayload(*_config, *_configStore)) {
      return;
    }
    // Apply semantics: configuration is persisted and copied to live state.
    // TODO(HW): Reconfigure Ethernet and Art-Net receiver immediately if required.
    g_server.send(200, "application/json", "{\"ok\":true,\"applied\":true,\"note\":\"restart-may-be-required\"}");
  });

  g_server.begin();
  return true;
}

void WebUiServer::tick() { g_server.handleClient(); }
