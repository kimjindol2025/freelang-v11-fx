#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_CALL_CONTEXT_OUT:?}"
compiler="${L08_CALL_CONTEXT_COMPILER:?}"
mkdir -p "$out"
flags=(-O1 -g -I "$ROOT/runtime")
if [[ -n ${L08_CALL_CONTEXT_SANITIZER_LIBDIR:-} ]];then
 flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer -L "$L08_CALL_CONTEXT_SANITIZER_LIBDIR")
 export LD_LIBRARY_PATH="$L08_CALL_CONTEXT_SANITIZER_LIBDIR${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
 export ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1
fi
pass=0;fail=0
for name in call-error-context compiler-blockers try-scope atom-snapshot;do
 "$compiler" "$ROOT/tests/l08-platform/$name.fl" "$out/$name.c" > "$out/$name-build.log" 2>&1
 gcc "${flags[@]}" -o "$out/$name" "$out/$name.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}" >> "$out/$name-build.log" 2>&1
 if timeout 60 "$out/$name" > "$out/$name.actual" 2> "$out/$name.stderr";then rc=0;else rc=$?;fi
 cat "$out/$name.actual" "$out/$name.stderr"
 printf '%s_PROGRAM_EXIT=%s\n' "$name" "$rc"
 if [[ "$rc" == 0 && ! -s "$out/$name.stderr" ]] && diff -u "$ROOT/tests/l08-platform/$name.expected" "$out/$name.actual";then pass=$((pass+1));else fail=$((fail+1));fi
done
printf 'L08_CALL_CONTEXT_PASS=%s FAIL=%s\n' "$pass" "$fail"
[[ "$fail" == 0 ]]
