#!/usr/bin/env bash
set -Eeuo pipefail
out="${L08_TIMER_NO_VISIT_OUT:?}"
server="${L08_TIMER_NO_VISIT_SERVER:?}"
mkdir -p "$out/root"
L08_PORT_FILE="$out/port" FL_WORKERS=4 FL_FILE_BASE="$out/root" "$server" "$out/root" > "$out/server.stdout" 2> "$out/server.stderr" &
pid=$!
trap 'kill "$pid" 2>/dev/null || true;wait "$pid" 2>/dev/null || true' EXIT
for attempt in {1..200};do [[ -s "$out/port" ]] && break;sleep .05;done
port=$(cat "$out/port")
[[ ! -e "$out/root/fired.txt" ]]
code=$(curl -q --noproxy '*' -sS --max-time 5 -o "$out/response" -w '%{http_code}' "http://127.0.0.1:$port/schedule" -H 'Content-Type: application/json' --data-binary '{"label":"captured-한😀"}')
[[ "$code" == 200 && $(cat "$out/response") == SCHEDULED ]]
# No HTTP request after scheduling: inspect only the independent filesystem effect.
for attempt in {1..300};do [[ -f "$out/root/fired.txt" ]] && break;sleep .01;done
printf 'captured-한😀' > "$out/expected-label"
cmp "$out/expected-label" "$out/root/fired.txt"
echo TIMER_WITHOUT_FURTHER_HTTP=PASS
kill "$pid";wait "$pid";trap - EXIT
