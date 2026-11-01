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
sources=("$root"/runtime/{core,collection,io,math,error,process,json,aliases,user-fns,gc,http,raw_tcp,websocket,sqlite,mariadb,debug,http_client,regex,smtp,sse}.c)
gcc -O2 -w -Werror=implicit-function-declaration -I "$root/runtime" -o "$work/raw" "$work/raw.c" "${sources[@]}" -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto -lcurl

run_one() {
  local port="$1" answer=''
  FX_RAW_TEST_PORT="$port" "$work/raw" >"$work/server.log" 2>&1 &
  server_pid=$!
  for attempt in 1 2 3 4 5 6 7 8 9 10; do
    answer="$(FX_RAW_TEST_PORT="$port" timeout 3 bash -c 'exec 3<>/dev/tcp/127.0.0.1/$FX_RAW_TEST_PORT; IFS= read -r line <&3; printf "%s" "$line"' 2>/dev/null || true)"
    [[ -n "$answer" ]] && break
    sleep 0.2
  done
  if [[ "$answer" != 'RAW_PEER=PASS' ]]; then
    printf 'TCP_PORT_FAILURE=%s ANSWER=%s\n' "$port" "$answer"
    cat "$work/server.log"
    return 1
  fi
  kill "$server_pid" 2>/dev/null || true
  wait "$server_pid" 2>/dev/null || true
  server_pid=''
}

# Keep test listeners outside this host's ephemeral-client range.
fixed="${FX_RAW_FIXED_PORT:-31000}"
for run in 1 2 3 4 5 6 7 8 9 10; do run_one "$fixed"; done
printf 'TCP_FIXED_PORT=10/10\n'
for run in 1 2 3 4 5 6 7 8 9 10; do
  port="$((20000 + RANDOM % 10000))"
  run_one "$port"
done
printf 'TCP_RANDOM_PORT=10/10\n'
