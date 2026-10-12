#!/usr/bin/env bash
# Phase 1 ③ — fl-build --bundle=… size + link smoke
set -Eeuo pipefail
ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
cd "$ROOT_DIR"
TMP="${TMPDIR:-/tmp}/fx-six-bundle-$$"
mkdir -p "$TMP"
cleanup() { rm -rf -- "$TMP"; }
trap cleanup EXIT

HELLO="$ROOT_DIR/templates/hello/server.fl"

echo "Building hello FULL (default)…"
bash "$ROOT_DIR/fl-build.sh" "$HELLO" "$TMP/hello-full.elf" >"$TMP/full.log" 2>&1
FULL_SIZE=$(stat -c%s "$TMP/hello-full.elf")
echo "FULL_BYTES=$FULL_SIZE"

echo "Building hello --bundle=core …"
bash "$ROOT_DIR/fl-build.sh" "$HELLO" "$TMP/hello-core.elf" --bundle=core >"$TMP/core.log" 2>&1
CORE_SIZE=$(stat -c%s "$TMP/hello-core.elf")
echo "CORE_BYTES=$CORE_SIZE"

if ! grep -q 'db stub' "$TMP/core.log"; then
  echo "SIX_BUNDLE=FAIL expected db stub in core-only log"
  exit 1
fi
if ! grep -q 'mail stub' "$TMP/core.log"; then
  echo "SIX_BUNDLE=FAIL expected mail stub in core-only log"
  exit 1
fi
if ! grep -q 'net stub' "$TMP/core.log"; then
  echo "SIX_BUNDLE=FAIL expected net stub in core-only log"
  exit 1
fi

if [[ "$CORE_SIZE" -ge "$FULL_SIZE" ]]; then
  echo "SIX_BUNDLE=FAIL core size not smaller than full ($CORE_SIZE >= $FULL_SIZE)"
  exit 1
fi

echo "Building hello --bundle=core,net …"
bash "$ROOT_DIR/fl-build.sh" "$HELLO" "$TMP/hello-corenet.elf" --bundle=core,net >"$TMP/corenet.log" 2>&1
CN_SIZE=$(stat -c%s "$TMP/hello-corenet.elf")
echo "CORE_NET_BYTES=$CN_SIZE"
if [[ "$CN_SIZE" -ge "$FULL_SIZE" ]]; then
  echo "SIX_BUNDLE=FAIL core,net size not smaller than full"
  exit 1
fi

# default path still mentions MariaDB (full)
if ! grep -q 'MariaDB' "$TMP/full.log"; then
  echo "SIX_BUNDLE=FAIL full build should include MariaDB line"
  exit 1
fi

echo "SIX_BUNDLE=PASS"
echo "FULL_BYTES=$FULL_SIZE"
echo "CORE_BYTES=$CORE_SIZE"
echo "CORE_NET_BYTES=$CN_SIZE"
echo "REGRESSION=tests/six/run-bundle.sh"
