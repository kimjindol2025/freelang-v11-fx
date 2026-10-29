#!/usr/bin/env bash
set -Eeuo pipefail
umask 077
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
blog="${L08_BLOG_SOURCE_ROOT:?approved source-only copy required}"
compiler="${L08_BLOG_NATIVE_COMPILER:?}"
out="${L08_BLOG_HTTP_OUT:-$(mktemp -d)}"
[[ -f "$blog/app/store.fl" && ! -e "$blog/data/blog.wal" ]]
mkdir -p "$out/source/tests" "$out/root/tmp" "$out/requests" "$out/rejected"
ln -s "$blog/app" "$out/source/app"
ln -s "$blog/src" "$out/source/src"
cp "$ROOT/tests/l08-platform/store-http.fl" "$out/source/tests/server.fl"
(cd "$out/source/tests";"$compiler" --extern http-atom-begin --extern http-atom-end --extern http-atom-stats server.fl "$out/server.c") > "$out/build.log" 2>&1
flags=(-O1 -g -w -D_GNU_SOURCE -I "$ROOT/runtime" -include "$ROOT/tests/l08-platform/http-atom.h")
if [[ -n "${L08_BLOG_HTTP_SANITIZER_LIBDIR:-}" ]];then
 flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer -L "$L08_BLOG_HTTP_SANITIZER_LIBDIR")
 export LD_LIBRARY_PATH="$L08_BLOG_HTTP_SANITIZER_LIBDIR" ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1
fi
gcc "${flags[@]}" -o "$out/server" "$out/server.c" "$ROOT/tests/l08-platform/http-atom.c" "$ROOT/tests/l08/http-port.c" "$ROOT/tests/l08-platform/http-loopback.c" -Wl,--wrap=listen,--wrap=bind "${L07_SRCS[@]}" "${L07_LIBS[@]}" >> "$out/build.log" 2>&1
pid=""
stderr_allowed='^\[http\] (라우트 등록: (GET|POST) /(health|rows|stats|write) → <closure>|스레드 풀: 4 워커, 큐 1024, 타임아웃 30s|서버 시작: http://0\.0\.0\.0:0|graceful shutdown \(최대 30s 대기\)\.\.\.|종료 완료 \(처리 중 연결: 0\))$'
stop(){
 [[ -n "$pid" ]] || return 0
 kill -TERM "$pid" 2>/dev/null || true
 for attempt in {1..200};do
  state=$(ps -p "$pid" -o stat= 2>/dev/null || true)
  [[ -n "$state" && "$state" != Z* ]] || break
  sleep .05
 done
 if [[ -n "$state" && "$state" != Z* ]];then echo DB_HTTP_OWN_STOP_TIMEOUT=FAIL;kill -KILL "$pid";wait "$pid" || true;pid="";return 1;fi
 if wait "$pid";then rc=0;else rc=$?;fi
 pid="";printf 'DB_HTTP_SERVER_EXIT=%s\n' "$rc";[[ "$rc" == 0 ]]
 if rg -n -v "$stderr_allowed" "$current_stderr";then return 1;fi
}
trap 'stop || true' EXIT
start(){
 local iteration="$1"
 current_stderr="$out/$iteration.stderr"
 (cd "$out/root";exec env FL_WORKERS=4 FL_FILE_BASE="$out/root" TMPDIR="$out/root/tmp" AFJ_DB_MODE=development AFJ_DB_BIND_HOST=127.0.0.1 AFJ_DB_WAL="$out/root/blog.wal" L08_PORT_FILE="$out/$iteration.port" "$out/server") > "$out/$iteration.stdout" 2> "$out/$iteration.stderr" &
 pid=$!
 for attempt in {1..300};do [[ ! -s "$out/$iteration.port" ]] || break;kill -0 "$pid";sleep .05;done
 port=$(cat "$out/$iteration.port")
 [[ $(curl --noproxy '*' -fsS --max-time 5 "http://127.0.0.1:$port/health") == OK ]]
 if rg -n -v "$stderr_allowed" "$current_stderr";then return 1;fi
}
exact_rows(){
 curl --noproxy '*' -fsS --max-time 10 "http://127.0.0.1:$port/rows" > "$out/$1.json"
 jq -e 'length == 32 and (map(.id)|sort) == ([range(1;33)|tostring]|sort) and all(.title == "한🙂")' "$out/$1.json" > /dev/null
}
start first
export DB_HTTP_PORT="$port" DB_HTTP_REQUESTS="$out/requests"
seq 1 32 | xargs -P10 -I '{}' bash -c 'curl --noproxy "*" -sS --max-time 15 -H "Content-Type: application/json" -d "{\"id\":\"$1\"}" -o "$DB_HTTP_REQUESTS/$1.body" -w "%{http_code}\n" "http://127.0.0.1:$DB_HTTP_PORT/write" > "$DB_HTTP_REQUESTS/$1.status"' _ '{}'
curl --noproxy '*' -fsS --max-time 5 "http://127.0.0.1:$port/stats" > "$out/stats.json"
exact_rows live
cp "$out/root/blog.wal" "$out/accepted.wal"
{
 printf 'DB_HTTP_WRITE_STATUS=%s\n' "$(sort -u "$out/requests/"*.status | paste -sd ',' -)"
 printf 'DB_HTTP_WRITE_RESPONSES=%s\n' "$(find "$out/requests" -name '*.status' | wc -l)"
 for field in workers peak active;do printf 'DB_HTTP_%s=%s\n' "${field^^}" "$(jq -r ".$field" "$out/stats.json")";done
 printf 'DB_HTTP_LIVE_ROWS=%s\n' "$(jq length "$out/live.json")"
 echo DB_HTTP_LIVE_EXACT=PASS
} > "$out/result"
export DB_HTTP_REQUESTS="$out/rejected"
seq 1 8 | xargs -P8 -I '{}' bash -c 'curl --noproxy "*" -sS --max-time 15 -H "Content-Type: application/json" -d "{\"id\":\"1\"}" -o "$DB_HTTP_REQUESTS/$1.body" -w "%{http_code}\n" "http://127.0.0.1:$DB_HTTP_PORT/write" > "$DB_HTTP_REQUESTS/$1.status"' _ '{}'
printf 'DB_HTTP_DUPLICATE_STATUS=%s\n' "$(sort -u "$out/rejected/"*.status | paste -sd ',' -)" >> "$out/result"
cmp "$out/root/blog.wal" "$out/accepted.wal"
exact_rows rejected
echo DB_HTTP_REJECT_WAL_UNCHANGED=PASS >> "$out/result"
stop
for iteration in 1 2;do
 start "$iteration"
 exact_rows "restarted-$iteration"
 printf 'DB_HTTP_RESTART_%s_ROWS=%s\n' "$iteration" "$(jq length "$out/restarted-$iteration.json")" >> "$out/result"
 stop
 cmp "$out/root/blog.wal" "$out/accepted.wal"
 printf 'DB_HTTP_RESTART_%s_WAL_UNCHANGED=PASS\n' "$iteration" >> "$out/result"
done
echo DB_HTTP_RESTARTS=2 >> "$out/result"
cat "$out/result"
diff -u "$ROOT/tests/l08-platform/store-http.expected" "$out/result"
echo DB_HTTP_SELF_EXIT=0
echo PASSWORD_LOGIN=UNEXECUTED
