#include "StatusTracker.h"

void StatusTracker::setDmxFramesOutput(uint32_t frames) { _status.dmxFramesOutput = frames; }

void StatusTracker::setUptime(uint32_t nowMs) { _status.uptimeMs = nowMs; }
