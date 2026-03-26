#include "App.h"

void App::begin() {
  // TODO(HW): Configure actual Ethernet board/platform pins and PHY settings.
  // TODO(HW): Initialize ETH with board-specific clock mode and PHY address.

  _dmxBuffer.clear();
  _status.setUptime(0);

  _configStore.begin();
  NodeConfig persistedConfig;
  bool usedDefaults = false;
  const bool configOk = _configStore.loadPersisted(persistedConfig, usedDefaults);
  _configStore.applyToRuntime(persistedConfig, _config);
  _status.markConfigLoaded(configOk || usedDefaults);

  // TODO(HW): Set based on real ETH link state.
  _status.markEthernetReady(true);

  if (!_artNetReceiver.begin(_config.universe)) {
    Serial.println("ArtNetReceiver init failed");
  }
  DmxOutputConfig dmxConfig;
  // TODO(HW): Confirm UART index + TX + DE/RE pins for final fixture-node board.
  dmxConfig.uartIndex = 2;
  dmxConfig.txPin = 17;
  dmxConfig.directionPin = -1;
  dmxConfig.framePeriodUs = 25000;  // ~40Hz
  if (!_dmxOutput.begin(dmxConfig)) {
    Serial.println("DmxOutput init failed");
  }
  const bool webOk = _webUi.begin(_config, _configStore, _status);
  _status.markWebUiReady(webOk);
}

void App::tick(uint32_t nowMs) {
  _status.setUptime(nowMs);
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
