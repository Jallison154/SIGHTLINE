#pragma once

#include <Arduino.h>

#include "ControllerConfig.h"

namespace sightline_v2 {

class ControllerConfigStore {
 public:
  bool begin();
  ControllerConfig defaults() const;
  bool loadPersisted(ControllerConfig& outConfig, bool& outUsedDefaults);
  bool savePersisted(const ControllerConfig& config, String& outError);
  bool applyToRuntime(const ControllerConfig& persisted, ControllerConfig& runtime, String& outError) const;

 private:
  bool validate(const ControllerConfig& config, String& outError) const;
};

}  // namespace sightline_v2
