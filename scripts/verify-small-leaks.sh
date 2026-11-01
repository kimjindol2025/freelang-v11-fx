#!/usr/bin/env bash
set -Eeuo pipefail
ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)
source "$ROOT/tests/l07/build.sh"
out=${SMALL_LEAKS_OUT:?};compiler=${SMALL_LEAKS_COMPILER:?}
libdir=${SMALL_LEAKS_SANITIZER_LIBDIR:?};mode=${SMALL_LEAKS_MODE:-after}
[[ "$mode" == before || "$mode" == after ]]
mkdir -p "$out"
export LD_LIBRARY_PATH="$libdir${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 LSAN_OPTIONS=exitcode=23
pass=0
for name in tail-call-context tail-call-extra;do
 "$compiler" "$ROOT/tests/small-leaks/$name.fl" "$out/$name.c" > "$out/$name.compile.log" 2>&1
 strace -f -e trace=execve -o "$out/$name.build.execve.log" gcc -O1 -g -w -fsanitize=address,undefined -fno-omit-frame-pointer -L "$libdir" -I "$ROOT/runtime" -o "$out/$name" "$out/$name.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}" > "$out/$name.build.log" 2>&1
 if timeout 60 "$out/$name" > "$out/$name.stdout" 2> "$out/$name.stderr";then rc=0;else rc=$?;fi
 cat "$out/$name.stdout" "$out/$name.stderr"
 printf '%s_PROGRAM_EXIT=%s\n' "$name" "$rc"
 if diff -u "$ROOT/tests/small-leaks/$name.expected" "$out/$name.stdout" > "$out/$name.diff";then diff_rc=0;else diff_rc=$?;fi
 printf '%s_ORACLE_DIFF_EXIT=%s\n' "$name" "$diff_rc"
 if [[ "$rc" == 0 && "$diff_rc" == 0 && ! -s "$out/$name.stderr" ]];then
  pass=$((pass+1))
 elif [[ "$mode" == after ]];then exit 1;fi
done
printf 'SMALL_LEAKS_PASS=%s TOTAL=2 MODE=%s\n' "$pass" "$mode"
[[ "$mode" == before || "$pass" == 2 ]]
