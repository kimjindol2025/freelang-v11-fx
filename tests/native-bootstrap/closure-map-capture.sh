#!/usr/bin/env bash
set -Eeuo pipefail

root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
compiler="${FX_NATIVE_COMPILER:?Set FX_NATIVE_COMPILER to the native compiler}"
work="$(mktemp -d)"
trap 'rm -rf -- "$work"' EXIT

"$compiler" "$root/tests/native-bootstrap/closure-map-capture.fl" "$work/capture.c"
gcc -O2 -w -Werror=implicit-function-declaration -I "$root/runtime" -o "$work/capture" "$work/capture.c" "$root"/runtime/{core,collection,io,math,error,process,json,aliases,user-fns,gc,http,websocket,sqlite,mariadb,debug,http_client,regex,smtp,sse}.c -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto -lcurl
output="$($work/capture)"
printf '%s\n' "$output"
[[ "$output" == 'CLOSURE_MAP_CAPTURE=PASS' ]]
