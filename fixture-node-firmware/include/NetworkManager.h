#pragma once

#include <Arduino.h>

#include "ConfigStore.h"

struct NetworkState {
  bool connected = false;
  bool setupMode = false;
  String mode = "ethernet";
  String statusText = "booting";
  String ip = "";
  String ssid = "";
};

class NetworkManager {
 public:
  void begin(const NodeConfig& config, bool forceSetupMode);
  void reconfigure(const NodeConfig& config, bool forceSetupMode);
  void tick(uint32_t nowMs, const NodeConfig& config);

  bool isConnected() const { return _state.connected; }
  bool isSetupMode() const { return _state.setupMode; }
  const NetworkState& state() const { return _state; }

 private:
  enum class ActiveMode : uint8_t {
    None = 0,
    Ethernet,
    WiFiStation,
    SetupAp,
  };

  void startMode(const NodeConfig& config, ActiveMode mode, uint32_t nowMs);
  void startEthernet(const NodeConfig& config);
  void startWiFiStation(const NodeConfig& config);
  void startSetupAp(const NodeConfig& config);
  void updateStateStrings();

  ActiveMode _activeMode = ActiveMode::None;
  uint32_t _modeStartMs = 0;
  NetworkState _state;
  bool _lastConnected = false;
  String _lastIp = "";
};
