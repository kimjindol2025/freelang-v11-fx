#!/usr/bin/env bash
set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
OUTPUT="${1:-${NATIVE_STAGE0_PATH:-$ROOT_DIR/bootstrap/stage0-bin}}"

"$ROOT_DIR/scripts/build-native-stage0.sh" "$OUTPUT"
echo "NATIVE_STAGE0_INSTALLED=$OUTPUT"
