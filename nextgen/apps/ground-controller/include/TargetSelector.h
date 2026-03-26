#pragma once

#include <Arduino.h>

#include "DiscoveryService.h"

namespace sightline_v2 {

enum class TargetStatus : uint8_t {
  kUnknown = 0,
  kAvailable,
  kClaimedByUs,
  kClaimedByOther,
  kOffline
};

struct TargetInfo {
  String nodeId;
  String ip;
  String friendlyName;
  TargetStatus status = TargetStatus::kUnknown;
};

class TargetSelector {
 public:
  void updateFromDiscovery(uint32_t nowMs, const DiscoveryService& discovery, const String& controllerId);

  // Selection API.
  bool selectNext();
  bool selectPrevious();
  bool selectByNodeId(const String& nodeId);

  // State access.
  bool hasTarget() const { return _hasTarget; }
  const TargetInfo& active() const { return _active; }
  const String& targetNodeId() const { return _active.nodeId; }
  const String& targetIp() const { return _active.ip; }

  // Simple view for future UI.
  size_t candidateCount() const { return _candidateCount; }
  bool getCandidate(size_t idx, TargetInfo& out) const;

 private:
  static constexpr size_t kMaxCandidates = 16;

  TargetInfo _candidates[kMaxCandidates];
  size_t _candidateCount = 0;
  size_t _activeIndex = 0;
  bool _hasTarget = false;
  TargetInfo _active;
};

}  // namespace sightline_v2
