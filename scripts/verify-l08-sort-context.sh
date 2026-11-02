#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_SORT_CONTEXT_OUT:-$(mktemp -d)}"
mkdir -p "$out"
compiler="${L08_SORT_CONTEXT_COMPILER:-$out/compiler}"
if [[ -z "${L08_SORT_CONTEXT_COMPILER:-}" ]];then l07_build_compiler "$out" > "$out/compiler-build.log" 2>&1;fi
"$compiler" --extern atom-test-start --extern atom-test-workers --extern atom-test-pause "$ROOT/tests/l08-platform/sort-context.fl" "$out/main.c" > "$out/compile.log" 2>&1
flags=(-O1 -g -w -D_GNU_SOURCE -I "$ROOT/runtime" -include "$ROOT/tests/l08-platform/atom-update.h")
if [[ -n "${L08_SORT_CONTEXT_SANITIZER_LIBDIR:-}" ]];then
 flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer -L "$L08_SORT_CONTEXT_SANITIZER_LIBDIR")
 export LD_LIBRARY_PATH="$L08_SORT_CONTEXT_SANITIZER_LIBDIR${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
 export ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1
fi
gcc "${flags[@]}" -o "$out/main" "$out/main.c" "$ROOT/tests/l08-platform/atom-update.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}" > "$out/build.log" 2>&1
if timeout 60 "$out/main" > "$out/actual" 2> "$out/stderr";then rc=0;else rc=$?;fi
cat "$out/actual" "$out/stderr"
printf 'SORT_CONTEXT_PROGRAM_EXIT=%s\n' "$rc"
[[ "$rc" == 0 && ! -s "$out/stderr" ]]
diff -u "$ROOT/tests/l08-platform/sort-context.expected" "$out/actual"
echo L08_SORT_CONTEXT_PASS=1 FAIL=0
if [[ -n ${L08_SORT_CONTEXT_SANITIZER_LIBDIR:-} ]];then echo SORT_CONTEXT_LEAK_CHECK=PASS;else echo SORT_CONTEXT_LEAK_CHECK=UNEXECUTED;fi
