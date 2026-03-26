#pragma once

#include <Arduino.h>

#include "ControlProtocol.h"

namespace sightline_v2 {

class ControlCodec {
 public:
  bool encode(const ControlFrameV1& frame, String& outJson);
  bool decode(const String& json, ControlFrameV1& outFrame, String& outError);
};

}  // namespace sightline_v2
