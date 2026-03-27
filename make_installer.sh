#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FW_DIR="${ROOT_DIR}/fixture-node-firmware"
INSTALLER_DIR="${ROOT_DIR}/web-installer/fixture-node"
ENV_NAME="${SIGHTLINE_PIO_ENV:-esp32_eth_dev}"
PORT="${1:-8080}"
PIO_CMD=()

if python3 -m platformio --version >/dev/null 2>&1; then
  PIO_CMD=(python3 -m platformio)
elif command -v pio >/dev/null 2>&1; then
  PIO_CMD=(pio)
else
  echo "ERROR: PlatformIO not found."
  echo "Install with: python3 -m pip install -U platformio"
  echo "Or ensure 'pio' is available in PATH."
  exit 1
fi

echo "SIGHTLINE installer pipeline"
echo "- env: ${ENV_NAME}"
echo "- port: ${PORT}"
echo "- pio: ${PIO_CMD[*]}"

cd "${FW_DIR}"
"${PIO_CMD[@]}" run -e "${ENV_NAME}"
"${PIO_CMD[@]}" run -e "${ENV_NAME}" -t buildfs
python3 tools/prepare_web_installer.py --env "${ENV_NAME}"

cd "${INSTALLER_DIR}"
./serve_local.sh "${PORT}"
