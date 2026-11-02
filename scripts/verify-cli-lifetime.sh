#!/usr/bin/env bash
set -Eeuo pipefail
ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)
source "$ROOT/tests/l07/build.sh"
out=${CLI_LIFETIME_OUT:?}
compiler=${CLI_LIFETIME_COMPILER:?}
sanitizers=${CLI_LIFETIME_SANITIZER_LIBDIR:?}
mkdir -p "$out"
export LD_LIBRARY_PATH="$sanitizers${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export ASAN_OPTIONS=detect_leaks=1:halt_on_error=1
export UBSAN_OPTIONS=halt_on_error=1 LSAN_OPTIONS=exitcode=23
for name in atom-snapshot normal uncaught exit; do
 expected_rc=0
 case "$name" in
  atom-snapshot) fixture="$ROOT/tests/l08-platform/atom-snapshot.fl"; expected="$ROOT/tests/l08-platform/atom-snapshot.expected";;
  exit) fixture="$ROOT/tests/l08-platform/timer-exit.fl"; expected="$ROOT/tests/cli-lifetime/exit.expected"; expected_rc=7;;
  *) fixture="$ROOT/tests/cli-lifetime/$name.fl"; expected="$ROOT/tests/cli-lifetime/$name.expected"; [[ "$name" != uncaught ]] || expected_rc=1;;
 esac
 "$compiler" "$fixture" "$out/$name.c" > "$out/$name.compile.log" 2>&1
 strace -f -e trace=execve -o "$out/$name.build.execve.log" gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -L "$sanitizers" -I "$ROOT/runtime" -o "$out/$name" "$out/$name.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}" > "$out/$name.build.log" 2>&1
 if timeout 90 "$out/$name" '한😀' > "$out/$name.actual" 2> "$out/$name.stderr"; then rc=0; else rc=$?; fi
 cat "$out/$name.actual" "$out/$name.stderr"
 printf '%s_EXIT=%s EXPECTED=%s\n' "$name" "$rc" "$expected_rc"
 [[ "$rc" == "$expected_rc" ]]
 diff -u "$expected" "$out/$name.actual"
 if [[ "$name" == uncaught ]]; then
  diff -u "$ROOT/tests/cli-lifetime/uncaught.stderr.expected" "$out/$name.stderr"
 else [[ ! -s "$out/$name.stderr" ]]; fi
 printf '%s=PASS\n' "$name"
done
bash "$ROOT/scripts/audit-l07-execve.sh" "$out/"*.build.execve.log
printf 'CLI_LIFETIME=4/4\n'
