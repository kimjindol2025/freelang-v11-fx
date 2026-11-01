#!/usr/bin/env bash
set -Eeuo pipefail
ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)
source "$ROOT/tests/l07/build.sh"
out=${WORKER_OUT:?}; compiler=${WORKER_COMPILER:?}; libdir=${WORKER_SANITIZER_LIBDIR:?}
mkdir -p "$out"
export LD_LIBRARY_PATH="$libdir${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 LSAN_OPTIONS=exitcode=23
for name in cancelled pending active; do
 "$compiler" --extern worker-timer-run --extern worker-observe --extern worker-pending --extern worker-exit "$ROOT/tests/worker-cleanup/$name.fl" "$out/$name.c" > "$out/$name.compile.log" 2>&1
 strace -f -e trace=execve -o "$out/$name.build.execve.log" gcc -O1 -g -w -fsanitize=address,undefined -fno-omit-frame-pointer -L "$libdir" -I "$ROOT/runtime" -include "$ROOT/tests/worker-cleanup/support.h" -o "$out/$name" "$out/$name.c" "$ROOT/tests/worker-cleanup/support.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}" > "$out/$name.build.log" 2>&1
 if timeout 120 "$out/$name" > "$out/$name.stdout" 2> "$out/$name.stderr"; then rc=0;else rc=$?;fi
 printf '%s\n' "$rc" > "$out/$name.exit"
 cat "$out/$name.stdout" "$out/$name.stderr"
 printf '%s_EXIT=%s\n' "$name" "$rc"
 if [[ "$rc" != 0 || -s "$out/$name.stderr" ]] || ! diff -u "$ROOT/tests/worker-cleanup/$name.expected" "$out/$name.stdout"; then
  [[ ${WORKER_BEFORE:-0} == 1 ]] || exit 1
 else printf '%s=PASS\n' "$name";fi
done
bash "$ROOT/scripts/audit-l07-execve.sh" "$out/"*.build.execve.log
[[ ${WORKER_BEFORE:-0} == 1 ]] || printf 'WORKER_CLEANUP=3/3\nLEAK_REPORTS=0\n'
