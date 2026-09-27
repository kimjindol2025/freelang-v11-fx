#!/usr/bin/env bash
set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
RUNTIME_DIR="${RUNTIME_DIR:-$ROOT_DIR/runtime}"
OUTPUT="${1:-$ROOT_DIR/bootstrap/stage0-bin}"

RUNTIME_SRCS=(
  "$RUNTIME_DIR/core.c"
  "$RUNTIME_DIR/collection.c"
  "$RUNTIME_DIR/io.c"
  "$RUNTIME_DIR/math.c"
  "$RUNTIME_DIR/error.c"
  "$RUNTIME_DIR/process.c"
  "$RUNTIME_DIR/json.c"
  "$RUNTIME_DIR/aliases.c"
  "$RUNTIME_DIR/user-fns.c"
  "$RUNTIME_DIR/cgc-bridge.c"
  "$RUNTIME_DIR/gc.c"
  "$RUNTIME_DIR/http.c"
  "$RUNTIME_DIR/websocket.c"
  "$RUNTIME_DIR/sqlite.c"
  "$RUNTIME_DIR/mariadb.c"
  "$RUNTIME_DIR/debug.c"
  "$RUNTIME_DIR/http_client.c"
  "$RUNTIME_DIR/regex.c"
  "$RUNTIME_DIR/smtp.c"
  "$RUNTIME_DIR/sse.c"
)

for required in "$ROOT_DIR/bootstrap/stage0.c" "$RUNTIME_DIR/runtime.h" "${RUNTIME_SRCS[@]}"; do
  [[ -f "$required" ]] || { echo "REQUIRED_PATH=FAIL:$required" >&2; exit 1; }
done

gcc -O2 -w -I "$RUNTIME_DIR" -o "$OUTPUT" \
  "$ROOT_DIR/bootstrap/stage0.c" "${RUNTIME_SRCS[@]}" \
  -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto -lcurl

[[ -x "$OUTPUT" ]]
echo "STAGE0_C_BUILD=PASS"
echo "STAGE0_BIN=$OUTPUT"
