#pragma once

#include <Arduino.h>

struct NodeStatus {
  bool ethernetReady = false;
  bool webUiReady = false;
  bool configLoaded = false;
  uint32_t lastArtNetRxMs = 0;
  uint32_t artNetPacketsSeen = 0;
  uint32_t artNetPacketsAccepted = 0;
  uint32_t dmxFramesOutput = 0;
  uint32_t uptimeMs = 0;
};

class StatusTracker {
 public:
  void markEthernetReady(bool ready) { _status.ethernetReady = ready; }
  void markWebUiReady(bool ready) { _status.webUiReady = ready; }
  void markConfigLoaded(bool loaded) { _status.configLoaded = loaded; }
  void onArtNetSeen(uint32_t nowMs);
  void onArtNetAccepted();
  void setDmxFramesOutput(uint32_t frames);
  void setUptime(uint32_t nowMs);
  const NodeStatus& current() const { return _status; }

 private:
  NodeStatus _status;
};
