#include "OwnershipManager.h"

namespace sightline_v2 {

void OwnershipManager::begin(const String& nodeId) {
  _nodeId = nodeId;
  _state = OwnershipLeaseState{};
  _state.nodeId = nodeId;
}

OwnershipClaimResponse OwnershipManager::buildResponse(const String& requestId, const String& controllerId,
                                                       OwnershipResult result, uint32_t nowMs) const {
  OwnershipClaimResponse res;
  res.requestId = requestId;
  res.nodeId = _nodeId;
  res.controllerId = controllerId;
  res.result = result;
  res.leaseMs = _state.leaseMs;
  res.expiresAtMs = (result == OwnershipResult::kAccepted) ? _state.leaseExpiryMs : nowMs;
  return res;
}

OwnershipClaimResponse OwnershipManager::handleClaimRequest(const OwnershipClaimRequest& request, uint32_t nowMs) {
  if (request.nodeId != _nodeId || request.controllerId.isEmpty()) {
    return buildResponse(request.requestId, request.controllerId, OwnershipResult::kRejectedInvalidTarget, nowMs);
  }

  if (!_state.claimed || _state.controllerId == request.controllerId) {
    _state.claimed = true;
    _state.controllerId = request.controllerId;
    _state.leaseMs = request.leaseMs;
    _state.leaseExpiryMs = nowMs + request.leaseMs;
    return buildResponse(request.requestId, request.controllerId, OwnershipResult::kAccepted, nowMs);
  }

  return buildResponse(request.requestId, request.controllerId, OwnershipResult::kRejectedAlreadyClaimed, nowMs);
}

OwnershipClaimResponse OwnershipManager::handleReleaseRequest(const OwnershipReleaseRequest& request, uint32_t nowMs) {
  if (request.nodeId != _nodeId) {
    return buildResponse(request.requestId, request.controllerId, OwnershipResult::kRejectedInvalidTarget, nowMs);
  }
  if (!_state.claimed || _state.controllerId != request.controllerId) {
    return buildResponse(request.requestId, request.controllerId, OwnershipResult::kRejectedNotOwner, nowMs);
  }

  _state.claimed = false;
  _state.controllerId = "";
  _state.leaseExpiryMs = 0;
  return buildResponse(request.requestId, request.controllerId, OwnershipResult::kAccepted, nowMs);
}

OwnershipClaimResponse OwnershipManager::handleHeartbeat(const OwnershipHeartbeat& heartbeat, uint32_t nowMs) {
  if (heartbeat.nodeId != _nodeId) {
    return buildResponse("hb", heartbeat.controllerId, OwnershipResult::kRejectedInvalidTarget, nowMs);
  }
  if (!_state.claimed || _state.controllerId != heartbeat.controllerId) {
    return buildResponse("hb", heartbeat.controllerId, OwnershipResult::kRejectedNotOwner, nowMs);
  }

  _state.leaseExpiryMs = nowMs + _state.leaseMs;
  return buildResponse("hb", heartbeat.controllerId, OwnershipResult::kAccepted, nowMs);
}

void OwnershipManager::tick(uint32_t nowMs) {
  if (_state.claimed && nowMs > _state.leaseExpiryMs) {
    _state.claimed = false;
    _state.controllerId = "";
    _state.leaseExpiryMs = 0;
  }
}

}  // namespace sightline_v2
