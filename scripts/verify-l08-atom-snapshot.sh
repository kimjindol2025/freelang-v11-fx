#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_SNAPSHOT_OUT:?}"
compiler="${L08_SNAPSHOT_COMPILER:?}"
mkdir -p "$out"
flags=(-O1 -g -I "$ROOT/runtime")
if [[ -n ${L08_SNAPSHOT_SANITIZER_LIBDIR:-} ]];then
 flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer -L "$L08_SNAPSHOT_SANITIZER_LIBDIR")
 export LD_LIBRARY_PATH="$L08_SNAPSHOT_SANITIZER_LIBDIR${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
 export ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1
fi
"$compiler" "$ROOT/tests/l08-platform/atom-snapshot.fl" "$out/atom-snapshot.c" > "$out/build.log" 2>&1
for name in atom-snapshot atom-snapshot-memory;do
 src="$out/$name.c";[[ "$name" != atom-snapshot-memory ]] || src="$ROOT/tests/l08-platform/$name.c"
 gcc "${flags[@]}" -o "$out/$name" "$src" "${L07_SRCS[@]}" "${L07_LIBS[@]}" >> "$out/build.log" 2>&1
 if timeout 60 "$out/$name" > "$out/$name.actual" 2> "$out/$name.stderr";then rc=0;else rc=$?;fi
 cat "$out/$name.actual" "$out/$name.stderr"
 printf '%s_EXIT=%s\n' "$name" "$rc"
 [[ "$rc" == 0 && ! -s "$out/$name.stderr" ]]
 [[ "$name" != atom-snapshot ]] || diff -u "$ROOT/tests/l08-platform/atom-snapshot.expected" "$out/$name.actual"
done
echo 'L08_ATOM_SNAPSHOT_PASS=2 FAIL=0'
echo SNAPSHOT_LEAK_CHECK=UNEXECUTED
