#!/usr/bin/env bash
# One-shot demo: start virtual node, run controller demo, exit.
set -euo pipefail
cd "$(dirname "$0")"
NODE_PY=fixture-node-sim/sim_node.py
CTL_PY=controller-sim/sim_controller.py
PROFILE="${1:-profiles/example_moving_head.json}"

python3 "$NODE_PY" --profile "$PROFILE" --quiet --summary-interval 2 &
PID=$!
cleanup() { kill "$PID" 2>/dev/null || true; }
trap cleanup EXIT
sleep 0.5
python3 "$CTL_PY" --demo --duration 6 --log-dir logs
