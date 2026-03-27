#include "NetworkManager.h"

#include <ETH.h>
#include <WiFi.h>

namespace {
constexpr uint32_t kConnectTimeoutMs = 20000;
}

void NetworkManager::begin(const NodeConfig& config, bool forceSetupMode) {
  const uint32_t nowMs = millis();
  Serial.printf("[net] begin forceSetupMode=%s requestedMode=%s\n", forceSetupMode ? "true" : "false",
                config.networkMode == NodeConfig::NetworkMode::WiFiStation ? "wifi-station" : "ethernet");
  if (forceSetupMode) {
    startMode(config, ActiveMode::SetupAp, nowMs);
    return;
  }

  if (config.networkMode == NodeConfig::NetworkMode::WiFiStation) {
    startMode(config, ActiveMode::WiFiStation, nowMs);
  } else {
    startMode(config, ActiveMode::Ethernet, nowMs);
  }
}

void NetworkManager::tick(uint32_t nowMs, const NodeConfig& config) {
  if (_activeMode == ActiveMode::WiFiStation) {
    wl_status_t st = WiFi.status();
    if (st == WL_CONNECTED) {
      _state.connected = true;
      _state.statusText = "wifi-connected";
      _state.ip = WiFi.localIP().toString();
      _state.ssid = WiFi.SSID();
    } else {
      _state.connected = false;
      _state.statusText = "wifi-connecting";
      _state.ip = "";
      if (config.fallbackToSetupAp && (nowMs - _modeStartMs) > kConnectTimeoutMs) {
        startMode(config, ActiveMode::SetupAp, nowMs);
      }
    }
  } else if (_activeMode == ActiveMode::Ethernet) {
    // TODO(HW): Confirm ETH link and IP behavior on final board/PHY combination.
    _state.connected = ETH.linkUp();
    _state.statusText = _state.connected ? "ethernet-connected" : "ethernet-link-down";
    _state.ip = _state.connected ? ETH.localIP().toString() : "";
    _state.ssid = "";
  } else if (_activeMode == ActiveMode::SetupAp) {
    _state.connected = true;
    _state.statusText = "setup-ap";
    _state.ip = WiFi.softAPIP().toString();
    _state.ssid = WiFi.softAPSSID();
  }

  if (_state.connected != _lastConnected || _state.ip != _lastIp) {
    Serial.printf("[net] mode=%s connected=%s ip=%s state=%s\n", _state.mode.c_str(),
                  _state.connected ? "true" : "false", _state.ip.length() ? _state.ip.c_str() : "-",
                  _state.statusText.c_str());
    _lastConnected = _state.connected;
    _lastIp = _state.ip;
  }

  updateStateStrings();
}

void NetworkManager::startMode(const NodeConfig& config, ActiveMode mode, uint32_t nowMs) {
  _modeStartMs = nowMs;
  _activeMode = mode;
  _state.setupMode = (mode == ActiveMode::SetupAp);
  _state.connected = false;
  _state.ip = "";
  _state.ssid = "";

  if (mode == ActiveMode::Ethernet) {
    startEthernet(config);
  } else if (mode == ActiveMode::WiFiStation) {
    startWiFiStation(config);
  } else if (mode == ActiveMode::SetupAp) {
    startSetupAp(config);
  }
  updateStateStrings();
}

void NetworkManager::startEthernet(const NodeConfig& config) {
  (void)config;
  WiFi.mode(WIFI_OFF);
  // TODO(HW): Replace default ETH.begin() with board-specific PHY config.
  Serial.println("[net] starting Ethernet");
  ETH.begin();
  if (!config.dhcp) {
    IPAddress ip;
    IPAddress gw;
    IPAddress subnet;
    if (ip.fromString(config.staticIp) && gw.fromString(config.gateway) && subnet.fromString(config.subnetMask)) {
      ETH.config(ip, gw, subnet);
    }
  }
}

void NetworkManager::startWiFiStation(const NodeConfig& config) {
  Serial.printf("[net] starting Wi-Fi station ssid=%s\n", config.wifiSsid.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true, true);
  if (!config.dhcp) {
    IPAddress ip;
    IPAddress gw;
    IPAddress subnet;
    if (ip.fromString(config.staticIp) && gw.fromString(config.gateway) && subnet.fromString(config.subnetMask)) {
      WiFi.config(ip, gw, subnet);
    }
  }
  WiFi.begin(config.wifiSsid.c_str(), config.wifiPassword.c_str());
}

void NetworkManager::startSetupAp(const NodeConfig& config) {
  WiFi.mode(WIFI_AP);
  const char* pass = config.setupApPassword.length() >= 8 ? config.setupApPassword.c_str() : nullptr;
  WiFi.softAP(config.setupApSsid.c_str(), pass);
  Serial.printf("[net] setup AP active ssid=%s ip=%s\n", config.setupApSsid.c_str(), WiFi.softAPIP().toString().c_str());
}

void NetworkManager::updateStateStrings() {
  if (_activeMode == ActiveMode::Ethernet) {
    _state.mode = "ethernet";
  } else if (_activeMode == ActiveMode::WiFiStation) {
    _state.mode = "wifi-station";
  } else if (_activeMode == ActiveMode::SetupAp) {
    _state.mode = "setup-ap";
  } else {
    _state.mode = "none";
  }
}
