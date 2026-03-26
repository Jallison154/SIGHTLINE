#pragma once

#include <Arduino.h>

struct NodeStatus {
  bool ethernetReady = false;
  bool webUiReady = false;
  bool configLoaded = false;
  bool artNetHasSignal = false;
  uint16_t artNetUniverse = 0;
  uint32_t lastArtNetRxMs = 0;
  uint32_t artNetLastFrameIntervalMs = 0;
  uint32_t artNetPacketsSeen = 0;
  uint32_t artNetPacketsAccepted = 0;
  uint32_t artNetPacketsIgnoredUniverse = 0;
  uint32_t artNetPacketsBad = 0;
  uint32_t dmxFramesOutput = 0;
  uint32_t uptimeMs = 0;
};

class StatusTracker {
 public:
  void markEthernetReady(bool ready) { _status.ethernetReady = ready; }
  void markWebUiReady(bool ready) { _status.webUiReady = ready; }
  void markConfigLoaded(bool loaded) { _status.configLoaded = loaded; }
  void setArtNetSignal(bool hasSignal) { _status.artNetHasSignal = hasSignal; }
  void setArtNetUniverse(uint16_t universe) { _status.artNetUniverse = universe; }
  void setArtNetLastFrameIntervalMs(uint32_t ms) { _status.artNetLastFrameIntervalMs = ms; }
  void setArtNetPacketsSeen(uint32_t count) { _status.artNetPacketsSeen = count; }
  void setArtNetPacketsAccepted(uint32_t count) { _status.artNetPacketsAccepted = count; }
  void setArtNetPacketsIgnoredUniverse(uint32_t count) { _status.artNetPacketsIgnoredUniverse = count; }
  void setArtNetPacketsBad(uint32_t count) { _status.artNetPacketsBad = count; }
  void setLastArtNetRxMs(uint32_t ms) { _status.lastArtNetRxMs = ms; }
  void setDmxFramesOutput(uint32_t frames);
  void setUptime(uint32_t nowMs);
  const NodeStatus& current() const { return _status; }

 private:
  NodeStatus _status;
};
