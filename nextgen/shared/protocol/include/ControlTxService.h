#pragma once

#include <Arduino.h>

#include "ControlCodec.h"

namespace sightline_v2 {

class ControlTxService {
 public:
  bool begin(uint16_t controlPort);
  void setTargetIp(const String& ip);
  void setTargetNodeId(const String& nodeId);
  void setControllerId(const String& controllerId);
  bool sendFrame(const ControlFrameV1& frame);

 private:
  ControlCodec _codec;
  uint16_t _port = 5570;
  String _targetIp = "0.0.0.0";
  String _targetNodeId;
  String _controllerId;
};

}  // namespace sightline_v2
