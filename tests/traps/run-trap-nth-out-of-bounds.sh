#!/usr/bin/env bash
# Phase 1 regression for trap-nth-out-of-bounds.
# Builds a fresh compiler from self/cgc-main.fl (installed cgc-bin may be stale),
# then compiles and runs nth OOB fixture (hi/neg/empty). Expect three nil markers.
set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
RUNTIME_DIR="${RUNTIME_DIR:-$ROOT_DIR/runtime}"
SRC="$ROOT_DIR/tests/traps/trap-nth-out-of-bounds.fl"
TMP="${TMPDIR:-/tmp}/fx-trap-nth-out-of-bounds-$$"
EXPECT=$'oob-hi=nil\noob-neg=nil\nempty-0=nil'

mkdir -p "$TMP"
cleanup() { rm -rf -- "$TMP"; }
trap cleanup EXIT

find_seed_cgc() {
  if [[ -n "${CGC_BIN:-}" && -x "${CGC_BIN}" ]]; then
    printf '%s\n' "$CGC_BIN"; return 0
  fi
  local c
  for c in \
    "$ROOT_DIR/../freelang-afj/bin/cgc-bin" \
    "$ROOT_DIR/bin/cgc-bin" \
    "$ROOT_DIR/bootstrap/stage0-bin"; do
    if [[ -x "$c" ]]; then printf '%s\n' "$c"; return 0; fi
  done
  return 1
}

SEED_CGC="$(find_seed_cgc)" || {
  echo "TRAP_NTH_OUT_OF_BOUNDS=FAIL no seed cgc-bin"
  exit 1
}

RUNTIME_SRCS=(
  "$RUNTIME_DIR/core.c" "$RUNTIME_DIR/collection.c" "$RUNTIME_DIR/io.c"
  "$RUNTIME_DIR/math.c" "$RUNTIME_DIR/error.c" "$RUNTIME_DIR/process.c"
  "$RUNTIME_DIR/json.c" "$RUNTIME_DIR/aliases.c" "$RUNTIME_DIR/user-fns.c"
  "$RUNTIME_DIR/cgc-bridge.c" "$RUNTIME_DIR/gc.c" "$RUNTIME_DIR/http.c"
  "$RUNTIME_DIR/websocket.c" "$RUNTIME_DIR/sqlite.c" "$RUNTIME_DIR/mariadb.c"
  "$RUNTIME_DIR/debug.c" "$RUNTIME_DIR/http_client.c" "$RUNTIME_DIR/regex.c"
  "$RUNTIME_DIR/smtp.c" "$RUNTIME_DIR/sse.c"
)

link_compiler() {
  local src="$1" out="$2" log="$3"
  if ! gcc -O2 -w -I "$RUNTIME_DIR" -o "$out" "$src" "${RUNTIME_SRCS[@]}" \
      -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto -lcurl >"$log" 2>&1; then
    echo "TRAP_NTH_OUT_OF_BOUNDS=FAIL link_compiler"
    tail -40 "$log" >&2 || true
    exit 1
  fi
}

link_app() {
  local src="$1" out="$2" log="$3"
  # App binaries do not need cgc-bridge (lex/parse live in the compiler).
  if ! gcc -O2 -w -I "$RUNTIME_DIR" -o "$out" "$src" \
      "$RUNTIME_DIR/core.c" "$RUNTIME_DIR/collection.c" "$RUNTIME_DIR/io.c" \
      "$RUNTIME_DIR/math.c" "$RUNTIME_DIR/error.c" "$RUNTIME_DIR/process.c" \
      "$RUNTIME_DIR/json.c" "$RUNTIME_DIR/aliases.c" "$RUNTIME_DIR/user-fns.c" \
      "$RUNTIME_DIR/gc.c" "$RUNTIME_DIR/http.c" "$RUNTIME_DIR/websocket.c" \
      "$RUNTIME_DIR/sqlite.c" "$RUNTIME_DIR/mariadb.c" "$RUNTIME_DIR/debug.c" \
      "$RUNTIME_DIR/http_client.c" "$RUNTIME_DIR/regex.c" "$RUNTIME_DIR/smtp.c" \
      "$RUNTIME_DIR/sse.c" \
      -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto -lcurl >"$log" 2>&1; then
    echo "TRAP_NTH_OUT_OF_BOUNDS=FAIL link_app"
    tail -40 "$log" >&2 || true
    head -80 "$src" >&2 || true
    exit 1
  fi
}

cd "$ROOT_DIR"
echo "SEED_CGC=$SEED_CGC"
if ! "$SEED_CGC" "$ROOT_DIR/self/cgc-main.fl" "$TMP/fresh-cgc.c" >/tmp/fx-nth-oob-seed-compile.log 2>&1; then
  echo "TRAP_NTH_OUT_OF_BOUNDS=FAIL seed_compile"
  tail -40 /tmp/fx-nth-oob-seed-compile.log >&2 || true
  exit 1
fi
link_compiler "$TMP/fresh-cgc.c" "$TMP/fresh-cgc-bin" /tmp/fx-nth-oob-link-cgc.log

if ! "$TMP/fresh-cgc-bin" "$SRC" "$TMP/nested.c" >/tmp/fx-nth-oob-fixture-compile.log 2>&1; then
  echo "TRAP_NTH_OUT_OF_BOUNDS=FAIL fixture_compile"
  cat /tmp/fx-nth-oob-fixture-compile.log >&2 || true
  exit 1
fi
link_app "$TMP/nested.c" "$TMP/nested-bin" /tmp/fx-nth-oob-link-app.log

ACTUAL="$("$TMP/nested-bin" 2>/tmp/fx-nth-oob-run.err | tr -d '\r' | sed '/^[[:space:]]*$/d')"
echo "ACTUAL=$ACTUAL"

if [[ "$ACTUAL" == "$EXPECT" ]]; then
  echo "TRAP_NTH_OUT_OF_BOUNDS=PASS"
  echo "REGRESSION=tests/traps/trap-nth-out-of-bounds.fl"
  echo "COMMAND=bash tests/traps/run-trap-nth-out-of-bounds.sh"
  exit 0
fi

echo "TRAP_NTH_OUT_OF_BOUNDS=FAIL"
echo "EXPECTED=$EXPECT"
echo "---- nested.c (head) ----"
head -60 "$TMP/nested.c" || true
if [[ -s /tmp/fx-nth-oob-run.err ]]; then cat /tmp/fx-nth-oob-run.err; fi
exit 1
