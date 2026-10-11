#!/usr/bin/env bash
# Phase 1 regression for trap-parallel-run-missing.
# Builds fresh compiler from self/cgc-main.fl (do not rely on shared cgc-bin upgrade).
set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
RUNTIME_DIR="${RUNTIME_DIR:-$ROOT_DIR/runtime}"
SRC="$ROOT_DIR/tests/traps/trap-parallel-run-missing.fl"
TMP="${TMPDIR:-/tmp}/fx-trap-parallel-$$"
EXPECT=$'run-parallel=2\nparallel-run=3\nparallel-map=60'

mkdir -p "$TMP"
cleanup() { rm -rf -- "$TMP"; }
trap cleanup EXIT

find_seed_cgc() {
  if [[ -n "${CGC_BIN:-}" && -x "${CGC_BIN}" ]]; then printf '%s\n' "$CGC_BIN"; return 0; fi
  local c
  for c in "$ROOT_DIR/../freelang-afj/bin/cgc-bin" "$ROOT_DIR/bin/cgc-bin"; do
    [[ -x "$c" ]] && { printf '%s\n' "$c"; return 0; }
  done
  return 1
}

SEED_CGC="$(find_seed_cgc)" || { echo "TRAP_PARALLEL_RUN_MISSING=FAIL no seed cgc"; exit 1; }

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
  gcc -O2 -w -I "$RUNTIME_DIR" -o "$2" "$1" "${RUNTIME_SRCS[@]}" \
    -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto -lcurl >"$3" 2>&1
}

link_app() {
  gcc -O2 -w -I "$RUNTIME_DIR" -o "$2" "$1" \
    "$RUNTIME_DIR/core.c" "$RUNTIME_DIR/collection.c" "$RUNTIME_DIR/io.c" \
    "$RUNTIME_DIR/math.c" "$RUNTIME_DIR/error.c" "$RUNTIME_DIR/process.c" \
    "$RUNTIME_DIR/json.c" "$RUNTIME_DIR/aliases.c" "$RUNTIME_DIR/user-fns.c" \
    "$RUNTIME_DIR/gc.c" "$RUNTIME_DIR/http.c" "$RUNTIME_DIR/websocket.c" \
    "$RUNTIME_DIR/sqlite.c" "$RUNTIME_DIR/mariadb.c" "$RUNTIME_DIR/debug.c" \
    "$RUNTIME_DIR/http_client.c" "$RUNTIME_DIR/regex.c" "$RUNTIME_DIR/smtp.c" \
    "$RUNTIME_DIR/sse.c" \
    -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto -lcurl >"$3" 2>&1
}

cd "$ROOT_DIR"
echo "SEED_CGC=$SEED_CGC"
"$SEED_CGC" "$ROOT_DIR/self/cgc-main.fl" "$TMP/fresh-cgc.c" >/tmp/fx-par-seed.log 2>&1
link_compiler "$TMP/fresh-cgc.c" "$TMP/fresh-cgc-bin" /tmp/fx-par-link-cgc.log || {
  echo "TRAP_PARALLEL_RUN_MISSING=FAIL link_compiler"; tail -30 /tmp/fx-par-link-cgc.log; exit 1
}

"$TMP/fresh-cgc-bin" "$SRC" "$TMP/par.c" >/tmp/fx-par-fixture.log 2>&1 || {
  echo "TRAP_PARALLEL_RUN_MISSING=FAIL fixture_compile"; cat /tmp/fx-par-fixture.log; exit 1
}
link_app "$TMP/par.c" "$TMP/par-bin" /tmp/fx-par-link-app.log || {
  echo "TRAP_PARALLEL_RUN_MISSING=FAIL link_app"; tail -30 /tmp/fx-par-link-app.log; exit 1
}

ACTUAL="$("$TMP/par-bin" 2>/tmp/fx-par-run.err | tr -d '\r' | sed '/^[[:space:]]*$/d')"
echo "ACTUAL<<EOF"
printf '%s\n' "$ACTUAL"
echo "EOF"

if [[ "$ACTUAL" == "$EXPECT" ]]; then
  echo "TRAP_PARALLEL_RUN_MISSING=PASS"
  echo "REGRESSION=tests/traps/trap-parallel-run-missing.fl"
  echo "COMMAND=bash tests/traps/run-trap-parallel-run-missing.sh"
  echo "NOTE=no cgc-main.fl change this cycle; runtime fl_run_parallel + fx-std/parallel.fl"
  exit 0
fi

echo "TRAP_PARALLEL_RUN_MISSING=FAIL"
echo "EXPECTED<<EOF"; printf '%s\n' "$EXPECT"; echo "EOF"
cat /tmp/fx-par-run.err 2>/dev/null || true
exit 1
