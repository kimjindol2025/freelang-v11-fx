#!/usr/bin/env bash
set -Eeuo pipefail
umask 077
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
compiler="${L01_TARGET_COMPILER:?verified native target compiler required}"
scenario="${L01_TARGET_SCENARIO:-semantics}"
case "$scenario" in
 semantics) entry=browser-entry.fl;server=browser-server.fl;proof=browser-proof.c;expected=browser-semantics.expected;group=SEMANTICS;;
 events) entry=browser-events.fl;server=browser-events-server.fl;proof=browser-events-proof.c;expected=browser-events.expected;group=EVENT;;
 dom) entry=browser-dom.fl;server=browser-events-server.fl;proof=browser-dom-proof.c;expected=browser-dom.expected;group=DOM;;
 blog-model) entry=tests/block-model-browser.fl;server=browser-server.fl;proof=browser-proof.c;expected=block-model.expected;group=BLOCK_MODEL;;
 *) echo L01_TARGET_SCENARIO=INVALID >&2;exit 2;;
esac
out="${L01_TARGET_OUT:-$(mktemp -d)}"
mkdir -p "$out/source" "$out/root" "$out/profile"
cp "$ROOT/tests/l01/"*.fl "$out/source/"
expected_file="$ROOT/tests/l01/$expected"
if [[ "$scenario" == blog-model ]];then
 blog="${L01_TARGET_BLOG_SOURCE:?approved source-only blog required}"
 [[ -f "$blog/web/block-model.fl" && ! -e "$blog/data/blog.wal" ]]
 mkdir -p "$out/source/tests" "$out/source/web"
 cp "$blog/web/block-model.fl" "$out/source/web/"
 cp "$blog/tests/"block-model*.fl "$out/source/tests/"
 expected_file="$blog/tests/block-model.expected"
fi
bash "$ROOT/scripts/refresh-browser-core.sh" --check
sha256sum "$compiler" "$ROOT/self/browser-core.fl" "$out/source/browser-semantics.fl" "$out/source/$entry"
"$compiler" --browser-js "$out/source/$entry" "$out/root/program.js" > "$out/generate.log" 2>&1
cat "$out/generate.log"
"$compiler" --browser-js "$out/source/$entry" "$out/again.js" > "$out/generate-again.log" 2>&1
cmp "$out/root/program.js" "$out/again.js"
echo L01_GENERATION_REPRODUCIBLE=PASS
sha256sum "$out/root/program.js"
failed=0;passed=0
for spec in private private-alias private-definition native eval parameters bindings recur;do
 file="$out/source/reject-$spec.fl";before=$(sha256sum "$file" | cut -d' ' -f1)
 if "$compiler" --browser-js "$file" "$out/reject-$spec.js" > "$out/reject-$spec.actual" 2> "$out/reject-$spec.stderr";then rc=0;else rc=$?;fi
 cat "$out/reject-$spec.actual" "$out/reject-$spec.stderr"
 printf 'L01_REJECT_CASE=%s EXIT=%s\n' "$spec" "$rc"
 if [[ "$rc" == 1 && ! -e "$out/reject-$spec.js" && "$before" == "$(sha256sum "$file" | cut -d' ' -f1)" ]];then passed=$((passed+1));else failed=$((failed+1));fi
done
for spec in extension same-source;do
 input="$out/source/browser-entry.fl";before=$(sha256sum "$input" | cut -d' ' -f1)
 output="$out/invalid.txt"
 [[ "$spec" != same-source ]] || { output="$out/alias.js";ln -s "$input" "$output"; }
 if "$compiler" --browser-js "$input" "$output" > "$out/$spec.actual" 2> "$out/$spec.stderr";then rc=0;else rc=$?;fi
 cat "$out/$spec.actual" "$out/$spec.stderr"
 printf 'L01_REJECT_CASE=%s EXIT=%s\n' "$spec" "$rc"
 if [[ "$rc" == 1 && "$before" == "$(sha256sum "$input" | cut -d' ' -f1)" ]] && { [[ "$spec" == same-source && -L "$output" ]] || [[ "$spec" == extension && ! -e "$output" ]]; };then passed=$((passed+1));else failed=$((failed+1));fi
done
printf 'L01_COMPILER_REJECT_PASS=%s FAIL=%s\n' "$passed" "$failed"
[[ "$failed" == 0 ]]
specs=(native server)
[[ "${L01_TARGET_GENERATION_ONLY:-0}" != 1 ]] || specs=(native)
for spec in "${specs[@]}";do
 input="$out/source/browser-native.fl";[[ "$spec" != server ]] || input="$out/source/$server"
 "$compiler" "$input" "$out/$spec.c" > "$out/$spec.build.log" 2>&1
 support=();[[ "$spec" != server ]] || support=("$ROOT/tests/l08/http-port.c" "$ROOT/tests/l08-platform/http-loopback.c" -Wl,--wrap=listen,--wrap=bind)
 gcc -O1 -g -w -I "$ROOT/runtime" -o "$out/$spec" "$out/$spec.c" "${support[@]}" "${L07_SRCS[@]}" "${L07_LIBS[@]}" >> "$out/$spec.build.log" 2>&1
done
"$out/native" > "$out/native.actual" 2> "$out/native.stderr"
cat "$out/native.actual" "$out/native.stderr"
[[ ! -s "$out/native.stderr" ]]
diff -u "$ROOT/tests/l01/browser-semantics.expected" "$out/native.actual"
echo L01_NATIVE_ORACLE=PASS
if [[ "${L01_TARGET_GENERATION_ONLY:-0}" == 1 ]];then
 echo L01_BROWSER_EXECUTION=UNEXECUTED_GENERATION_ONLY
 echo L01_TARGET_NATIVE_GATE=PASS
 exit 0
fi
gcc -O1 -g -w -D_GNU_SOURCE -I "$ROOT/runtime" -o "$out/proof" "$ROOT/tests/l01/$proof" "${L07_SRCS[@]}" "${L07_LIBS[@]}" > "$out/proof.build.log" 2>&1
server_pid="";browser_pid=""
stop(){
 for kind in browser server;do
  if [[ "$kind" == browser ]];then process="$browser_pid";else process="$server_pid";fi
  [[ -n "$process" ]] || continue
  kill -TERM "$process" 2>/dev/null || true
  for attempt in {1..200};do state=$(ps -p "$process" -o stat= 2>/dev/null || true);[[ -n "$state" && "$state" != Z* ]] || break;sleep .05;done
  if [[ -n "${state:-}" && "$state" != Z* ]];then echo "L01_OWN_${kind}_STOP_TIMEOUT=FAIL";kill -KILL "$process" 2>/dev/null || true;fi
  if wait "$process";then status=0;else status=$?;fi
  echo "L01_OWN_${kind}_EXIT=$status"
 done
}
trap stop EXIT
export FL_FILE_BASE="$out/root" L08_PORT_FILE="$out/port" FL_WORKERS=4
(cd "$out/root";exec "$out/server") > "$out/server.stdout" 2> "$out/server.stderr" &
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
if timeout 90 "$out/proof" "$ws" "$base" "$out" > "$out/browser.actual" 2> "$out/browser-error.log";then rc=0;else rc=$?;fi
cat "$out/browser.actual" "$out/browser-error.log"
printf 'L01_BROWSER_PROOF_EXIT=%s\n' "$rc"
[[ "$rc" == 0 && ! -s "$out/browser-error.log" ]]
rg '^(J_|BM_)' "$out/browser.actual" > "$out/browser.result"
passed=$(rg -c '=PASS$' "$out/browser.result" || true)
failed=$(rg -c '=FAIL$' "$out/browser.result" || true)
printf 'L01_BROWSER_%s_PASS=%s FAIL=%s\n' "$group" "${passed:-0}" "${failed:-0}"
diff -u "$expected_file" "$out/browser.result"
echo L01_TARGET_SELF=PASS
