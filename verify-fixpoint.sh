#!/usr/bin/env bash
# verify-fixpoint.sh — current FreeLang fx self-hosting fixed-point check.
#
# Verifies cgc-bin -> gen-a.c -> gen-a-bin -> gen-b.c -> gen-b-bin -> gen-c.c
# and requires SHA-256(gen-a.c) == SHA-256(gen-b.c) == SHA-256(gen-c.c).
#
# The script is non-mutating: it never replaces cgc-bin and does not append to
# a repository log. Override paths with CGC_MAIN, CGC_BIN, or RUNTIME_DIR when
# a clean checkout does not have sibling repositories.

set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
CGC_MAIN="${CGC_MAIN:-${1:-$ROOT_DIR/self/cgc-main.fl}}"
RUNTIME_DIR="${RUNTIME_DIR:-$ROOT_DIR/runtime}"
TIMEOUT_SECONDS="${FIXPOINT_TIMEOUT_SECONDS:-180}"

find_cgc_bin() {
  if [[ -n "${CGC_BIN:-}" ]]; then
    printf '%s\n' "$CGC_BIN"
    return 0
  fi

  local candidate
  for candidate in \
    "$ROOT_DIR/../freelang-afj/bin/cgc-bin" \
    "$ROOT_DIR/bin/cgc-bin"; do
    if [[ -x "$candidate" ]]; then
      printf '%s\n' "$candidate"
      return 0
    fi
  done

  if command -v cgc-bin >/dev/null 2>&1; then
    command -v cgc-bin
    return 0
  fi
  return 1
}

CGC_BIN="$(find_cgc_bin)" || {
  echo "CGC_BIN=FAIL"
  echo "cgc-bin을 찾지 못했습니다. CGC_BIN=/path/to/cgc-bin 을 지정하세요." >&2
  exit 1
}

for required in "$CGC_MAIN" "$CGC_BIN" "$RUNTIME_DIR/runtime.h" \
  "$RUNTIME_DIR/user-fns.c" "$ROOT_DIR/tests/fixpoint-loop-array.fl"; do
  if [[ ! -e "$required" ]]; then
    echo "REQUIRED_PATH=FAIL: $required" >&2
    exit 1
  fi
done

if [[ ! -x "$CGC_BIN" ]]; then
  echo "CGC_BIN=FAIL: 실행 권한 없음: $CGC_BIN" >&2
  exit 1
fi

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

for source in "${RUNTIME_SRCS[@]}"; do
  [[ -f "$source" ]] || { echo "RUNTIME_SOURCE=FAIL: $source" >&2; exit 1; }
done

APP_RUNTIME_SRCS=(
  "$RUNTIME_DIR/core.c"
  "$RUNTIME_DIR/collection.c"
  "$RUNTIME_DIR/io.c"
  "$RUNTIME_DIR/math.c"
  "$RUNTIME_DIR/error.c"
  "$RUNTIME_DIR/process.c"
  "$RUNTIME_DIR/json.c"
  "$RUNTIME_DIR/aliases.c"
  "$RUNTIME_DIR/user-fns.c"
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

TMP_DIR="$(mktemp -d "${TMPDIR:-/tmp}/freelang-fixpoint.XXXXXX")"
cleanup() { rm -rf -- "$TMP_DIR"; }
trap cleanup EXIT

run_checked() {
  local label="$1"
  shift
  local log="$TMP_DIR/${label}.log"
  if ! timeout "$TIMEOUT_SECONDS" "$@" >"$log" 2>&1; then
    echo "${label}=FAIL"
    echo "--- ${label} diagnostics ---" >&2
    tail -80 "$log" >&2 || true
    exit 1
  fi
  echo "${label}=PASS"
}

build_c() {
  local label="$1"
  local source="$2"
  local output="$3"
  local log="$TMP_DIR/${label}.log"
  if ! gcc -O2 -w -I "$RUNTIME_DIR" -o "$output" "$source" \
      "${RUNTIME_SRCS[@]}" \
      -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto -lcurl \
      >"$log" 2>&1; then
    echo "${label}=FAIL"
    echo "--- ${label} diagnostics ---" >&2
    tail -80 "$log" >&2 || true
    exit 1
  fi
  [[ -x "$output" ]] || { echo "${label}=FAIL: executable missing"; exit 1; }
  echo "${label}=PASS"
}

build_app_c() {
  local label="$1"
  local source="$2"
  local output="$3"
  local log="$TMP_DIR/${label}.log"
  if ! gcc -O2 -w -I "$RUNTIME_DIR" -o "$output" "$source" \
      "${APP_RUNTIME_SRCS[@]}" \
      -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto -lcurl \
      >"$log" 2>&1; then
    echo "${label}=FAIL"
    echo "--- ${label} diagnostics ---" >&2
    tail -80 "$log" >&2 || true
    exit 1
  fi
  [[ -x "$output" ]] || { echo "${label}=FAIL: executable missing"; exit 1; }
  echo "${label}=PASS"
}

echo "SELF_HOST_FIXED_POINT=START"
echo "ROOT_DIR=$ROOT_DIR"
echo "CGC_MAIN=$CGC_MAIN"
echo "CGC_BIN=$CGC_BIN"
echo "RUNTIME_DIR=$RUNTIME_DIR"

run_checked GEN_A_GENERATE "$CGC_BIN" "$CGC_MAIN" "$TMP_DIR/gen-a.c"
build_c GEN_A_BUILD "$TMP_DIR/gen-a.c" "$TMP_DIR/gen-a-bin"
run_checked GEN_A_EXEC "$TMP_DIR/gen-a-bin" "$CGC_MAIN" "$TMP_DIR/gen-b.c"
build_c GEN_B_BUILD "$TMP_DIR/gen-b.c" "$TMP_DIR/gen-b-bin"
run_checked GEN_B_EXEC "$TMP_DIR/gen-b-bin" "$CGC_MAIN" "$TMP_DIR/gen-c.c"
build_c GEN_C_BUILD "$TMP_DIR/gen-c.c" "$TMP_DIR/gen-c-bin"

SHA_A="$(sha256sum "$TMP_DIR/gen-a.c" | awk '{print $1}')"
SHA_B="$(sha256sum "$TMP_DIR/gen-b.c" | awk '{print $1}')"
SHA_C="$(sha256sum "$TMP_DIR/gen-c.c" | awk '{print $1}')"
echo "GEN_A_SHA=$SHA_A"
echo "GEN_B_SHA=$SHA_B"
echo "GEN_C_SHA=$SHA_C"

if [[ "$SHA_A" == "$SHA_B" && "$SHA_B" == "$SHA_C" ]]; then
  echo "GEN_A_EQ_GEN_B_EQ_GEN_C=PASS"
else
  echo "GEN_A_EQ_GEN_B_EQ_GEN_C=FAIL"
  exit 1
fi

# Omitting user-fns.c must fail at link time because generated cgc-main uses
# the user-defined str-indent bridge. This guards the required source list.
MISSING_LOG="$TMP_DIR/missing-user-fns.log"
if gcc -O2 -w -I "$RUNTIME_DIR" -o "$TMP_DIR/missing-user-fns-bin" \
    "$TMP_DIR/gen-a.c" \
    "$RUNTIME_DIR/core.c" "$RUNTIME_DIR/collection.c" "$RUNTIME_DIR/io.c" \
    "$RUNTIME_DIR/math.c" "$RUNTIME_DIR/error.c" "$RUNTIME_DIR/process.c" \
    "$RUNTIME_DIR/json.c" "$RUNTIME_DIR/aliases.c" "$RUNTIME_DIR/cgc-bridge.c" \
    "$RUNTIME_DIR/gc.c" "$RUNTIME_DIR/http.c" "$RUNTIME_DIR/websocket.c" \
    "$RUNTIME_DIR/sqlite.c" "$RUNTIME_DIR/mariadb.c" "$RUNTIME_DIR/debug.c" \
    "$RUNTIME_DIR/http_client.c" "$RUNTIME_DIR/regex.c" "$RUNTIME_DIR/smtp.c" \
    "$RUNTIME_DIR/sse.c" -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto -lcurl \
    >"$MISSING_LOG" 2>&1; then
  echo "MISSING_USER_FNS_NEGATIVE_TEST=UNEXPECTED_PASS"
  exit 1
fi
if rg -q 'ufl_str_indent' "$MISSING_LOG"; then
  echo "MISSING_USER_FNS_NEGATIVE_TEST=FAIL_AS_EXPECTED"
else
  echo "MISSING_USER_FNS_NEGATIVE_TEST=UNEXPECTED_FAILURE"
  tail -80 "$MISSING_LOG" >&2 || true
  exit 1
fi

run_checked ARRAY_GENERATE "$CGC_BIN" "$ROOT_DIR/tests/fixpoint-loop-array.fl" "$TMP_DIR/array-loop.c"
build_app_c ARRAY_BUILD "$TMP_DIR/array-loop.c" "$TMP_DIR/array-loop-bin"
run_checked ARRAY_EXEC "$TMP_DIR/array-loop-bin"
if [[ "$(tr -d '\r' < "$TMP_DIR/ARRAY_EXEC.log" | sed '/^[[:space:]]*$/d')" == "3" ]]; then
  echo "ARRAY_LOOP=PASS"
else
  echo "ARRAY_LOOP=FAIL"
  cat "$TMP_DIR/ARRAY_EXEC.log" >&2
  exit 1
fi
echo "LABELED_LOOP=NOT_SUPPORTED_BY_CURRENT_STRUCTURE"

RESIDUAL_COUNT="$(
  {
    pgrep -x cgc-bin || true
    pgrep -x gen-a-bin || true
    pgrep -x gen-b-bin || true
    pgrep -x gen-c-bin || true
    pgrep -x array-loop-bin || true
  } | sort -u | sed '/^[[:space:]]*$/d' | wc -l
)"
if [[ "$RESIDUAL_COUNT" == "0" ]]; then
  echo "RESIDUAL_PROCESSES=0"
else
  echo "RESIDUAL_PROCESSES=$RESIDUAL_COUNT"
  exit 1
fi

echo "SELF_HOST_FIXED_POINT=PASS"
