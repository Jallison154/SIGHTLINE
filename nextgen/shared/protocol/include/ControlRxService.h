#pragma once

#include <Arduino.h>

#include "ControlCodec.h"
#include "ControlProtocol.h"

namespace sightline_v2 {

class ControlRxService {
 public:
  bool begin(uint16_t controlPort, const String& nodeId);
  bool poll(uint32_t nowMs, const String& ownerControllerId, bool nodeClaimed);
  bool hasLatestFrame() const { return _hasFrame; }
  const ControlFrameV1& latestFrame() const { return _latest; }
  const ControlStreamStatus& status() const { return _status; }

 private:
  bool acceptFrame(const ControlFrameV1& frame, const String& ownerControllerId, bool nodeClaimed, uint32_t nowMs);

  ControlCodec _codec;
  String _nodeId;
  uint16_t _port = 5570;
  bool _hasFrame = false;
  ControlFrameV1 _latest;
  ControlStreamStatus _status;
};

}  // namespace sightline_v2
