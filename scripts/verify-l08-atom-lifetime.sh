#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_ATOM_LIFETIME_OUT:-$(mktemp -d)}"
mkdir -p "$out"
compiler="${L08_ATOM_LIFETIME_COMPILER:-$out/compiler}"
if [[ -z "${L08_ATOM_LIFETIME_COMPILER:-}" ]];then l07_build_compiler "$out" > "$out/compiler-build.log" 2>&1;fi
flags=(-O1 -g -w -D_GNU_SOURCE -I "$ROOT/runtime" -include "$ROOT/tests/l08-platform/atom-lifetime.h")
if [[ -n "${L08_ATOM_LIFETIME_SANITIZER_LIBDIR:-}" ]];then
 flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer -L "$L08_ATOM_LIFETIME_SANITIZER_LIBDIR")
 export LD_LIBRARY_PATH="$L08_ATOM_LIFETIME_SANITIZER_LIBDIR${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
 export ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1
fi
for name in atom-lifetime atom-cycles;do
 "$compiler" --extern atom-meter-start --extern atom-meter-live --extern atom-meter-created --extern atom-meter-scope --extern atom-meter-barrier "$ROOT/tests/l08-platform/$name.fl" "$out/$name.c" > "$out/$name.compile.log" 2>&1
 gcc "${flags[@]}" -o "$out/$name" "$out/$name.c" "$ROOT/tests/l08-platform/atom-lifetime.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}" -Wl,--wrap=fl_atom_new,--wrap=free > "$out/$name.build.log" 2>&1
 if timeout 120 "$out/$name" > "$out/$name.actual" 2> "$out/$name.stderr";then rc=0;else rc=$?;fi
 cat "$out/$name.actual" "$out/$name.stderr"
 printf '%s_PROGRAM_EXIT=%s\n' "$name" "$rc"
 [[ "$rc" == 0 && ! -s "$out/$name.stderr" ]]
 diff -u "$ROOT/tests/l08-platform/$name.expected" "$out/$name.actual"
done
echo 'L08_ATOM_LIFETIME_PASS=2 FAIL=0'
if [[ -n ${L08_ATOM_LIFETIME_SANITIZER_LIBDIR:-} ]];then echo GENERAL_LSAN=PASS;else echo GENERAL_LSAN=UNEXECUTED;fi
