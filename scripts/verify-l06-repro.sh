#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l06/build.sh"
out="${L06_OUT:-$(mktemp -d)}"
mkdir -p "$out"
l07_build_compiler "$out" >"$out/build.log" 2>&1
l06_app "$out" "$ROOT/tests/l06/repro.fl" repro >>"$out/build.log" 2>&1
rm -f "$out/port"
FL_WORKERS=1 L06_PORT_FILE="$out/port" "$out/repro" >"$out/server.log" 2>&1 &
pid=$!
trap 'kill "$pid" 2>/dev/null || true; wait "$pid" 2>/dev/null || true' EXIT
for attempt in {1..200}; do test ! -s "$out/port" || break; sleep .05; done
test -s "$out/port"
port=$(cat "$out/port")
code=$(curl --noproxy '*' -sS --max-time 10 -o "$out/body" -w '%{http_code}' \
  -H 'Connection: close' --data 'name=%ED%95%9C%EA%B8%80' "http://127.0.0.1:$port/form")
{ printf 'HTTP=%s\n' "$code"; cat "$out/body"; printf '\n'; } | tee "$out/repro.actual"
diff -u "$ROOT/tests/l06/repro.expected" "$out/repro.actual"
