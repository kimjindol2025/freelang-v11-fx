#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_TIMER_OUT:-$(mktemp -d)}"
mkdir -p "$out/root"
l07_build_compiler "$out" > "$out/compiler-build.log" 2>&1
for name in timer timer-exit;do l07_build_app "$out" "$ROOT/tests/l08-platform/$name.fl" "$name" > "$out/$name-build.log" 2>&1;done
pass=0;fail=0
if timeout 10 "$out/timer" > "$out/timer.actual" 2> "$out/timer.stderr";then rc=0;else rc=$?;fi
cat "$out/timer.actual" "$out/timer.stderr"
printf 'TIMER_PROGRAM_EXIT=%s\n' "$rc"
if [[ "$rc" == 0 && ! -s "$out/timer.stderr" ]] && diff -u "$ROOT/tests/l08-platform/timer.expected" "$out/timer.actual";then pass=$((pass+1));else fail=$((fail+1));fi
if timeout 10 "$out/timer-exit" > "$out/exit.actual" 2> "$out/exit.stderr";then rc=0;else rc=$?;fi
printf 'TIMER_EXPLICIT_EXIT=%s\n' "$rc"
if [[ "$rc" == 7 && ! -s "$out/exit.actual" && ! -s "$out/exit.stderr" ]];then echo EXIT_CANCELS_TIMER=PASS;pass=$((pass+1));else fail=$((fail+1));fi
"$out/compiler" "$ROOT/tests/l08-platform/timer-http.fl" "$out/server.c" > "$out/server-build.log" 2>&1
gcc -O2 -w -I "$ROOT/runtime" -o "$out/server" "$out/server.c" "$ROOT/tests/l08/http-port.c" "${L07_SRCS[@]}" -Wl,--wrap=listen "${L07_LIBS[@]}" >> "$out/server-build.log" 2>&1
L08_PORT_FILE="$out/port" FL_WORKERS=4 FL_FILE_BASE="$out/root" "$out/server" "$out/root" > "$out/server.stdout" 2> "$out/server.stderr" &
pid=$!
trap 'kill "$pid" 2>/dev/null || true;wait "$pid" 2>/dev/null || true' EXIT
for attempt in {1..200};do [[ -s "$out/port" ]] && break;sleep .05;done
port=$(cat "$out/port");base="http://127.0.0.1:$port"
code=$(curl -q --noproxy '*' -sS --max-time 5 -o "$out/scheduled" -w '%{http_code}' "$base/schedule" -H 'Content-Type: application/json' --data-binary '{"label":"captured-한😀"}')
[[ "$code" == 200 && $(cat "$out/scheduled") == SCHEDULED ]]
for attempt in {1..32};do curl -q --noproxy '*' -fsS --max-time 5 "$base/churn" -o "$out/churn";done
for attempt in {1..200};do [[ -f "$out/root/fired.txt" ]] && break;sleep .01;done
printf 'captured-한😀' > "$out/expected-label"
if cmp -s "$out/root/fired.txt" "$out/expected-label";then echo CAPTURED_FILE_AFTER_CHURN=PASS;pass=$((pass+1));else echo CAPTURED_FILE_AFTER_CHURN=FAIL;fail=$((fail+1));fi
curl -q --noproxy '*' -fsS --max-time 5 "$base/result" -o "$out/result"
if [[ $(cat "$out/result") == '["captured-한😀"]' ]];then echo TIMER_ATOM_HTTP=PASS;pass=$((pass+1));else cat "$out/result";fail=$((fail+1));fi
sleep .2
curl -q --noproxy '*' -fsS --max-time 5 "$base/result" -o "$out/result-again"
if cmp -s "$out/result" "$out/result-again";then echo TIMER_ONCE=PASS;pass=$((pass+1));else fail=$((fail+1));fi
kill "$pid";wait "$pid";trap - EXIT
echo TIMER_HTTP_SHUTDOWN=PASS
if L08_TIMER_NO_VISIT_OUT="$out/no-visit" L08_TIMER_NO_VISIT_SERVER="$out/server" bash "$ROOT/scripts/verify-l08-timer-no-visit.sh";then pass=$((pass+1));else fail=$((fail+1));fi
for name in timer-memory timer-serialization timer-arena-oom;do
 flags=();[[ "$name" != timer-arena-oom ]] || flags=(-Wl,--wrap=calloc)
 gcc -O2 -w -I "$ROOT/runtime" -o "$out/$name" "$ROOT/tests/l08-platform/$name.c" "${L07_SRCS[@]}" "${flags[@]}" "${L07_LIBS[@]}" > "$out/$name-build.log" 2>&1
 if timeout 30 "$out/$name" > "$out/$name.actual" 2> "$out/$name.stderr";then rc=0;else rc=$?;fi
 cat "$out/$name.actual" "$out/$name.stderr"
 printf '%s_PROGRAM_EXIT=%s\n' "$name" "$rc"
 if [[ "$rc" == 0 && ! -s "$out/$name.stderr" ]];then pass=$((pass+1));else fail=$((fail+1));fi
done
echo TIMER_ASAN_UBSAN=SEPARATE_DRIVER
printf 'L08_TIMER_PARTIAL_PASS=%s FAIL=%s\n' "$pass" "$fail"
[[ "$fail" == 0 ]]
