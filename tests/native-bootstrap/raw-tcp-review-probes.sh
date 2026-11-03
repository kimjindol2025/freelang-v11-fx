#!/usr/bin/env bash
set -Eeuo pipefail

root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
compiler="${FX_NATIVE_COMPILER:?Set FX_NATIVE_COMPILER to a native .fl compiler}"
raw_tcp_source="${FX_RAW_TCP_SOURCE:-$root/runtime/raw_tcp.c}"
work="$(mktemp -d)"
server_pid=''
client_pid=''
pass=0
fail=0
cleanup() {
  [[ -z "$client_pid" ]] || { kill "$client_pid" 2>/dev/null || true; wait "$client_pid" 2>/dev/null || true; }
  [[ -z "$server_pid" ]] || { kill "$server_pid" 2>/dev/null || true; wait "$server_pid" 2>/dev/null || true; }
  rm -rf -- "$work"
}
trap cleanup EXIT

cat >"$work/http-v11-stub.c" <<'STUB'
#include "runtime.h"
FLValue fl_v11_http(FLValue name, FLValue argv) { (void)name; (void)argv; return fl_nil(); }
FLValue fl_v11_http_client(FLValue name, FLValue argv) { (void)name; (void)argv; return fl_nil(); }
STUB
"$compiler" "$root/tests/native-bootstrap/raw-tcp-deferred-boundaries.fl" "$work/boundaries.c"
"$compiler" "$root/tests/native-bootstrap/raw-name-input.fl" "$work/raw-name.c"
sources=("$root"/runtime/{core,collection,io,math,error,process,json,aliases,user-fns,gc,http-stub,http_client-stub,websocket,sqlite,mariadb,debug,regex,smtp,sse}.c)
gcc -O2 -w -I "$root/runtime" -o "$work/boundaries" "$work/boundaries.c" "${sources[@]}" "$raw_tcp_source" "$work/http-v11-stub.c" -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto
gcc -O2 -w -I "$root/runtime" -o "$work/raw-name" "$work/raw-name.c" "${sources[@]}" "$raw_tcp_source" "$work/http-v11-stub.c" -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto

port=$((40000 + RANDOM % 20000))
FX_RAW_TEST_PORT="$port" FX_RAW_HANDLER_NAME=raw-name-probe stdbuf -oL "$work/raw-name" >"$work/name.log" 2>&1
if grep -Fxq 'RAW_NAME_RESULT=error' "$work/name.log" && ! grep -Fxq 'RAW_NAME_CALLBACK=CALLED' "$work/name.log"; then
  echo RAW_NAME_DISPATCH=REJECTED
else
  echo RAW_NAME_DISPATCH=FAIL
  cat "$work/name.log"
  exit 1
fi

check() {
  local name="$1" result="$2"
  if [[ "$result" == PASS ]]; then
    echo "RAW_TCP_CASE_${name}=PASS"
    pass=$((pass + 1))
  else
    echo "RAW_TCP_CASE_${name}=FAIL"
    fail=$((fail + 1))
  fi
}

port=$((40000 + RANDOM % 20000))
empty="$(FX_RAW_PROBE_MODE=empty-host FX_RAW_TEST_PORT="$port" "$work/boundaries" 2>&1 || true)"
null="$(FX_RAW_PROBE_MODE=null-host FX_RAW_TEST_PORT="$port" "$work/boundaries" 2>&1 || true)"
if grep -Fxq 'RAW_EMPTY_HOST_RESULT=error' <<<"$empty" && grep -Fxq 'RAW_NULL_HOST_RESULT=error' <<<"$null"; then check HOST PASS; else check HOST FAIL; fi
printf '%s\n%s\n' "$empty" "$null"

start_ns=$(date +%s%N)
float_output="$(FX_RAW_PROBE_MODE=float-yield FX_RAW_TEST_PORT=0 "$work/boundaries" 2>&1 || true)"
float_ns=$(( $(date +%s%N) - start_ns ))
float_ms=$((float_ns / 1000000))
if grep -Fxq 'RAW_FLOAT_YIELD_DONE=1' <<<"$float_output" && (( float_ms >= 50 )); then check FLOAT_YIELD PASS; else check FLOAT_YIELD FAIL; fi
printf 'RAW_FLOAT_YIELD_ELAPSED_MS=%s\n' "$float_ms"

start_server() {
  local mode="$1" port="$2" log="$3"
  FX_RAW_PROBE_MODE="$mode" FX_RAW_TEST_PORT="$port" stdbuf -oL "$work/boundaries" >"$log" 2>&1 &
  server_pid=$!
  for _ in {1..40}; do
    grep -Fxq 'RAW_SERVER_RESULT=ok' "$log" && return 0
    kill -0 "$server_pid" 2>/dev/null || return 1
    sleep 0.025
  done
  return 1
}
finish_server() {
  wait "$server_pid" || true
  server_pid=''
}

port=$((40000 + RANDOM % 20000))
if start_server chunk "$port" "$work/chunk.log"; then
  head -c 8192 /dev/zero | tr '\000' x | FX_RAW_TEST_PORT="$port" timeout 3 bash -c 'cat > /dev/tcp/127.0.0.1/$FX_RAW_TEST_PORT'
  finish_server
  if awk -F= '/^RAW_DATA_LEN=/{sum+=$2; n++} END{if(sum==8192 && n>=2) exit 0; exit 1}' "$work/chunk.log"; then check LARGE_CHUNKS PASS; else check LARGE_CHUNKS FAIL; fi
  cat "$work/chunk.log"
  else cat "$work/chunk.log"; check LARGE_CHUNKS FAIL; fi

port=$((40000 + RANDOM % 20000))
if start_server nul "$port" "$work/nul.log"; then
  FX_RAW_TEST_PORT="$port" bash -c 'exec 3<>/dev/tcp/127.0.0.1/$FX_RAW_TEST_PORT; printf "A\0BCD" >&3; dd bs=5 count=1 <&3 2>/dev/null' >"$work/binary.echo"
  finish_server
  printf 'A\0BCD' >"$work/binary.expected"
  if grep -Fxq 'RAW_DATA_LEN=5' "$work/nul.log" && grep -Fxq 'RAW_BINARY_WRITE_RESULT=ok' "$work/nul.log" && cmp -s "$work/binary.expected" "$work/binary.echo"; then check BINARY_LENGTH PASS; else check BINARY_LENGTH FAIL; fi
  printf 'RAW_BINARY_ECHO_BYTES=%s\n' "$(wc -c <"$work/binary.echo" | tr -d ' ')"
  cat "$work/nul.log"
else cat "$work/nul.log"; check BINARY_LENGTH FAIL; fi

port=$((40000 + RANDOM % 20000))
if start_server large-response "$port" "$work/large-response.log"; then
  timeout 3 bash -c 'exec 3<>/dev/tcp/127.0.0.1/'"$port"'; dd bs=8192 count=1 <&3 2>/dev/null' >"$work/response.bin"
  finish_server
  head -c 8192 /dev/zero | tr '\000' R >"$work/response.expected"
  if grep -Fxq 'RAW_LARGE_RESPONSE_RESULT=ok' "$work/large-response.log" && cmp -s "$work/response.expected" "$work/response.bin"; then check LARGE_RESPONSE PASS; else check LARGE_RESPONSE FAIL; fi
  printf 'RAW_LARGE_RESPONSE_BYTES=%s\n' "$(wc -c <"$work/response.bin" | tr -d ' ')"
else cat "$work/large-response.log"; check LARGE_RESPONSE FAIL; fi

port=$((40000 + RANDOM % 20000))
if start_server slow "$port" "$work/slow.log"; then
  start_ns=$(date +%s%N)
  FX_RAW_TEST_PORT="$port" timeout 40 bash -c 'exec 3<>/dev/tcp/127.0.0.1/$FX_RAW_TEST_PORT; sleep 31' &
  client_pid=$!
  for _ in {1..1400}; do
    grep -q '^RAW_SLOW_WRITE_RESULT=' "$work/slow.log" && break
    sleep 0.025
  done
  elapsed_ms=$(( ($(date +%s%N) - start_ns) / 1000000 ))
  result="$(sed -n 's/^RAW_SLOW_WRITE_RESULT=//p' "$work/slow.log" | head -n 1)"
  if [[ "$result" == error ]] && (( elapsed_ms >= 29000 && elapsed_ms < 40000 )); then check SLOW_CLIENT PASS; else check SLOW_CLIENT FAIL; fi
  kill "$client_pid" 2>/dev/null || true
  wait "$client_pid" 2>/dev/null || true
  client_pid=''
  finish_server
  result="$(sed -n 's/^RAW_SLOW_WRITE_RESULT=//p' "$work/slow.log" | head -n 1)"
  printf 'RAW_SLOW_WRITE_RESULT=%s\nRAW_SLOW_WRITE_ELAPSED_MS=%s\n' "$result" "$elapsed_ms"
  cat "$work/slow.log"
else cat "$work/slow.log"; check SLOW_CLIENT FAIL; fi

printf 'RAW_TCP_REVIEW_SUMMARY=%s/6\n' "$pass"
(( fail == 0 ))
