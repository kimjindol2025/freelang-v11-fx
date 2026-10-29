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
"$compiler" "${extern_args[@]}" "$ROOT/tests/l08-platform/atom-update.fl" "$out/main.c" > "$out/compile.log" 2>&1
flags=(-O1 -g -w -D_GNU_SOURCE -I "$ROOT/runtime" -include "$ROOT/tests/l08-platform/atom-update.h")
if [[ -n "${L08_ATOM_UPDATE_SANITIZER_LIBDIR:-}" ]];then
 flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer -L "$L08_ATOM_UPDATE_SANITIZER_LIBDIR")
 export LD_LIBRARY_PATH="$L08_ATOM_UPDATE_SANITIZER_LIBDIR${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
 export ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1
fi
gcc "${flags[@]}" -o "$out/main" "$out/main.c" "$ROOT/tests/l08-platform/atom-update.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}" > "$out/build.log" 2>&1
if timeout 60 "$out/main" > "$out/actual" 2> "$out/stderr";then rc=0;else rc=$?;fi
cat "$out/actual" "$out/stderr"
printf 'ATOM_UPDATE_PROGRAM_EXIT=%s\n' "$rc"
[[ "$rc" == 0 && ! -s "$out/stderr" ]]
diff -u "$ROOT/tests/l08-platform/atom-update.expected" "$out/actual"
echo L08_ATOM_UPDATE_PASS=1 FAIL=0
echo ATOM_UPDATE_LEAK_CHECK=UNEXECUTED
