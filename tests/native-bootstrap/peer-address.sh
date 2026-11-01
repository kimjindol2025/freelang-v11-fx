#!/usr/bin/env bash
set -Eeuo pipefail

root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
native="${FX_NATIVE_STAGE0:?Set FX_NATIVE_STAGE0 to the native stage0 compiler}"
work="$(mktemp -d)"
server_pid=''
cleanup() {
  if [[ -n "$server_pid" ]]; then kill "$server_pid" 2>/dev/null || true; wait "$server_pid" 2>/dev/null || true; fi
  rm -rf -- "$work"
}
trap cleanup EXIT

"$native" "$root/tests/native-bootstrap/peer-address.fl" "$work/peer.c"
gcc -O2 -w -I "$root/runtime" -o "$work/peer" "$work/peer.c" "$root"/runtime/{core,collection,io,math,error,process,json,aliases,user-fns,gc,http,websocket,sqlite,mariadb,debug,http_client,regex,smtp,sse}.c -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto -lcurl

port="${FX_PEER_TEST_PORT:-40977}"
FX_PEER_TEST_PORT="$port" "$work/peer" >"$work/server.log" 2>&1 &
server_pid=$!
body=''
for attempt in 1 2 3 4 5 6 7 8 9 10; do
  body="$(curl --silent --max-time 1 -H 'X-Forwarded-For: 198.51.100.7' "http://127.0.0.1:$port/peer" || true)"
  [[ -n "$body" ]] && break
  sleep 0.2
done
printf 'PEER_ADDRESS_BODY=%s\n' "$body"
[[ "$body" == '127.0.0.1' ]]
echo 'PEER_ADDRESS=PASS'
