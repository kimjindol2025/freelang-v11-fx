#!/usr/bin/env bash
set -Eeuo pipefail
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
compiler="${FX_NATIVE_COMPILER:?Set FX_NATIVE_COMPILER to a native .fl compiler}"
raw_tcp_source="${FX_RAW_TCP_SOURCE:-$root/runtime/raw_tcp.c}"
work="$(mktemp -d)"
server_pid=''
cleanup() {
  [[ -z "$server_pid" ]] || { kill "$server_pid" 2>/dev/null || true; wait "$server_pid" 2>/dev/null || true; }
  rm -rf -- "$work"
}
trap cleanup EXIT

cat >"$work/http-v11-stub.c" <<'STUB'
#include "runtime.h"
FLValue fl_v11_http(FLValue name, FLValue argv) { (void)name; (void)argv; return fl_nil(); }
FLValue fl_v11_http_client(FLValue name, FLValue argv) { (void)name; (void)argv; return fl_nil(); }
STUB
"$compiler" "$root/tests/native-bootstrap/raw-tcp-deferred-boundaries.fl" "$work/server.c"
sources=("$root"/runtime/{core,collection,io,math,error,process,json,aliases,user-fns,gc,http-stub,http_client-stub,websocket,sqlite,mariadb,debug,regex,smtp,sse}.c)
gcc -O2 -w -I "$root/runtime" -o "$work/server" "$work/server.c" "${sources[@]}" "$raw_tcp_source" "$work/http-v11-stub.c" -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto

start_server() {
  local port="$1"
  FX_RAW_PROBE_MODE=v9 FX_RAW_TEST_PORT="$port" stdbuf -oL "$work/server" >"$work/server.log" 2>&1 &
  server_pid=$!
  for _ in {1..80}; do
    grep -Fxq 'RAW_SERVER_RESULT=ok' "$work/server.log" && return 0
    kill -0 "$server_pid" 2>/dev/null || return 1
    sleep 0.025
  done
  return 1
}
finish_server() { wait "$server_pid" || true; server_pid=''; }
secondary() {
  local port="$1" out="$2" start end line
  start=$(date +%s%N)
  exec 4<>"/dev/tcp/127.0.0.1/$port"
  IFS= read -r -t 5 line <&4 || true
  end=$(date +%s%N)
  printf 'READY=%s\nLATENCY_MS=%s\n' "$line" "$(((end-start)/1000000))" >"$out"
  exec 4>&-
  exec 4<&-
}
drain_client() {
  local port="$1" delay="$2" out="$3" ready total=0 n started ended
  exec 3<>"/dev/tcp/127.0.0.1/$port"
  IFS= read -r -t 5 ready <&3 || true
  printf 'READY=%s\n' "$ready" >"$out"
  printf START >&3
  started=$(date +%s%N)
  while (( total < 16777216 )); do
    n=$(dd bs=65536 count=1 iflag=fullblock <&3 2>/dev/null | wc -c | tr -d ' ')
    (( n > 0 )) || break
    total=$((total+n))
    sleep "$delay"
  done
  ended=$(date +%s%N)
  printf 'BYTES=%s\nELAPSED_MS=%s\n' "$total" "$(((ended-started)/1000000))" >>"$out"
  exec 3>&-
  exec 3<&-
}
no_read_client() {
  local port="$1" out="$2" ready total=0 n started ended
  exec 3<>"/dev/tcp/127.0.0.1/$port"
  IFS= read -r -t 5 ready <&3 || true
  printf 'READY=%s\n' "$ready" >"$out"
  printf START >&3
  started=$(date +%s%N)
  sleep 31
  while :; do
    n=$(dd bs=65536 count=1 iflag=fullblock <&3 2>/dev/null | wc -c | tr -d ' ')
    (( n > 0 )) || break
    total=$((total+n))
  done
  ended=$(date +%s%N)
  printf 'BYTES_AFTER_CLOSE=%s\nELAPSED_MS=%s\n' "$total" "$(((ended-started)/1000000))" >>"$out"
  exec 3>&-
  exec 3<&-
}
run_drain_case() {
  local label="$1" delay="$2" port=$((41000 + RANDOM % 20000))
  start_server "$port"
  drain_client "$port" "$delay" "$work/$label.client" &
  local client_pid=$!
  sleep 0.15
  secondary "$port" "$work/$label.secondary"
  wait "$client_pid"
  for _ in {1..100}; do grep -q '^V9_WRITE_RESULT=' "$work/server.log" && break; sleep 0.05; done
  printf 'CASE=%s\n' "$label"
  cat "$work/$label.client" "$work/$label.secondary"
  grep '^V9_.*RESULT=' "$work/server.log" | tail -n 2
  finish_server
}

run_drain_case SLIGHTLY_SLOW 0.002
run_drain_case MUCH_SLOWER 0.02

port=$((41000 + RANDOM % 20000))
start_server "$port"
timeout 40 bash -c 'exec 3<>"/dev/tcp/127.0.0.1/'"$port"'"; IFS= read -r -t 5 ready <&3 || true; printf "READY=%s\n" "$ready"; printf START >&3; started=$(date +%s%N); sleep 31; total=0; while :; do n=$(dd bs=65536 count=1 iflag=fullblock <&3 2>/dev/null | wc -c | tr -d " "); (( n > 0 )) || break; total=$((total+n)); done; ended=$(date +%s%N); printf "BYTES_AFTER_CLOSE=%s\nELAPSED_MS=%s\n" "$total" "$(((ended-started)/1000000))"' >"$work/no-read.client" &
no_read_pid=$!
sleep 0.15
secondary "$port" "$work/no-read.secondary"
wait "$no_read_pid"
for _ in {1..800}; do grep -q '^V9_WRITE_RESULT=' "$work/server.log" && break; sleep 0.05; done
printf 'CASE=NO_READ\n'
cat "$work/no-read.client" "$work/no-read.secondary"
grep '^V9_.*RESULT=' "$work/server.log" | tail -n 2
finish_server
