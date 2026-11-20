#!/usr/bin/env bash
# FX 회귀 테스트: 사용법 run.sh <stage0-bin> [outroot]
set -u
compiler="${1:?stage0 bin}"
out="${2:-/home/kim/.tmp/db95-iso-fx-regress-out}"
fx=/home/kim/.tmp/db95-iso/fx
here="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
mkdir -p "$out"
SRCS="core.c collection.c io.c math.c error.c process.c json.c aliases.c user-fns.c gc.c http.c raw_tcp.c websocket.c sqlite.c mariadb.c debug.c http_client.c regex.c smtp.c sse.c cgc-bridge.c"
full=(); for s in $SRCS; do full+=("$fx/runtime/$s"); done
fail=0
for src in "$here"/t*.fl; do
  name="$(basename "$src" .fl)"
  "$compiler" "$src" "$out/$name.c" >"$out/$name.compile.log" 2>&1 || {
    echo "[$name] COMPILE_FAIL"; tail -3 "$out/$name.compile.log"; fail=$((fail+1)); continue; }
  gcc -O2 -w -I "$fx/runtime" -o "$out/$name" "$out/$name.c" "${full[@]}" \
    -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto >"$out/$name.gcc.log" 2>&1 || {
    echo "[$name] GCC_FAIL"; grep -E 'error|undefined' "$out/$name.gcc.log" | head -3; fail=$((fail+1)); continue; }
  echo "[$name] $(timeout 20 "$out/$name" 2>&1 | tr '\n' '|')"
done
echo "FAIL_COUNT=$fail"
exit "$fail"
