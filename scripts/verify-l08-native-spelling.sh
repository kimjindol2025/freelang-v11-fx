#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_NATIVE_SPELLING_OUT:-$(mktemp -d)}"
mkdir -p "$out"
compiler="${L08_NATIVE_SPELLING_COMPILER:-$out/compiler}"
if [[ -z "${L08_NATIVE_SPELLING_COMPILER:-}" ]];then l07_build_compiler "$out" > "$out/compiler-build.log" 2>&1;fi
"$compiler" "$ROOT/tests/l08-platform/native-spelling.fl" "$out/main.c" > "$out/build.log" 2>&1
gcc -O2 -w -I "$ROOT/runtime" -o "$out/main" "$out/main.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}" >> "$out/build.log" 2>&1
if timeout 60 "$out/main" > "$out/actual" 2> "$out/stderr";then rc=0;else rc=$?;fi
cat "$out/actual" "$out/stderr"
printf 'NATIVE_SPELLING_PROGRAM_EXIT=%s\n' "$rc"
[[ "$rc" == 0 && ! -s "$out/stderr" ]]
diff -u "$ROOT/tests/l08-platform/native-spelling.expected" "$out/actual"
echo L08_NATIVE_SPELLING_SEQUENCE_PASS=1 FAIL=0
