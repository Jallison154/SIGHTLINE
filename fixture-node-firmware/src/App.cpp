#include "App.h"

void App::begin() {
  // TODO(HW): Configure actual Ethernet board/platform pins and PHY settings.
  // TODO(HW): Initialize ETH with board-specific clock mode and PHY address.

  _dmxBuffer.clear();
  _status.setUptime(0);

  _configStore.begin();
  const bool configOk = _configStore.load(_config);
  _status.markConfigLoaded(configOk);

  // TODO(HW): Set based on real ETH link state.
  _status.markEthernetReady(true);

  _artNetReceiver.begin(_config.universe);
  _dmxOutput.begin();
  _webUi.begin(_config, _configStore, _status);
  _status.markWebUiReady(true);
}

void App::tick(uint32_t nowMs) {
  _status.setUptime(nowMs);
  serviceArtNet(nowMs);
  _dmxOutput.tick(nowMs, _dmxBuffer);
  _status.setDmxFramesOutput(_dmxOutput.framesOutput());
  _webUi.tick();
}

void App::serviceArtNet(uint32_t nowMs) {
  const bool accepted = _artNetReceiver.poll(_dmxBuffer);
  if (_artNetReceiver.packetsSeen() > _status.current().artNetPacketsSeen) {
    _status.onArtNetSeen(nowMs);
  }
  if (accepted) {
    _status.onArtNetAccepted();
  }
}
