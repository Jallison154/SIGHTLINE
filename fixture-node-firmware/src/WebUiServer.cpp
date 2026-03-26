#include "WebUiServer.h"

#include <ArduinoJson.h>
#include <WebServer.h>

namespace {
WebServer g_server(80);
}

bool WebUiServer::begin(NodeConfig& liveConfig, ConfigStore& configStore, const StatusTracker& status) {
  _config = &liveConfig;
  _configStore = &configStore;
  _status = &status;

  g_server.on("/", HTTP_GET, []() {
    g_server.send(200, "text/html",
                  "<!doctype html><html><body><h1>SIGHTLINE Fixture Node</h1>"
                  "<p>Use /api/status and /api/config endpoints.</p></body></html>");
  });

  g_server.on("/api/status", HTTP_GET, [this]() {
    StaticJsonDocument<512> doc;
    const NodeStatus& s = _status->current();
    doc["ethernetReady"] = s.ethernetReady;
    doc["webUiReady"] = s.webUiReady;
    doc["configLoaded"] = s.configLoaded;
    doc["lastArtNetRxMs"] = s.lastArtNetRxMs;
    doc["artNetPacketsSeen"] = s.artNetPacketsSeen;
    doc["artNetPacketsAccepted"] = s.artNetPacketsAccepted;
    doc["dmxFramesOutput"] = s.dmxFramesOutput;
    doc["uptimeMs"] = s.uptimeMs;

    String payload;
    serializeJson(doc, payload);
    g_server.send(200, "application/json", payload);
  });

  g_server.on("/api/config", HTTP_GET, [this]() {
    StaticJsonDocument<512> doc;
    doc["nodeName"] = _config->nodeName;
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
    StaticJsonDocument<512> doc;
    const String body = g_server.arg("plain");
    const DeserializationError err = deserializeJson(doc, body);
    if (err) {
      g_server.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid-json\"}");
      return;
    }

    NodeConfig next = *_config;
    if (doc.containsKey("nodeName")) next.nodeName = static_cast<const char*>(doc["nodeName"]);
    if (doc.containsKey("universe")) next.universe = doc["universe"];
    if (doc.containsKey("dmxStartAddress")) next.dmxStartAddress = doc["dmxStartAddress"];
    if (doc.containsKey("dhcp")) next.dhcp = doc["dhcp"];
    if (doc.containsKey("staticIp")) next.staticIp = static_cast<const char*>(doc["staticIp"]);
    if (doc.containsKey("subnetMask")) next.subnetMask = static_cast<const char*>(doc["subnetMask"]);
    if (doc.containsKey("gateway")) next.gateway = static_cast<const char*>(doc["gateway"]);

    if (!_configStore->save(next)) {
      g_server.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid-config\"}");
      return;
    }

    *_config = next;
    g_server.send(200, "application/json", "{\"ok\":true}");
  });

  g_server.begin();
  return true;
}

void WebUiServer::tick() { g_server.handleClient(); }
