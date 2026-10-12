#!/usr/bin/env bash
# Phase 1 ④ — cgc kebab API naming warn + --strict-names
# Builds a fresh compiler from self/cgc-main.fl (installed cgc-bin may be stale).
set -Eeuo pipefail
ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
RUNTIME_DIR="${RUNTIME_DIR:-$ROOT_DIR/runtime}"
TMP="${TMPDIR:-/tmp}/fx-six-kebab-$$"
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
    "$ROOT_DIR/bin/cgc-bin"; do
    if [[ -x "$c" ]]; then printf '%s\n' "$c"; return 0; fi
  done
  return 1
}

SEED_CGC="$(find_seed_cgc)" || {
  echo "SIX_KEBAB_NAMES=FAIL no seed cgc-bin"
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
    echo "SIX_KEBAB_NAMES=FAIL link_compiler"
    tail -40 "$log" >&2 || true
    exit 1
  fi
}

cd "$ROOT_DIR"
echo "SEED_CGC=$SEED_CGC"
if ! "$SEED_CGC" "$ROOT_DIR/self/cgc-main.fl" "$TMP/fresh-cgc.c" >"$TMP/seed-compile.log" 2>&1; then
  echo "SIX_KEBAB_NAMES=FAIL seed_compile"
  tail -40 "$TMP/seed-compile.log" >&2 || true
  exit 1
fi
link_compiler "$TMP/fresh-cgc.c" "$TMP/fresh-cgc-bin" "$TMP/link-cgc.log"
FRESH="$TMP/fresh-cgc-bin"

# 1) kebab fixture → naming 경고 (http-get / server-get / json-stringify)
set +e
OUT1="$("$FRESH" "$ROOT_DIR/tests/six/kebab-names.fl" "$TMP/kebab.c" 2>&1)"
EC1=$?
set -e
echo "$OUT1" >"$TMP/out1.log"
echo "--- kebab fixture ---"
echo "$OUT1" | grep ' | naming | ' || true
for name in http-get server-get json-stringify; do
  if ! echo "$OUT1" | grep -q " | naming | ${name} | "; then
    echo "SIX_KEBAB_NAMES=FAIL missing_warn_${name}"
    echo "$OUT1" >&2
    exit 1
  fi
  if ! echo "$OUT1" | grep " | naming | ${name} | " | grep -qE 'http_get|server_get|json_stringify'; then
    echo "SIX_KEBAB_NAMES=FAIL bad_fix_for_${name}"
    exit 1
  fi
done
if [[ "$EC1" -ne 0 ]]; then
  echo "SIX_KEBAB_NAMES=FAIL unexpected_fail_without_strict ec=$EC1"
  exit 1
fi
if [[ ! -f "$TMP/kebab.c" ]]; then
  echo "SIX_KEBAB_NAMES=FAIL no_output_without_strict"
  exit 1
fi

# 2) --strict-names → 거부 (exit≠0, C 미기록)
rm -f "$TMP/strict.c"
set +e
OUT2="$("$FRESH" --strict-names "$ROOT_DIR/tests/six/kebab-names.fl" "$TMP/strict.c" 2>&1)"
EC2=$?
set -e
echo "$OUT2" >"$TMP/out2.log"
if [[ "$EC2" -eq 0 ]]; then
  echo "SIX_KEBAB_NAMES=FAIL strict_should_reject"
  echo "$OUT2" >&2
  exit 1
fi
if [[ -f "$TMP/strict.c" ]]; then
  echo "SIX_KEBAB_NAMES=FAIL strict_wrote_c"
  exit 1
fi
if ! echo "$OUT2" | grep -q 'strict-names:'; then
  echo "SIX_KEBAB_NAMES=FAIL strict_msg_missing"
  echo "$OUT2" >&2
  exit 1
fi

# 3) underscore fixture → naming 경고 0
set +e
OUT3="$("$FRESH" "$ROOT_DIR/tests/six/kebab-names-underscore.fl" "$TMP/under.c" 2>&1)"
EC3=$?
set -e
if [[ "$EC3" -ne 0 ]]; then
  echo "SIX_KEBAB_NAMES=FAIL underscore_compile"
  echo "$OUT3" >&2
  exit 1
fi
if echo "$OUT3" | grep -q ' | naming | '; then
  echo "SIX_KEBAB_NAMES=FAIL unexpected_naming_on_underscore"
  echo "$OUT3" >&2
  exit 1
fi

# 4) fx-world: 실제 kebab 이름과 경고 일치 (http-get 필수)
set +e
OUT4="$("$FRESH" "$ROOT_DIR/fx-world/server.fl" "$TMP/fx-world.c" 2>&1)"
EC4=$?
set -e
echo "$OUT4" >"$TMP/fx-world-warn.log"
echo "--- fx-world naming (sample) ---"
echo "$OUT4" | grep ' | naming | ' | head -20 || true
if ! echo "$OUT4" | grep -q ' | naming | http-get | '; then
  echo "SIX_KEBAB_NAMES=FAIL fx-world_missing_http-get"
  echo "$OUT4" | head -40 >&2
  exit 1
fi
# fx-world line 11 uses (http-get $url) — warn line should mention http-get → http_get
if ! echo "$OUT4" | grep ' | naming | http-get | ' | grep -q 'http_get 사용'; then
  echo "SIX_KEBAB_NAMES=FAIL fx-world_http-get_fix"
  exit 1
fi
if [[ "$EC4" -ne 0 ]]; then
  echo "SIX_KEBAB_NAMES=FAIL fx-world_compile_ec=$EC4"
  exit 1
fi

echo "SIX_KEBAB_NAMES=PASS"
echo "REGRESSION=tests/six/run-kebab-names.sh"
echo "FX_WORLD_NAMING_SAMPLE=$(echo "$OUT4" | grep ' | naming | ' | head -5 | tr '\n' '; ')"
