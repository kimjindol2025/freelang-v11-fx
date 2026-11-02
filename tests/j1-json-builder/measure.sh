#!/usr/bin/env bash
# Caller records ps and holds the shared heavy-test lock. Same limits before and after.
set -Eeuo pipefail
ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)
source "$ROOT/tests/l07/build.sh"
out=${J1_OUT:?};mkdir -p "$out"
gcc -O2 -w -I "$ROOT/runtime" -o "$out/measure" "$ROOT/tests/j1-json-builder/measure.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}"
for api in legacy v11;do
 for n in 1000 10000 100000;do
  printf 'MEASUREMENT API=%s ITEMS=%s VM_LIMIT_KIB=2097152 TIME_LIMIT_SECONDS=120\n' "$api" "$n"
  if (ulimit -c 0;ulimit -v 2097152; /usr/bin/time -f 'PROCESS_SECONDS=%e MAX_RSS_KIB=%M EXIT=%x' timeout 120 "$out/measure" "$api" "$n");then rc=0;else rc=$?;fi
  printf 'MEASUREMENT_EXIT=%s\n' "$rc"
 done
done
