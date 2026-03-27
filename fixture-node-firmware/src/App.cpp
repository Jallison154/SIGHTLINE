#include "App.h"

void App::begin() {
  Serial.println("[app] begin");
  _dmxBuffer.clear();
  _status.setUptime(0);

  _configStore.begin();
  NodeConfig persistedConfig;
  bool usedDefaults = false;
  const bool configOk = _configStore.loadPersisted(persistedConfig, usedDefaults);
  _configStore.applyToRuntime(persistedConfig, _config);
  _status.markConfigLoaded(configOk || usedDefaults);
  const bool firstBootSetupMode = (!configOk && usedDefaults);
  Serial.printf("[cfg] loaded=%s defaults=%s firstBootSetup=%s nodeName=%s\n", configOk ? "true" : "false",
                usedDefaults ? "true" : "false", firstBootSetupMode ? "true" : "false", _config.nodeName.c_str());
  Serial.printf("[cfg] networkMode=%s dhcp=%s staticIp=%s\n",
                _config.networkMode == NodeConfig::NetworkMode::WiFiStation ? "wifi-station" : "ethernet",
                _config.dhcp ? "true" : "false", _config.staticIp.c_str());
  _network.begin(_config, firstBootSetupMode);
  _status.markSetupMode(firstBootSetupMode);

  if (!_artNetReceiver.begin(_config.universe)) {
    Serial.println("ArtNetReceiver init failed");
  } else {
    Serial.printf("[artnet] listening universe=%u\n", _config.universe);
  }
  DmxOutputConfig dmxConfig;
  // TODO(HW): Confirm UART index + TX + DE/RE pins for final fixture-node board.
  dmxConfig.uartIndex = 2;
  dmxConfig.txPin = 17;
  dmxConfig.directionPin = -1;
  dmxConfig.framePeriodUs = 25000;  // ~40Hz
  if (!_dmxOutput.begin(dmxConfig)) {
    Serial.println("DmxOutput init failed");
  } else {
    Serial.printf("[dmx] uart=%u txPin=%d framePeriodUs=%lu\n", dmxConfig.uartIndex, dmxConfig.txPin, dmxConfig.framePeriodUs);
  }
  const bool webOk = _webUi.begin(_config, _configStore, _status, _network);
  _status.markWebUiReady(webOk);
  Serial.printf("[web] server=%s\n", webOk ? "ready" : "failed");
}

void App::tick(uint32_t nowMs) {
  _status.setUptime(nowMs);
  _network.tick(nowMs, _config);
  const NetworkState& net = _network.state();
  _status.markEthernetReady(net.mode == "ethernet" && net.connected);
  _status.markNetworkConnected(net.connected);
  _status.markSetupMode(net.setupMode);
  _status.setNetworkMode(net.mode);
  _status.setNetworkState(net.statusText);
  _status.setNetworkIp(net.ip);
  _status.setNetworkSsid(net.ssid);

  serviceArtNet(nowMs);
  _dmxOutput.tick(nowMs, _dmxBuffer);
  _status.setDmxFramesOutput(_dmxOutput.framesOutput());
  _webUi.tick();
}

void App::serviceArtNet(uint32_t nowMs) {
  (void)_artNetReceiver.poll(nowMs, _dmxBuffer);

  const ArtNetSignalStatus signal = _artNetReceiver.signalStatus(nowMs);
  _status.setArtNetSignal(signal.hasSignal);
  _status.setArtNetUniverse(signal.listenUniverse);
  _status.setLastArtNetRxMs(signal.lastAcceptedMs);
  _status.setArtNetLastFrameIntervalMs(signal.lastFrameIntervalMs);
  _status.setArtNetPacketsSeen(signal.packetsSeen);
  _status.setArtNetPacketsAccepted(signal.packetsAccepted);
  _status.setArtNetPacketsIgnoredUniverse(signal.packetsIgnoredUniverse);
  _status.setArtNetPacketsBad(signal.packetsBad);
}
