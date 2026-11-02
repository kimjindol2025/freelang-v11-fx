#!/usr/bin/env bash
set -Eeuo pipefail
out="${L08_HTTP_LOG_OUT:?set an output directory}"
server="${L08_HTTP_LOG_SERVER:?set the native server}"
root="${L08_HTTP_LOG_ROOT:?set the isolated fixture root}"
mkdir -p "$out"
L08_PORT_FILE="$out/port" FL_FILE_BASE="$root" FL_DEBUG=2 FL_LOG_FILE="$out/debug.log" "$server" "$root" > "$out/server.stdout" 2> "$out/server.stderr" &
pid=$!
trap 'kill "$pid" 2>/dev/null || true;wait "$pid" 2>/dev/null || true' EXIT
for attempt in {1..200};do [[ -s "$out/port" ]] && break;sleep .05;done
[[ -s "$out/port" ]]
port=$(cat "$out/port")
curl -q --noproxy '*' -fsS --max-time 10 "http://127.0.0.1:$port/param/HTTP_C14_PRIVATE_MARKER" -o "$out/path-body"
curl -q --noproxy '*' -fsS --max-time 10 "http://127.0.0.1:$port/echo" -H 'Content-Type: application/json' --data-binary '{"value":"HTTP_C14_PRIVATE_MARKER"}' -o "$out/json-body"
kill "$pid";wait "$pid";trap - EXIT
if rg -q HTTP_C14_PRIVATE_MARKER "$out/debug.log";then echo HTTP_LOG_REDACTION=FAIL;exit 1;fi
if ! rg -q '/param/:id' "$out/debug.log";then echo HTTP_LOG_ROUTE_PATTERN=MISSING;exit 1;fi
echo HTTP_LOG_REDACTION=PASS
