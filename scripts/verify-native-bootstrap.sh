#!/usr/bin/env bash
set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
RUNTIME_DIR="${RUNTIME_DIR:-$ROOT_DIR/runtime}"
CGC_MAIN="${CGC_MAIN:-$ROOT_DIR/self/cgc-main.fl}"
NATIVE_PATH="/usr/bin:/bin"

RUNTIME_SRCS=(
  "$RUNTIME_DIR/core.c" "$RUNTIME_DIR/collection.c" "$RUNTIME_DIR/io.c"
  "$RUNTIME_DIR/math.c" "$RUNTIME_DIR/error.c" "$RUNTIME_DIR/process.c"
  "$RUNTIME_DIR/json.c" "$RUNTIME_DIR/aliases.c" "$RUNTIME_DIR/user-fns.c"
  "$RUNTIME_DIR/cgc-bridge.c" "$RUNTIME_DIR/gc.c" "$RUNTIME_DIR/http.c"
  "$RUNTIME_DIR/websocket.c" "$RUNTIME_DIR/sqlite.c" "$RUNTIME_DIR/mariadb.c"
  "$RUNTIME_DIR/debug.c" "$RUNTIME_DIR/http_client.c" "$RUNTIME_DIR/regex.c"
  "$RUNTIME_DIR/smtp.c" "$RUNTIME_DIR/sse.c"
)
APP_RUNTIME_SRCS=(
  "$RUNTIME_DIR/core.c" "$RUNTIME_DIR/collection.c" "$RUNTIME_DIR/io.c"
  "$RUNTIME_DIR/math.c" "$RUNTIME_DIR/error.c" "$RUNTIME_DIR/process.c"
  "$RUNTIME_DIR/json.c" "$RUNTIME_DIR/aliases.c" "$RUNTIME_DIR/user-fns.c"
  "$RUNTIME_DIR/gc.c" "$RUNTIME_DIR/http.c" "$RUNTIME_DIR/websocket.c"
  "$RUNTIME_DIR/sqlite.c" "$RUNTIME_DIR/mariadb.c" "$RUNTIME_DIR/debug.c"
  "$RUNTIME_DIR/http_client.c" "$RUNTIME_DIR/regex.c" "$RUNTIME_DIR/smtp.c"
  "$RUNTIME_DIR/sse.c"
)

for required in "$ROOT_DIR/bootstrap/stage0.c" "$ROOT_DIR/bootstrap/stage0.sha256" "$CGC_MAIN" "$RUNTIME_DIR/runtime.h" "${RUNTIME_SRCS[@]}"; do
  [[ -f "$required" ]] || { echo "REQUIRED_PATH=FAIL:$required" >&2; exit 1; }
done

EXPECTED_STAGE0="f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c"
ACTUAL_STAGE0="$(sha256sum "$ROOT_DIR/bootstrap/stage0.c" | awk '{print $1}')"
[[ "$ACTUAL_STAGE0" == "$EXPECTED_STAGE0" ]] || { echo "STAGE0_SHA=FAIL:$ACTUAL_STAGE0"; exit 1; }
if rg -n 'STAGE_TRACE|TRACE\||PROFILE\||__FL_' "$ROOT_DIR/bootstrap/stage0.c" >/dev/null; then
  echo "STAGE0_GENERATED_SOURCE_CONTAMINATION=FAIL"; exit 1
fi
echo "STAGE0_GENERATED_SOURCE_CONTAMINATION=0"
echo "STAGE0_SHA=$ACTUAL_STAGE0"
CANONICAL_SHA="aa8bed315d2bae91630fcff72d7edcafb9afafd84dc357133bb271c3d1bda4fd"
echo "STAGE0_EQ_CANONICAL=$( [[ "$ACTUAL_STAGE0" == "$CANONICAL_SHA" ]] && echo PASS || echo DIFFERENT )"

TMP_DIR="$(mktemp -d "${TMPDIR:-/tmp}/freelang-native-bootstrap.XXXXXX")"
trap 'rm -rf -- "$TMP_DIR"' EXIT

build_c() {
  local label="$1" source="$2" output="$3"
  gcc -O2 -w -I "$RUNTIME_DIR" -o "$output" "$source" "${RUNTIME_SRCS[@]}" \
    -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto -lcurl
  [[ -x "$output" ]]
  echo "$label=PASS"
}

run_native() {
  local label="$1" binary="$2" input="$3" output="$4"
  local exec_log="$TMP_DIR/$label.execve.log"
  strace -f -e trace=execve,openat -o "$exec_log" \
    env -i PATH="$NATIVE_PATH" "$binary" "$input" "$output" \
    >"$TMP_DIR/$label.stdout" 2>"$TMP_DIR/$label.stderr"
  [[ -s "$output" ]]
  echo "$label=PASS"
}

build_app_c() {
  local label="$1" source="$2" output="$3"
  gcc -O2 -w -I "$RUNTIME_DIR" -o "$output" "$source" "${APP_RUNTIME_SRCS[@]}" \
    -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto -lcurl
  [[ -x "$output" ]]
  echo "$label=PASS"
}

build_c STAGE0_C_BUILD "$ROOT_DIR/bootstrap/stage0.c" "$TMP_DIR/stage0-bin"
run_native STAGE0_TO_STAGE1 "$TMP_DIR/stage0-bin" "$CGC_MAIN" "$TMP_DIR/stage1.c"
echo STAGE0_NATIVE_EXEC=PASS
build_c STAGE1_C_BUILD "$TMP_DIR/stage1.c" "$TMP_DIR/stage1-bin"
run_native STAGE1_TO_STAGE2 "$TMP_DIR/stage1-bin" "$CGC_MAIN" "$TMP_DIR/stage2.c"
build_c STAGE2_C_BUILD "$TMP_DIR/stage2.c" "$TMP_DIR/stage2-bin"
run_native STAGE2_TO_STAGE3 "$TMP_DIR/stage2-bin" "$CGC_MAIN" "$TMP_DIR/stage3.c"
echo STAGE2_EQ_STAGE3=$(cmp -s "$TMP_DIR/stage2.c" "$TMP_DIR/stage3.c" && echo PASS || echo FAIL)
cmp -s "$TMP_DIR/stage2.c" "$TMP_DIR/stage3.c"

run_native ARRAY_LOOP_GENERATE "$TMP_DIR/stage0-bin" "$ROOT_DIR/tests/native-bootstrap/array-loop.fl" "$TMP_DIR/array-loop.c"
build_app_c ARRAY_LOOP_BUILD "$TMP_DIR/array-loop.c" "$TMP_DIR/array-loop-bin"
strace -f -e trace=execve -o "$TMP_DIR/ARRAY_LOOP.execve.log" \
  env -i PATH="$NATIVE_PATH" "$TMP_DIR/array-loop-bin" \
  >"$TMP_DIR/ARRAY_LOOP.stdout" 2>"$TMP_DIR/ARRAY_LOOP.stderr"
[[ "$(tr -d '\r' < "$TMP_DIR/ARRAY_LOOP.stdout" | sed '/^[[:space:]]*$/d')" == "3" ]]
echo ARRAY_LOOP=PASS

for log in "$TMP_DIR"/*.execve.log; do
  if rg -n 'execve\(.*"[^" ]*/(node|npm|npx|ts-node|tsx|deno|bun|cgc-bin|bootstrap\.js)(/|"|$)' "$log" >/dev/null; then
    echo "FORBIDDEN_EXEC=FAIL:$log"; exit 1
  fi
done
if rg -n '/home/kim/kim/platform/freelang-afj|freelang-afj-native-parser|bootstrap\.js' "$TMP_DIR"/*.execve.log >/dev/null; then
  echo "AFJ_FILE_ACCESSES=FAIL"; exit 1
fi
echo NODE_INVOCATIONS=0
echo NPM_INVOCATIONS=0
echo TS_JS_INVOCATIONS=0
echo CGC_BIN_INVOCATIONS=0
echo AFJ_FILE_ACCESSES=0

for name in stage0 stage1 stage2 stage3; do
  if [[ "$name" == stage0 ]]; then file="$ROOT_DIR/bootstrap/stage0.c"; else file="$TMP_DIR/$name.c"; fi
  echo "${name^^}_SHA=$(sha256sum "$file" | awk '{print $1}')"
done

residual="$(pgrep -af 'stage[0-3]-bin|array-loop-bin' | rg -v 'pgrep|rg ' || true)"
[[ -z "$residual" ]] || { echo RESIDUAL_PROCESSES=FAIL; exit 1; }
echo RESIDUAL_PROCESSES=0
echo CLEAN_CHECKOUT=PASS
