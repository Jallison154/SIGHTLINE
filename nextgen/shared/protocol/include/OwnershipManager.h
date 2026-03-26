#pragma once

#include <Arduino.h>

#include "ClaimProtocol.h"

namespace sightline_v2 {

class OwnershipManager {
 public:
  void begin(const String& nodeId);

  OwnershipClaimResponse handleClaimRequest(const OwnershipClaimRequest& request, uint32_t nowMs);
  OwnershipClaimResponse handleReleaseRequest(const OwnershipReleaseRequest& request, uint32_t nowMs);
  OwnershipClaimResponse handleHeartbeat(const OwnershipHeartbeat& heartbeat, uint32_t nowMs);

  void tick(uint32_t nowMs);
  const OwnershipLeaseState& state() const { return _state; }

 private:
  OwnershipClaimResponse buildResponse(const String& requestId, const String& controllerId, OwnershipResult result,
                                       uint32_t nowMs) const;

  String _nodeId;
  OwnershipLeaseState _state;
};

}  // namespace sightline_v2
