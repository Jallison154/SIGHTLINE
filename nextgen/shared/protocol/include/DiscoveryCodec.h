#pragma once

#include <Arduino.h>

#include "DiscoveryProtocol.h"

namespace sightline_v2 {

class DiscoveryCodec {
 public:
  bool encode(const DiscoveryMessageV1& message, String& outJson);
  bool decode(const String& json, DiscoveryMessageV1& outMessage, String& outError);
};

}  // namespace sightline_v2
