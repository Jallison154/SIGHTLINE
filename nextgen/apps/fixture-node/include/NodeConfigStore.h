#pragma once

#include <Arduino.h>

#include "NodeConfig.h"

namespace sightline_v2 {

class NodeConfigStore {
 public:
  bool begin();
  NodeConfig defaults() const;
  bool loadPersisted(NodeConfig& outConfig, bool& outUsedDefaults);
  bool savePersisted(const NodeConfig& config, String& outError);
  bool applyToRuntime(const NodeConfig& persisted, NodeConfig& runtime, String& outError) const;

 private:
  bool validate(const NodeConfig& config, String& outError) const;
};

}  // namespace sightline_v2
