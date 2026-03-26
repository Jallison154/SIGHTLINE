#include "FixtureProfileWebApi.h"

namespace sightline_v2 {

bool FixtureProfileWebApi::begin(FixtureProfileManager& manager) {
  _manager = &manager;
  // TODO: Register lightweight HTTP handlers:
  //
  //  GET /api/profiles
  //  -> {
  //       "activeProfileId": string,
  //       "profiles": [
  //         { "id": string, "displayName": string, "manufacturer": string, "modeName": string }
  //       ]
  //     }
  //
  //  GET /api/profiles/:id
  //  -> full profile JSON document (for export/edit)
  //
  //  POST /api/profiles
  //  body: full profile JSON
  //  -> { "ok": true, "id": string } or { "ok": false, "error": string }
  //
  //  PUT /api/profiles/:id
  //  body: full profile JSON
  //  -> { "ok": true } or { "ok": false, "error": string }
  //
  //  DELETE /api/profiles/:id
  //  -> { "ok": true } or { "ok": false, "error": string }
  //
  //  POST /api/profiles/activate
  //  body: { "profileId": string }
  //  -> { "ok": true } or { "ok": false, "error": string }
  //
  // All handlers should:
  //  - decode/validate incoming JSON
  //  - never replace the active profile until validation success
  //  - on failure, keep the previous active profile in place
  return true;
}

void FixtureProfileWebApi::tick() {
  // TODO: Pump HTTP layer if needed by chosen web server implementation.
}

}  // namespace sightline_v2
