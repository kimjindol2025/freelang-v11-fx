#!/usr/bin/env bash
set -Eeuo pipefail

root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
compiler="${FX_NATIVE_COMPILER:?Set FX_NATIVE_COMPILER to the native compiler}"
work="$(mktemp -d)"
trap 'rm -rf -- "$work"' EXIT

sources=("$root"/runtime/{core,collection,io,math,error,process,json,aliases,user-fns,gc,http,websocket,sqlite,mariadb,debug,http_client,regex,smtp,sse}.c)
for name in has-key-values has-key-non-map; do
  "$compiler" "$root/tests/native-bootstrap/$name.fl" "$work/$name.c"
  gcc -O2 -w -Werror=implicit-function-declaration -I "$root/runtime" \
    -o "$work/$name" "$work/$name.c" "${sources[@]}" \
    -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto -lcurl
done

"$work/has-key-values"
if "$work/has-key-non-map" > "$work/non-map.log" 2>&1; then
  cat "$work/non-map.log"
  exit 1
fi
cat "$work/non-map.log"
rg -q 'HAS_KEY_EXPECTED_MAP' "$work/non-map.log"
printf 'HAS_KEY_NON_MAP=PASS\n'
