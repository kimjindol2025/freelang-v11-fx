#!/usr/bin/env bash
set -Eeuo pipefail
umask 077
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_BLOG_EDITOR_OUT:?}"
blog="${L08_BLOG_SOURCE_ROOT:?source-only approved copy required}"
compiler="${L08_BLOG_NATIVE_COMPILER:?}"
[[ -f "$blog/app/main.fl" && ! -e "$blog/data/blog.wal" ]]
mkdir -p "$out/root"
for name in main b1-editor-seed editor-http;do
 directory=tests;[[ "$name" != main ]] || directory=app
 (cd "$blog/$directory";"$compiler" "$name.fl" "$out/$name.c") > "$out/$name-build.log" 2>&1
 link=()
 if [[ "$name" == main ]];then link=("$ROOT/tests/l08/http-port.c" "$ROOT/tests/l08-platform/http-loopback.c" -Wl,--wrap=listen -Wl,--wrap=bind);fi
 gcc -O1 -w -I "$ROOT/runtime" -o "$out/$name" "$out/$name.c" "${link[@]}" "${L07_SRCS[@]}" "${L07_LIBS[@]}" >> "$out/$name-build.log" 2>&1
 printf '%s_NATIVE_BUILD_EXIT=0\n' "$name"
done
# Generate synthetic secrets inside this shell, never in execve arguments/logs.
export BLOG_TEST_SESSION_TOKEN="$(openssl rand -hex 32)"
export BLOG_CSRF_SECRET="$(openssl rand -hex 32)"
export FL_FILE_BASE="$out/root" AFJ_DB_WAL="$out/root/blog.wal" AFJ_DB_MODE=development AFJ_DB_BIND_HOST=127.0.0.1
export TMPDIR="$out/root/tmp"
export BLOG_PORT=0 BLOG_COOKIE_SECURE=0
if (cd "$out/root";timeout 60 "$out/b1-editor-seed") > "$out/seed.stdout" 2> "$out/seed.stderr";then rc=0;else rc=$?;fi
cat "$out/seed.stdout" "$out/seed.stderr";printf 'EDITOR_SEED_EXIT=%s\n' "$rc"
[[ "$rc" == 0 && ! -s "$out/seed.stderr" ]]
[[ $(tail -1 "$out/seed.stdout") == 'PASS editor-session-seed' ]]
pid=""
stop(){
 [[ -n "$pid" ]] || return 0
 kill -TERM "$pid" 2>/dev/null || true
 for attempt in {1..200};do
  kill -0 "$pid" 2>/dev/null || break
  state=$(ps -p "$pid" -o stat= 2>/dev/null || true);[[ "$state" != Z* ]] || break
  sleep .05
 done
 if kill -0 "$pid" 2>/dev/null && [[ ${state:-} != Z* ]];then
  echo OWN_BLOG_SHUTDOWN_TIMEOUT=FAIL;kill -KILL "$pid" 2>/dev/null || true;wait "$pid" 2>/dev/null || true;pid="";return 1
 fi
 if wait "$pid";then rc=0;else rc=$?;fi
 pid="";printf 'OWN_BLOG_SERVER_EXIT=%s\n' "$rc";[[ "$rc" == 0 ]]
}
trap 'stop || true' EXIT
for iteration in 1 2 3;do
 export L08_PORT_FILE="$out/port-$iteration"
 (cd "$out/root";exec "$out/main") > "$out/server-$iteration.stdout" 2> "$out/server-$iteration.stderr" &
 pid=$!
 for attempt in {1..300};do
  [[ -s "$L08_PORT_FILE" ]] && break
  kill -0 "$pid" 2>/dev/null || break
  sleep .05
 done
 if [[ ! -s "$L08_PORT_FILE" ]];then echo NATIVE_BLOG_START=FAIL;tail -10 "$out/server-$iteration.stderr";exit 1;fi
 port=$(cat "$L08_PORT_FILE");export BLOG_TEST_BASE_URL="http://127.0.0.1:$port"
 if timeout 60 "$out/editor-http" > "$out/client-$iteration.stdout" 2> "$out/client-$iteration.stderr";then rc=0;else rc=$?;fi
 cat "$out/client-$iteration.stdout" "$out/client-$iteration.stderr"
 printf 'EDITOR_HTTP_%s_EXIT=%s\n' "$iteration" "$rc"
 [[ "$rc" == 0 && ! -s "$out/client-$iteration.stderr" ]]
 diff -u "$ROOT/tests/l08-platform/blog-editor-native.expected" "$out/client-$iteration.stdout"
 printf 'HOME_LOGIN_SCRIPT_EDITOR_HTTP_200_%s=PASS\n' "$iteration"
 stop
 if [[ "$iteration" == 1 ]];then cp "$AFJ_DB_WAL" "$out/after-first-start.wal";else cmp "$AFJ_DB_WAL" "$out/after-first-start.wal";fi
done
echo NATIVE_BLOG_RESTARTS=2
echo NATIVE_BLOG_WAL_RESTART_UNCHANGED=PASS
echo 'NATIVE_BLOG_EDITOR_PASS=3 FAIL=0'
echo AUTHENTICATION_METHOD=SYNTHETIC_PRESEEDED_SESSION
echo PASSWORD_LOGIN=UNEXECUTED
echo EXISTING_EDITOR_JS=SERVED_NOT_EXECUTED
