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

"$compiler" "$root/tests/native-bootstrap/raw-peer.fl" "$work/raw.c"
cat >"$work/http-v11-stub.c" <<'STUB'
#include "runtime.h"
FLValue fl_v11_http(FLValue name, FLValue argv) { (void)name; (void)argv; return fl_nil(); }
FLValue fl_v11_http_client(FLValue name, FLValue argv) { (void)name; (void)argv; return fl_nil(); }
STUB
sources=("$root"/runtime/{core,collection,io,math,error,process,json,aliases,user-fns,gc,http-stub,http_client-stub,raw_tcp,websocket,sqlite,mariadb,debug,regex,smtp,sse}.c)
gcc -O2 -w -Werror=implicit-function-declaration -I "$root/runtime" -o "$work/raw" "$work/raw.c" "${sources[@]}" "$work/http-v11-stub.c" -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto

port="${FX_RAW_TEST_PORT:-$((40000 + RANDOM % 20000))}"
FX_RAW_TEST_PORT="$port" "$work/raw" >"$work/server.log" 2>&1 &
server_pid=$!
answer=''
for attempt in 1 2 3 4 5 6 7 8 9 10; do
  answer="$(FX_RAW_TEST_PORT="$port" timeout 3 bash -c 'exec 3<>/dev/tcp/127.0.0.1/$FX_RAW_TEST_PORT; IFS= read -r line <&3; printf "%s" "$line"' 2>/dev/null || true)"
  [[ -n "$answer" ]] && break
  sleep 0.2
done
printf '%s\n' "$answer"
if [[ "$answer" != 'RAW_PEER=PASS' ]]; then
  cat "$work/server.log" >&2
fi
[[ "$answer" == 'RAW_PEER=PASS' ]]
