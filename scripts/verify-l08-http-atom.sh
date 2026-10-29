#!/usr/bin/env bash
set -Eeuo pipefail
umask 077
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_HTTP_ATOM_OUT:-$(mktemp -d)}"
mkdir -p "$out/requests" "$out/root/tmp"
compiler="${L08_HTTP_ATOM_COMPILER:-$out/compiler}"
if [[ -z "${L08_HTTP_ATOM_COMPILER:-}" ]];then l07_build_compiler "$out" > "$out/compiler-build.log" 2>&1;fi
runtime_root="${L08_HTTP_ATOM_RUNTIME_ROOT:-$ROOT}"
sources=()
for unit in "${L07_RUNTIME[@]}";do sources+=("$runtime_root/runtime/$unit.c");done
"$compiler" --extern http-atom-begin --extern http-atom-end --extern http-atom-pause --extern http-atom-stats "$ROOT/tests/l08-platform/http-atom.fl" "$out/server.c" > "$out/compile.log" 2>&1
flags=(-O1 -g -w -D_GNU_SOURCE -I "$runtime_root/runtime" -include "$ROOT/tests/l08-platform/http-atom.h")
if [[ -n "${L08_HTTP_ATOM_SANITIZER_LIBDIR:-}" ]];then
 flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer -L "$L08_HTTP_ATOM_SANITIZER_LIBDIR")
 export LD_LIBRARY_PATH="$L08_HTTP_ATOM_SANITIZER_LIBDIR${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
 export ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1
fi
gcc "${flags[@]}" -o "$out/server" "$out/server.c" "$ROOT/tests/l08-platform/http-atom.c" \
 "$ROOT/tests/l08/http-port.c" "$ROOT/tests/l08-platform/http-loopback.c" \
 -Wl,--wrap=listen -Wl,--wrap=bind "${sources[@]}" "${L07_LIBS[@]}" > "$out/build.log" 2>&1
pid=""
stop(){
 [[ -n "$pid" ]] || return 0
 kill -TERM "$pid" 2>/dev/null || true
 for attempt in {1..200};do
  state=$(ps -p "$pid" -o stat= 2>/dev/null || true)
  [[ -n "$state" && "$state" != Z* ]] || break
  sleep .05
 done
 if [[ -n "$state" && "$state" != Z* ]];then echo HTTP_ATOM_OWN_STOP_TIMEOUT=FAIL;kill -KILL "$pid" 2>/dev/null || true;fi
 if wait "$pid";then rc=0;else rc=$?;fi
 pid="";printf 'HTTP_ATOM_SERVER_EXIT=%s\n' "$rc";[[ "$rc" == 0 ]]
}
trap 'stop || true' EXIT
start(){
 local label="$1"
 FL_WORKERS=4 FL_FILE_BASE="$out/root" TMPDIR="$out/root/tmp" L08_PORT_FILE="$out/$label.port" \
  "$out/server" > "$out/$label.stdout" 2> "$out/$label.stderr" &
 pid=$!
 for attempt in {1..200};do [[ ! -s "$out/$label.port" ]] || break;kill -0 "$pid";sleep .05;done
 port=$(cat "$out/$label.port")
 [[ $(curl --noproxy '*' -fsS --max-time 5 "http://127.0.0.1:$port/health") == OK ]]
}
start first
[[ $(curl --noproxy '*' -fsS --max-time 5 "http://127.0.0.1:$port/count") == 0 ]]
export HTTP_ATOM_PORT="$port" HTTP_ATOM_REQUESTS="$out/requests"
seq 1 1000 | xargs -P10 -I '{}' bash -c '
 curl --noproxy "*" -fsS --max-time 10 -X POST -d "" \
  -o "$HTTP_ATOM_REQUESTS/$1.body" -w "%{http_code}\n" \
  "http://127.0.0.1:$HTTP_ATOM_PORT/increment" > "$HTTP_ATOM_REQUESTS/$1.status"
' _ '{}'
curl --noproxy '*' -fsS --max-time 5 "http://127.0.0.1:$port/stats" > "$out/stats.json"
{
 printf 'HTTP_ATOM_STATUS=%s\n' "$(sort -u "$out/requests/"*.status | paste -sd ',' -)"
 printf 'HTTP_ATOM_RESPONSES=%s\n' "$(find "$out/requests" -name '*.status' | wc -l)"
 count=$(curl --noproxy '*' -fsS --max-time 5 "http://127.0.0.1:$port/count")
 printf 'HTTP_ATOM_COUNTER=%s\n' "$count"
 printf 'HTTP_ATOM_UNIQUE_VALUES=%s\n' "$(jq -s 'map(.value)|unique|length' "$out/requests/"*.body)"
 printf 'HTTP_ATOM_WORKERS=%s\n' "$(jq -r '.workers' "$out/stats.json")"
 printf 'HTTP_ATOM_PEAK=%s\n' "$(jq -r '.peak' "$out/stats.json")"
 printf 'HTTP_ATOM_ACTIVE=%s\n' "$(jq -r '.active' "$out/stats.json")"
 if jq -se 'map(.value)|sort == [range(1;1001)]' "$out/requests/"*.body > "$out/exact-values.json";then echo HTTP_ATOM_EXACT_VALUES=PASS;else echo HTTP_ATOM_EXACT_VALUES=FAIL;fi
 reads=PASS
 for iteration in {1..10};do [[ $(curl --noproxy '*' -fsS --max-time 5 "http://127.0.0.1:$port/count") == 1000 ]] || reads=FAIL;done
 printf 'HTTP_ATOM_REPEATED_READS=%s\n' "$reads"
} > "$out/results"
stop
start second
printf 'HTTP_ATOM_VOLATILE_RESTART_COUNTER=%s\n' "$(curl --noproxy '*' -fsS --max-time 5 "http://127.0.0.1:$port/count")" >> "$out/results"
stop
cat "$out/results"
head -n -1 "$ROOT/tests/l08-platform/http-atom.expected" > "$out/expected-results"
diff -u "$out/expected-results" "$out/results"
echo HTTP_ATOM_SELF=PASS
