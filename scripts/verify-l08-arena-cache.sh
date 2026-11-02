#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_ARENA_CACHE_OUT:-$(mktemp -d)}"
mkdir -p "$out"
compiler="${L08_ARENA_CACHE_COMPILER:-$out/compiler}"
if [[ -z "${L08_ARENA_CACHE_COMPILER:-}" ]];then l07_build_compiler "$out" > "$out/compiler-build.log" 2>&1;fi
"$compiler" --extern arena-cache-run "$ROOT/tests/l08-platform/arena-cache.fl" "$out/main.c" > "$out/compile.log" 2>&1
runtime_root="${L08_ARENA_CACHE_RUNTIME_ROOT:-$ROOT}"
sources=()
for unit in "${L07_RUNTIME[@]}";do sources+=("$runtime_root/runtime/$unit.c");done
flags=(-O1 -g -w -D_GNU_SOURCE -I "$runtime_root/runtime" -include "$ROOT/tests/l08-platform/arena-cache.h")
if [[ -n "${L08_ARENA_CACHE_SANITIZER_LIBDIR:-}" ]];then
 flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer -L "$L08_ARENA_CACHE_SANITIZER_LIBDIR")
 export LD_LIBRARY_PATH="$L08_ARENA_CACHE_SANITIZER_LIBDIR${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
 export ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1
fi
gcc "${flags[@]}" -o "$out/main" "$out/main.c" "$ROOT/tests/l08-platform/arena-cache.c" "${sources[@]}" "${L07_LIBS[@]}" -Wl,--wrap=malloc,--wrap=free,--wrap=fl_arena_alloc > "$out/build.log" 2>&1
if timeout 120 "$out/main" > "$out/actual" 2> "$out/stderr";then rc=0;else rc=$?;fi
cat "$out/actual" "$out/stderr"
printf 'ARENA_CACHE_PROGRAM_EXIT=%s\n' "$rc"
[[ "$rc" == 0 && ! -s "$out/stderr" ]]
diff -u "$ROOT/tests/l08-platform/arena-cache.expected" "$out/actual"
echo L08_ARENA_CACHE_PASS=1 FAIL=0
if [[ -n ${L08_ARENA_CACHE_SANITIZER_LIBDIR:-} ]];then echo GENERAL_LSAN=PASS;else echo GENERAL_LSAN=UNEXECUTED;fi
