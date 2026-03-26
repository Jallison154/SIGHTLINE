#include "OwnershipClient.h"

namespace sightline_v2 {

void OwnershipClient::begin(const String& controllerId) { _controllerId = controllerId; }

void OwnershipClient::tick(uint32_t nowMs) {
  if (_owned && nowMs > _leaseExpiryMs) {
    // Safe fallback if responses are lost: treat ownership as expired locally.
    _owned = false;
  }
}

OwnershipClaimRequest OwnershipClient::makeClaimRequest(uint32_t nowMs) const {
  OwnershipClaimRequest req;
  req.requestId = String(nowMs);
  req.controllerId = _controllerId;
  req.nodeId = _targetNodeId;
  req.leaseMs = 3000;
  req.sentAtMs = nowMs;
  return req;
}

OwnershipReleaseRequest OwnershipClient::makeReleaseRequest(uint32_t nowMs) const {
  OwnershipReleaseRequest req;
  req.requestId = String(nowMs);
  req.controllerId = _controllerId;
  req.nodeId = _targetNodeId;
  req.sentAtMs = nowMs;
  return req;
}

OwnershipHeartbeat OwnershipClient::makeHeartbeat(uint32_t nowMs) const {
  OwnershipHeartbeat hb;
  hb.controllerId = _controllerId;
  hb.nodeId = _targetNodeId;
  hb.sentAtMs = nowMs;
  return hb;
}

void OwnershipClient::onClaimResponse(const OwnershipClaimResponse& response) {
  if (response.result == OwnershipResult::kAccepted && response.controllerId == _controllerId &&
      response.nodeId == _targetNodeId) {
    _owned = true;
    _leaseExpiryMs = response.expiresAtMs;
  } else if (response.result != OwnershipResult::kAccepted && response.controllerId == _controllerId) {
    _owned = false;
  }
}

}  // namespace sightline_v2
