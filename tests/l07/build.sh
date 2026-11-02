#!/usr/bin/env bash
# Native-only build helpers; do not source fl-build.sh (it invokes Python).
set -Eeuo pipefail
L07_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
L07_RUNTIME=(core collection io math error process json aliases user-fns
  gc http websocket sqlite mariadb debug http_client regex smtp sse)
L07_SRCS=()
for unit in "${L07_RUNTIME[@]}"; do L07_SRCS+=("$L07_ROOT/runtime/$unit.c"); done
L07_LIBS=(-lm -lpthread -ldl -lsqlite3 -lssl -lcrypto -lcurl)
l07_build_compiler() {
  local out="$1"
  mkdir -p "$out"
  gcc -O2 -w -I "$L07_ROOT/runtime" -o "$out/seed" \
    "$L07_ROOT/bootstrap/stage0.c" "${L07_SRCS[@]}" \
    "$L07_ROOT/runtime/cgc-bridge.c" "${L07_LIBS[@]}"
  "$out/seed" "$L07_ROOT/self/cgc-main.fl" "$out/compiler.c"
  gcc -O2 -w -I "$L07_ROOT/runtime" -o "$out/compiler" \
    "$out/compiler.c" "${L07_SRCS[@]}" \
    "$L07_ROOT/runtime/cgc-bridge.c" "${L07_LIBS[@]}"
}
l07_build_app() {
  local out="$1" source="$2" name="$3"
  "$out/compiler" "$source" "$out/$name.c"
  gcc -O2 -w -I "$L07_ROOT/runtime" -o "$out/$name" \
    "$out/$name.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}"
}
