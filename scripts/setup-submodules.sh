#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

echo "[CrossPoint] Synchronizing git submodules..."
git submodule sync --recursive
git submodule update --init --recursive --force

required_paths=(
  "freeink-sdk/libs/hardware/BatteryMonitor"
  "freeink-sdk/libs/hardware/InputManager"
  "freeink-sdk/libs/display/FreeInkDisplay"
  "freeink-sdk/libs/hardware/SDCardManager"
)

for path in "${required_paths[@]}"; do
  if [[ ! -d "$path" ]]; then
    echo "[CrossPoint] ERROR: missing required SDK path: $path" >&2
    exit 1
  fi
done

SDK_COMMIT="$(git -C freeink-sdk rev-parse HEAD)"
echo "[CrossPoint] FreeInk SDK ready at ${SDK_COMMIT}"
echo "[CrossPoint] Submodule setup complete."
