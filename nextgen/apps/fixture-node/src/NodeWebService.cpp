#include "NodeWebService.h"

namespace sightline_v2 {

bool NodeWebService::begin(NodeConfig& runtimeConfig, NodeConfigStore& configStore, const NodeStatus& status) {
  _runtimeConfig = &runtimeConfig;
  _configStore = &configStore;
  _status = &status;
  // TODO: Bind lightweight web routes using chosen web server. Static assets live in
  // apps/fixture-node/data/ (e.g. index.html, style.css, app.js, sightline_logo.png).
  // After editing data/, upload the filesystem: pio run -e fixture_node_dev -t uploadfs
  // If using AsyncWebServer: server.serveStatic("/", LittleFS, "/"); (same partition as profiles).
  // Logo URL: GET /sightline_logo.png
  //
  //  GET /api/status
  //  -> {
  //       "networkReady": bool,
  //       "discoveryReady": bool,
  //       "controlRxReady": bool,
  //       "webReady": bool,
  //       "dmxReady": bool,
  //       "profileActive": bool,
  //       "claimed": bool,
  //       "assignedControllerId": string,
  //       "activeProfileId": string,
  //       "uptimeMs": uint32,
  //       "controlFramesAccepted": uint32,
  //       "dmxFramesOutput": uint32
  //     }
  //
  //  GET /api/config
  //  -> {
  //       "nodeName": string,
  //       "fixtureLabel": string,
  //       "dhcp": bool,
  //       "staticIp": string,
  //       "subnetMask": string,
  //       "gateway": string
  //     }
  //
  //  POST /api/config
  //  body: { nodeName, fixtureLabel, dhcp, staticIp, subnetMask, gateway }
  //  -> { "ok": true } or { "ok": false, "error": string }
  //
  //  POST /api/config/apply
  //  -> same response shape; runtime apply may require restart for network settings.
  return true;
}

void NodeWebService::tick() {
  // TODO: Pump web server/event loop.
}

}  // namespace sightline_v2
