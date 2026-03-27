#!/usr/bin/env bash
set -euo pipefail

# Serve this installer folder over localhost so the browser can fetch manifest.json.
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PORT="${1:-8080}"

echo "Serving SIGHTLINE installer at: http://localhost:${PORT}"
echo "Press Ctrl+C to stop."
cd "${ROOT_DIR}"
python3 -m http.server "${PORT}"
