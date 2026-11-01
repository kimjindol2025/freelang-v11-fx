#!/usr/bin/env bash
set -Eeuo pipefail

root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
compiler="${FX_NATIVE_COMPILER:?Set FX_NATIVE_COMPILER to the native compiler}"
work="$(mktemp -d)"
trap 'rm -rf -- "$work"' EXIT

"$compiler" "$root/tests/native-bootstrap/afj-file-names.fl" "$work/names.c"
gcc -O2 -w -Werror=implicit-function-declaration -I "$root/runtime" -o "$work/names" "$work/names.c" "$root"/runtime/{core,collection,io,math,error,process,json,aliases,user-fns,gc,http,websocket,sqlite,mariadb,debug,http_client,regex,smtp,sse}.c -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto -lcurl
output="$(FX_AFL_TEST_FILE="$work/data" "$work/names")"
printf '%s\n' "$output"
[[ "$output" == 'AFJ_FILE_NAMES=PASS' ]]
