#!/usr/bin/env bash
set -Eeuo pipefail
umask 077
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_EDITOR_UI_OUT:-$(mktemp -d)}"
blog="${L08_BLOG_SOURCE_ROOT:?approved source-only copy required}"
compiler="${L08_BLOG_NATIVE_COMPILER:?native compiler required}"
fixture="${L08_UI_PNG_FIXTURE:-}"
[[ -f "$blog/app/main.fl" && ! -e "$blog/data/blog.wal" ]]
mkdir -p "$out/source/tests" "$out/root/tmp" "$out/profile"
ln -s "$blog/app" "$out/source/app"
ln -s "$blog/src" "$out/source/src"
cp "$ROOT/tests/l08-platform/editor-ui-seed.fl" "$out/source/tests/seed.fl"
if [[ -n "$fixture" ]];then
 [[ -f "$fixture" ]];cp "$fixture" "$out/root/ui-fixture.png"
else
 gcc -O2 -Wall -Wextra "$ROOT/tests/l08-platform/editor-ui-png.c" -o "$out/png-fixture" > "$out/png.build.log" 2>&1
 "$out/png-fixture" "$out/root/ui-fixture.png"
fi
sha256sum "$out/root/ui-fixture.png" > "$out/fixture-before.sha256"
for phase in main seed;do
 if [[ -n "${L08_EDITOR_UI_BIN_DIR:-}" ]];then
  [[ -x "$L08_EDITOR_UI_BIN_DIR/$phase" ]]
  ln -s "$L08_EDITOR_UI_BIN_DIR/$phase" "$out/$phase"
  sha256sum "$L08_EDITOR_UI_BIN_DIR/$phase"
  continue
 fi
 input="$blog/app/main.fl";[[ "$phase" != seed ]] || input="$out/source/tests/seed.fl"
 "$compiler" "$input" "$out/$phase.c" > "$out/$phase.build.log" 2>&1
 support=()
 [[ "$phase" != main ]] || support=("$ROOT/tests/l08/http-port.c" "$ROOT/tests/l08-platform/http-loopback.c" -Wl,--wrap=listen,--wrap=bind)
 gcc -O1 -g -w -I "$ROOT/runtime" -o "$out/$phase" "$out/$phase.c" "${support[@]}" "${L07_SRCS[@]}" "${L07_LIBS[@]}" >> "$out/$phase.build.log" 2>&1
done
gcc -O1 -g -w -D_GNU_SOURCE -I "$ROOT/runtime" -o "$out/client" "$ROOT/tests/l08-platform/editor-ui-cdp.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}" > "$out/client.build.log" 2>&1
export BLOG_TEST_SESSION_TOKEN="$(openssl rand -hex 32)" BLOG_CSRF_SECRET="$(openssl rand -hex 32)"
export FL_FILE_BASE="$out/root" TMPDIR="$out/root/tmp" AFJ_DB_WAL="$out/root/blog.wal" AFJ_DB_MODE=development AFJ_DB_BIND_HOST=127.0.0.1
export BLOG_PORT=0 BLOG_COOKIE_SECURE=0 FL_WORKERS=4 L08_PORT_FILE="$out/port" L08_UI_PNG_FIXTURE=ui-fixture.png
(cd "$out/root";"$out/seed") > "$out/seed.actual" 2> "$out/seed.stderr"
cat "$out/seed.actual" "$out/seed.stderr"
[[ ! -s "$out/seed.stderr" ]] && rg -q '^EDITOR_UI_SEED=PASS$' "$out/seed.actual"
server_pid="";browser_pid=""
stop(){
 for kind in browser server;do
  if [[ "$kind" == browser ]];then process="$browser_pid";else process="$server_pid";fi
  [[ -n "$process" ]] || continue
  kill -TERM "$process" 2>/dev/null || true
  for attempt in {1..200};do state=$(ps -p "$process" -o stat= 2>/dev/null || true);[[ -n "$state" && "$state" != Z* ]] || break;sleep .05;done
  if [[ -n "${state:-}" && "$state" != Z* ]];then echo "EDITOR_UI_OWN_${kind}_STOP_TIMEOUT=FAIL";kill -KILL "$process" 2>/dev/null || true;fi
  if wait "$process";then status=0;else status=$?;fi
  echo "EDITOR_UI_OWN_${kind}_EXIT=$status"
 done
}
trap stop EXIT
(cd "$out/root";exec "$out/main") > "$out/server.stdout" 2> "$out/server.stderr" &
server_pid=$!
for attempt in {1..300};do [[ ! -s "$out/port" ]] || break;kill -0 "$server_pid" 2>/dev/null || break;sleep .05;done
[[ -s "$out/port" ]]
base="http://127.0.0.1:$(cat "$out/port")"
google-chrome --headless=new --disable-gpu --no-first-run --disable-background-networking --disable-component-update --disable-sync --user-data-dir="$out/profile" --remote-debugging-port=0 about:blank > "$out/browser.stdout" 2> "$out/browser.stderr" &
browser_pid=$!
for attempt in {1..300};do [[ ! -s "$out/profile/DevToolsActivePort" ]] || break;kill -0 "$browser_pid" 2>/dev/null || break;sleep .05;done
[[ -s "$out/profile/DevToolsActivePort" ]]
debug_port=$(head -n1 "$out/profile/DevToolsActivePort")
ws=$(curl -q --noproxy '*' -fsS --max-time 10 "http://127.0.0.1:$debug_port/json/list" | jq -r '.[]|select(.type=="page")|.webSocketDebuggerUrl' | head -n1)
[[ "$ws" == ws://127.0.0.1:* ]]
if timeout 120 "$out/client" "$ws" "$base" "$out" > "$out/ui.actual" 2> "$out/ui.stderr";then rc=0;else rc=$?;fi
cat "$out/ui.actual" "$out/ui.stderr"
echo "EDITOR_UI_CLIENT_EXIT=$rc"
[[ "$rc" == 0 && ! -s "$out/ui.stderr" ]]
rg '^EDITOR_UI_[0-9]+_' "$out/ui.actual" > "$out/ui.result"
diff -u "$ROOT/tests/l08-platform/editor-ui.expected" "$out/ui.result"
rg -q '^EDITOR_UI_PASS=27 FAIL=0$' "$out/ui.actual"
for width in 375 768 1440;do
 [[ -s "$out/editor-image-$width.png" ]]
 sha256sum "$out/editor-image-$width.png"
done
sha256sum -c "$out/fixture-before.sha256"
echo EDITOR_UI_SUITE=PASS
