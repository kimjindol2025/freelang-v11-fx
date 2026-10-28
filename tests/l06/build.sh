#!/usr/bin/env bash
set -Eeuo pipefail
source "$(dirname -- "${BASH_SOURCE[0]}")/../l07/build.sh"
l06_app() {
  local out="$1" source="$2" name="$3"
  "$out/compiler" "$source" "$out/$name.c"
  gcc -O2 -w -D_GNU_SOURCE -I "$L07_ROOT/runtime" -I "$L07_ROOT/tests/l06" \
    -include "$L07_ROOT/tests/l06/support.h" -Wl,--wrap=listen \
    -o "$out/$name" "$out/$name.c" "$L07_ROOT/tests/l06/support.c" \
    "${L07_SRCS[@]}" "${L07_LIBS[@]}"
}
