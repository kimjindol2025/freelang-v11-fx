#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_ATOM_UPDATE_OUT:-$(mktemp -d)}"
mkdir -p "$out"
compiler="${L08_ATOM_UPDATE_COMPILER:-$out/compiler}"
if [[ -z "${L08_ATOM_UPDATE_COMPILER:-}" ]];then l07_build_compiler "$out" > "$out/compiler-build.log" 2>&1;fi
extern_args=()
for name in atom-test-start atom-test-workers atom-test-tick atom-test-ticks atom-test-pause atom-test-independent-pair atom-test-wait-peer atom-test-wait-owner atom-test-signal-peer;do
 extern_args+=(--extern "$name")
done
flags=(-O1 -g -w -D_GNU_SOURCE -I "$ROOT/runtime" -include "$ROOT/tests/l08-platform/atom-update.h")
if [[ -n "${L08_ATOM_UPDATE_SANITIZER_LIBDIR:-}" ]];then
 flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer -L "$L08_ATOM_UPDATE_SANITIZER_LIBDIR")
 export LD_LIBRARY_PATH="$L08_ATOM_UPDATE_SANITIZER_LIBDIR${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
 export ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1
fi
for name in atom-update atom-update-alias;do
 "$compiler" "${extern_args[@]}" "$ROOT/tests/l08-platform/$name.fl" "$out/$name.c" > "$out/$name.compile.log" 2>&1
 gcc "${flags[@]}" -o "$out/$name" "$out/$name.c" "$ROOT/tests/l08-platform/atom-update.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}" > "$out/$name.build.log" 2>&1
 if timeout 60 "$out/$name" > "$out/$name.actual" 2> "$out/$name.stderr";then rc=0;else rc=$?;fi
 cat "$out/$name.actual" "$out/$name.stderr"
 printf '%s_PROGRAM_EXIT=%s\n' "$name" "$rc"
 [[ "$rc" == 0 && ! -s "$out/$name.stderr" ]]
 diff -u "$ROOT/tests/l08-platform/$name.expected" "$out/$name.actual"
done
echo L08_ATOM_UPDATE_PASS=2 FAIL=0
echo ATOM_UPDATE_LEAK_CHECK=UNEXECUTED
