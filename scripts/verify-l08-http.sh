#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_OUT:-$(mktemp -d)}"
mkdir -p "$out"
[[ -x "$out/compiler" ]] || l07_build_compiler "$out" >"$out/build.log" 2>&1
"$out/compiler" "$ROOT/tests/l08/http.fl" "$out/http.c" >"$out/http.build.log" 2>&1
gcc -O2 -w -I "$ROOT/runtime" -o "$out/http" "$out/http.c" "$ROOT/tests/l08/http-port.c" \
    "${L07_SRCS[@]}" -Wl,--wrap=listen "${L07_LIBS[@]}" >>"$out/http.build.log" 2>&1
L08_PORT_FILE="$out/http.port" "$out/http" >"$out/http.stdout" 2>"$out/http.stderr" &
server_pid=$!
trap 'kill "$server_pid" 2>/dev/null || true; wait "$server_pid" 2>/dev/null || true' EXIT
for attempt in {1..200}; do [[ -s "$out/http.port" ]] && break; sleep .05; done
[[ -s "$out/http.port" ]]
port=$(cat "$out/http.port")
request() {
    local label="$1" method="$2" path="$3" status
    status=$(curl --noproxy '*' -sS --max-time 5 -X "$method" -o "$out/response" -w '%{http_code}' "http://127.0.0.1:$port$path")
    printf '%s=%s:%s\n' "$label" "$status" "$(cat "$out/response")"
}
{
    request GET_NAMED GET /named
    request GET_SAVED GET /saved
    request POST_NAMED POST /post
    request GET_UNDERSCORE GET /underscore
    request GET_ALIAS GET /alias
    request GET_LATE GET /late
    request GET_PROTOCOL GET /protocol
} >"$out/http.actual"
cat "$out/http.actual"
diff -u "$ROOT/tests/l08/http.expected" "$out/http.actual"
echo L08_HTTP_PASS=7
