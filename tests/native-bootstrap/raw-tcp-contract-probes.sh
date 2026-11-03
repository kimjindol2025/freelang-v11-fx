#!/usr/bin/env bash
set -Eeuo pipefail

root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
compiler="${FX_NATIVE_COMPILER:?Set FX_NATIVE_COMPILER to the native compiler}"
work="$(mktemp -d)"
server_pid=''
cleanup() {
  if [[ -n "$server_pid" ]]; then kill "$server_pid" 2>/dev/null || true; wait "$server_pid" 2>/dev/null || true; fi
  rm -rf -- "$work"
}
trap cleanup EXIT

sources=("$root"/runtime/{core,collection,io,math,error,process,json,aliases,user-fns,gc,http,raw_tcp,websocket,sqlite,mariadb,debug,http_client,regex,smtp,sse}.c)
compile() {
  local name="$1" source="$2"
  "$compiler" "$source" "$work/$name.c"
  gcc -O2 -w -rdynamic -Werror=implicit-function-declaration \
    -I "$root/runtime" -o "$work/$name" "$work/$name.c" "${sources[@]}" \
    -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto -lcurl
}
compile raw-name "$root/tests/native-bootstrap/raw-name-input.fl"
compile boundaries "$root/tests/native-bootstrap/raw-tcp-deferred-boundaries.fl"

# The raw server must reject a handler selected by an environment supplied name.
name_port=$((40000 + RANDOM % 20000))
FX_RAW_TEST_PORT="$name_port" FX_RAW_HANDLER_NAME=raw-name-probe \
  "$work/raw-name" >"$work/name.log" 2>&1 &
server_pid=$!
for attempt in 1 2 3 4 5 6 7 8 9 10; do
  rg -q '^RAW_NAME_RESULT=' "$work/name.log" && break
  kill -0 "$server_pid" 2>/dev/null || break
  sleep 0.05
done
wait "$server_pid" || true
server_pid=''
rg -q '^RAW_NAME_RESULT=error$' "$work/name.log"
! rg -q '^RAW_NAME_CALLBACK=CALLED$' "$work/name.log"
echo 'RAW_NAME_DISPATCH=REJECTED'
start=$(date +%s)
FX_RAW_PROBE_MODE=float-yield FX_RAW_TEST_PORT=0 "$work/boundaries" >"$work/float.log" 2>&1
elapsed=$(( $(date +%s) - start ))
(( elapsed < 3 ))
echo "RAW_FLOAT_YIELD_SECONDS=$elapsed"

empty_port=$((40000 + RANDOM % 20000))
FX_RAW_PROBE_MODE=empty-host FX_RAW_TEST_PORT="$empty_port" \
  "$work/boundaries" >"$work/empty.log" 2>&1
rg -q '^RAW_EMPTY_HOST_RESULT=error$' "$work/empty.log"
cat "$work/empty.log"

start_server() {
  local mode="$1" port="$2" log="$3"
  FX_RAW_PROBE_MODE="$mode" FX_RAW_TEST_PORT="$port" \
    "$work/boundaries" >"$log" 2>&1 &
  server_pid=$!
  for attempt in 1 2 3 4 5 6 7 8 9 10; do
    rg -q '^RAW_SERVER_RESULT=ok$' "$log" && return 0
    kill -0 "$server_pid" 2>/dev/null || return 1
    sleep 0.05
  done
  return 1
}
finish_server() {
  wait "$server_pid"
  server_pid=''
}

chunk_port=$((40000 + RANDOM % 20000))
start_server chunk "$chunk_port" "$work/chunk.log"
head -c 8192 /dev/zero | tr '\000' x | \
  FX_RAW_TEST_PORT="$chunk_port" timeout 3 bash -c \
  'cat > /dev/tcp/127.0.0.1/$FX_RAW_TEST_PORT'
finish_server
rg -q '^RAW_DATA_LEN=4096$' "$work/chunk.log"
cat "$work/chunk.log"

nul_port=$((40000 + RANDOM % 20000))
start_server nul "$nul_port" "$work/nul.log"
FX_RAW_TEST_PORT="$nul_port" bash -c \
  'printf "A\0BCD" > /dev/tcp/127.0.0.1/$FX_RAW_TEST_PORT'
finish_server
rg -q '^RAW_DATA_LEN=1$' "$work/nul.log"
cat "$work/nul.log"

slow_port=$((40000 + RANDOM % 20000))
start_server slow "$slow_port" "$work/slow.log"
FX_RAW_TEST_PORT="$slow_port" timeout 6 bash -c \
  'exec 3<>/dev/tcp/127.0.0.1/$FX_RAW_TEST_PORT; sleep 4'
finish_server
rg -q '^RAW_SLOW_WRITE_RESULT=ok$' "$work/slow.log"
cat "$work/slow.log"
