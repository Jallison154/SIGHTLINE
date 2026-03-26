#pragma once

#include <Arduino.h>

#include "ClaimProtocol.h"

namespace sightline_v2 {

class OwnershipClient {
 public:
  void begin(const String& controllerId);
  void setTargetNodeId(const String& nodeId) { _targetNodeId = nodeId; }
  void tick(uint32_t nowMs);

  OwnershipClaimRequest makeClaimRequest(uint32_t nowMs) const;
  OwnershipReleaseRequest makeReleaseRequest(uint32_t nowMs) const;
  OwnershipHeartbeat makeHeartbeat(uint32_t nowMs) const;
  void onClaimResponse(const OwnershipClaimResponse& response);

  bool isOwner() const { return _owned; }
  const String& targetNodeId() const { return _targetNodeId; }

 private:
  String _controllerId;
  String _targetNodeId;
  bool _owned = false;
  uint32_t _leaseExpiryMs = 0;
  uint32_t _lastHeartbeatMs = 0;
  uint32_t _heartbeatPeriodMs = 1000;
};

}  // namespace sightline_v2
