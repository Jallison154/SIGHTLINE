#pragma once

#include <Arduino.h>

namespace sightline_v2 {

enum class DiscoveryDeviceType : uint8_t {
  kController = 1,
  kFixtureNode = 2
};

enum class DiscoveryHealth : uint8_t {
  kOk = 1,
  kDegraded = 2,
  kError = 3
};

enum class ClaimState : uint8_t {
  kAvailable = 1,
  kClaimed = 2
};

struct DiscoveryMessageV1 {
  uint16_t schemaVersion = 1;
  String msgType = "announce";
  String deviceId;
  DiscoveryDeviceType deviceType = DiscoveryDeviceType::kFixtureNode;
  String friendlyName;
  String ip;
  String firmwareVersion;
  DiscoveryHealth health = DiscoveryHealth::kOk;
  uint32_t uptimeMs = 0;
  uint32_t sentAtMs = 0;

  // Fixture-node specific fields
  String fixtureProfileName;
  ClaimState claimState = ClaimState::kAvailable;
  String assignedControllerId;

  // Controller specific field
  String targetNodeId;
};

struct DiscoveryAnnouncement {
  String deviceId;
  String friendlyName;
  String firmwareVersion;
  String ip;
  String status;  // e.g. "ok", "degraded"
  uint32_t uptimeMs = 0;
};

struct DiscoveredDeviceState {
  String deviceId;
  DiscoveryDeviceType deviceType = DiscoveryDeviceType::kFixtureNode;
  String friendlyName;
  String ip;
  String firmwareVersion;
  DiscoveryHealth health = DiscoveryHealth::kOk;
  String fixtureProfileName;
  ClaimState claimState = ClaimState::kAvailable;
  String assignedControllerId;
  String targetNodeId;
  uint32_t lastSeenMs = 0;
  uint32_t lastAnnounceAtMs = 0;
};

}  // namespace sightline_v2
