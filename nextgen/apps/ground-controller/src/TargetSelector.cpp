#include "TargetSelector.h"

namespace sightline_v2 {

void TargetSelector::updateFromDiscovery(uint32_t nowMs, const DiscoveryService& discovery,
                                         const String& controllerId) {
  (void)nowMs;
  _candidateCount = 0;

  const DiscoveryRegistry& reg = discovery.registry();
  for (size_t i = 0; i < reg.size() && _candidateCount < kMaxCandidates; ++i) {
    DiscoveredDeviceState d;
    if (!reg.get(i, d)) continue;
    if (d.deviceType != DiscoveryDeviceType::kFixtureNode) continue;

    TargetInfo& t = _candidates[_candidateCount++];
    t.nodeId = d.deviceId;
    t.ip = d.ip;
    t.friendlyName = d.friendlyName;

    const bool isOnline = true;  // TODO: use heartbeat age/health when exposed in registry.
    if (!isOnline) {
      t.status = TargetStatus::kOffline;
    } else if (d.claimState == ClaimState::kClaimed && d.assignedControllerId == controllerId) {
      t.status = TargetStatus::kClaimedByUs;
    } else if (d.claimState == ClaimState::kClaimed) {
      t.status = TargetStatus::kClaimedByOther;
    } else {
      t.status = TargetStatus::kAvailable;
    }
  }

  if (_candidateCount == 0) {
    _hasTarget = false;
    _active = TargetInfo{};
    return;
  }

  if (!_hasTarget || _active.nodeId.isEmpty()) {
    _activeIndex = 0;
    _active = _candidates[_activeIndex];
    _hasTarget = true;
  }
}

bool TargetSelector::selectNext() {
  if (_candidateCount == 0) return false;
  _activeIndex = (_activeIndex + 1) % _candidateCount;
  _active = _candidates[_activeIndex];
  _hasTarget = true;
  return true;
}

bool TargetSelector::selectPrevious() {
  if (_candidateCount == 0) return false;
  if (_activeIndex == 0) {
    _activeIndex = _candidateCount - 1;
  } else {
    _activeIndex--;
  }
  _active = _candidates[_activeIndex];
  _hasTarget = true;
  return true;
}

bool TargetSelector::selectByNodeId(const String& nodeId) {
  for (size_t i = 0; i < _candidateCount; ++i) {
    if (_candidates[i].nodeId == nodeId) {
      _activeIndex = i;
      _active = _candidates[i];
      _hasTarget = true;
      return true;
    }
  }
  return false;
}

bool TargetSelector::getCandidate(size_t idx, TargetInfo& out) const {
  if (idx >= _candidateCount) return false;
  out = _candidates[idx];
  return true;
}

}  // namespace sightline_v2
