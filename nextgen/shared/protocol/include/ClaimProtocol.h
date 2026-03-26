#pragma once

#include <Arduino.h>

namespace sightline_v2 {

enum class OwnershipMsgType : uint8_t {
  kClaimRequest = 1,
  kClaimResponse = 2,
  kReleaseRequest = 3,
  kHeartbeat = 4,
  kStatus = 5
};

enum class OwnershipResult : uint8_t {
  kAccepted = 1,
  kRejectedAlreadyClaimed = 2,
  kRejectedInvalidTarget = 3,
  kRejectedNotOwner = 4
};

struct OwnershipClaimRequest {
  uint16_t schemaVersion = 1;
  String requestId;
  String controllerId;
  String nodeId;
  uint32_t leaseMs = 3000;
  uint32_t sentAtMs = 0;
};

struct OwnershipClaimResponse {
  uint16_t schemaVersion = 1;
  String requestId;
  String nodeId;
  String controllerId;
  OwnershipResult result = OwnershipResult::kAccepted;
  uint32_t leaseMs = 3000;
  uint32_t expiresAtMs = 0;
};

struct OwnershipReleaseRequest {
  uint16_t schemaVersion = 1;
  String requestId;
  String controllerId;
  String nodeId;
  uint32_t sentAtMs = 0;
};

struct OwnershipHeartbeat {
  uint16_t schemaVersion = 1;
  String controllerId;
  String nodeId;
  uint32_t sentAtMs = 0;
};

struct OwnershipLeaseState {
  bool claimed = false;
  String controllerId;
  String nodeId;
  uint32_t leaseMs = 3000;
  uint32_t leaseExpiryMs = 0;
};

}  // namespace sightline_v2
