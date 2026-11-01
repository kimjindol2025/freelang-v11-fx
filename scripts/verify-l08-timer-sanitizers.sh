#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_TIMER_SANITIZER_OUT:?}"
libdir="${L08_TIMER_SANITIZER_LIBDIR:?}"
native="${L08_TIMER_NATIVE_OUT:?}"
mkdir -p "$out"
[[ -r "$libdir/libasan.so" && -r "$libdir/libubsan.so" && -x "$native/compiler" ]]
export LD_LIBRARY_PATH="$libdir${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
# Check invalid accesses, undefined behavior, and leaks outside ptrace.
export ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1
flags=(-O1 -g -w -fsanitize=address,undefined -fno-omit-frame-pointer -L "$libdir" -I "$ROOT/runtime")
pass=0;fail=0
for name in timer-memory timer-serialization timer-arena-oom;do
 link=();[[ "$name" != timer-arena-oom ]] || link=(-Wl,--wrap=calloc)
 gcc "${flags[@]}" -o "$out/$name" "$ROOT/tests/l08-platform/$name.c" "${L07_SRCS[@]}" "${link[@]}" "${L07_LIBS[@]}" > "$out/$name-build.log" 2>&1
 if timeout 30 "$out/$name" > "$out/$name.actual" 2> "$out/$name.stderr";then rc=0;else rc=$?;fi
 cat "$out/$name.actual" "$out/$name.stderr"
 printf '%s_SANITIZED_EXIT=%s\n' "$name" "$rc"
 if [[ "$rc" == 0 && ! -s "$out/$name.stderr" ]];then pass=$((pass+1));else fail=$((fail+1));fi
done
for name in timer timer-http;do
 "$native/compiler" "$ROOT/tests/l08-platform/$name.fl" "$out/$name.c" > "$out/$name-build.log" 2>&1
 link=();[[ "$name" != timer-http ]] || link=("$ROOT/tests/l08/http-port.c" -Wl,--wrap=listen)
 gcc "${flags[@]}" -o "$out/$name" "$out/$name.c" "${L07_SRCS[@]}" "${link[@]}" "${L07_LIBS[@]}" >> "$out/$name-build.log" 2>&1
done
if timeout 30 "$out/timer" > "$out/timer.actual" 2> "$out/timer.stderr";then rc=0;else rc=$?;fi
cat "$out/timer.actual" "$out/timer.stderr"
printf 'TIMER_CLI_SANITIZED_EXIT=%s\n' "$rc"
if [[ "$rc" == 0 && ! -s "$out/timer.stderr" ]] && diff -u "$ROOT/tests/l08-platform/timer.expected" "$out/timer.actual";then pass=$((pass+1));else fail=$((fail+1));fi
if L08_TIMER_NO_VISIT_OUT="$out/http" L08_TIMER_NO_VISIT_SERVER="$out/timer-http" bash "$ROOT/scripts/verify-l08-timer-no-visit.sh";then pass=$((pass+1));else fail=$((fail+1));fi
printf 'L08_TIMER_SANITIZER_PASS=%s FAIL=%s\n' "$pass" "$fail"
[[ "$fail" == 0 ]]
echo TIMER_LEAK_CHECK=PASS
