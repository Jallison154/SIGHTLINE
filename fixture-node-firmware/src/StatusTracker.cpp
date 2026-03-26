#include "StatusTracker.h"

void StatusTracker::onArtNetSeen(uint32_t nowMs) {
  _status.artNetPacketsSeen++;
  _status.lastArtNetRxMs = nowMs;
}

void StatusTracker::onArtNetAccepted() { _status.artNetPacketsAccepted++; }

void StatusTracker::setDmxFramesOutput(uint32_t frames) { _status.dmxFramesOutput = frames; }

void StatusTracker::setUptime(uint32_t nowMs) { _status.uptimeMs = nowMs; }
