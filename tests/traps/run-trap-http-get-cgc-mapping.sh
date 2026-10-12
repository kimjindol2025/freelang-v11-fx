#!/usr/bin/env bash
# Phase 1 regression for trap-http-get-cgc-mapping.
# Local HTTP probe + fresh/installed cgc → expect map {status,body,headers}.
set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
RUNTIME_DIR="${RUNTIME_DIR:-$ROOT_DIR/runtime}"
SRC="$ROOT_DIR/tests/traps/trap-http-get-cgc-mapping.fl"
TMP="${TMPDIR:-/tmp}/fx-trap-http-get-$$"
PORT="${TRAP_HTTP_PORT:-18771}"
EXPECT=$'shape=map\nstatus=200\nbody=hello-body\nheaders=ok\nkebab-status=200'

mkdir -p "$TMP"
SRV_PID=""
cleanup() {
  if [[ -n "$SRV_PID" ]] && kill -0 "$SRV_PID" 2>/dev/null; then
    kill "$SRV_PID" 2>/dev/null || true
    wait "$SRV_PID" 2>/dev/null || true
  fi
  rm -rf -- "$TMP"
}
trap cleanup EXIT

find_seed_cgc() {
  if [[ -n "${CGC_BIN:-}" && -x "${CGC_BIN}" ]]; then
    printf '%s\n' "$CGC_BIN"; return 0
  fi
  local c
  for c in \
    "$ROOT_DIR/../freelang-afj/bin/cgc-bin" \
    "$ROOT_DIR/bin/cgc-bin" \
    "$ROOT_DIR/bootstrap/stage0-bin"; do
    if [[ -x "$c" ]]; then printf '%s\n' "$c"; return 0; fi
  done
  return 1
}

SEED_CGC="$(find_seed_cgc)" || {
  echo "TRAP_HTTP_GET_CGC_MAPPING=FAIL no seed cgc-bin"
  exit 1
}

link_app() {
  local src="$1" out="$2" log="$3"
  if ! gcc -O2 -w -I "$RUNTIME_DIR" -o "$out" "$src" \
      "$RUNTIME_DIR/core.c" "$RUNTIME_DIR/collection.c" "$RUNTIME_DIR/io.c" \
      "$RUNTIME_DIR/math.c" "$RUNTIME_DIR/error.c" "$RUNTIME_DIR/process.c" \
      "$RUNTIME_DIR/json.c" "$RUNTIME_DIR/aliases.c" "$RUNTIME_DIR/user-fns.c" \
      "$RUNTIME_DIR/gc.c" "$RUNTIME_DIR/http.c" "$RUNTIME_DIR/websocket.c" \
      "$RUNTIME_DIR/sqlite.c" "$RUNTIME_DIR/mariadb.c" "$RUNTIME_DIR/debug.c" \
      "$RUNTIME_DIR/http_client.c" "$RUNTIME_DIR/regex.c" "$RUNTIME_DIR/smtp.c" \
      "$RUNTIME_DIR/sse.c" \
      -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto -lcurl >"$log" 2>&1; then
    echo "TRAP_HTTP_GET_CGC_MAPPING=FAIL link_app"
    tail -40 "$log" >&2 || true
    exit 1
  fi
}

cd "$ROOT_DIR"
echo "SEED_CGC=$SEED_CGC"

# Local probe server (keeps accepting until killed)
python3 - "$PORT" <<'PY' &
import http.server, socketserver, sys
port = int(sys.argv[1])
class H(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-Type", "text/plain")
        self.send_header("X-Probe", "fx")
        self.end_headers()
        self.wfile.write(b"hello-body")
    def log_message(self, *a):
        pass
socketserver.TCPServer.allow_reuse_address = True
with socketserver.TCPServer(("127.0.0.1", port), H) as httpd:
    httpd.serve_forever()
PY
SRV_PID=$!
sleep 0.25
if ! kill -0 "$SRV_PID" 2>/dev/null; then
  echo "TRAP_HTTP_GET_CGC_MAPPING=FAIL probe_server"
  exit 1
fi

if ! "$SEED_CGC" "$SRC" "$TMP/app.c" >/tmp/fx-http-get-fixture-compile.log 2>&1; then
  echo "TRAP_HTTP_GET_CGC_MAPPING=FAIL fixture_compile"
  cat /tmp/fx-http-get-fixture-compile.log >&2 || true
  exit 1
fi

# Must call http_get (map API), never fl_http_get (legacy body string).
if ! grep -q 'http_get(' "$TMP/app.c"; then
  echo "TRAP_HTTP_GET_CGC_MAPPING=FAIL emit_missing_http_get"
  head -80 "$TMP/app.c" >&2 || true
  exit 1
fi
if grep -q 'fl_http_get(' "$TMP/app.c"; then
  echo "TRAP_HTTP_GET_CGC_MAPPING=FAIL emit_uses_legacy_fl_http_get"
  grep -n 'fl_http_get(' "$TMP/app.c" >&2 || true
  exit 1
fi

link_app "$TMP/app.c" "$TMP/app-bin" /tmp/fx-http-get-link-app.log

export TRAP_HTTP_URL="http://127.0.0.1:${PORT}/"
export FL_HTTP_ALLOW="127.0.0.1,localhost"
ACTUAL="$("$TMP/app-bin" 2>/tmp/fx-http-get-run.err | tr -d '\r' | sed '/^[[:space:]]*$/d')"
echo "ACTUAL=$ACTUAL"

if [[ "$ACTUAL" == "$EXPECT" ]]; then
  echo "TRAP_HTTP_GET_CGC_MAPPING=PASS"
  echo "REGRESSION=tests/traps/trap-http-get-cgc-mapping.fl"
  echo "COMMAND=bash tests/traps/run-trap-http-get-cgc-mapping.sh"
  exit 0
fi

echo "TRAP_HTTP_GET_CGC_MAPPING=FAIL"
echo "EXPECTED=$EXPECT"
if [[ -s /tmp/fx-http-get-run.err ]]; then cat /tmp/fx-http-get-run.err; fi
head -60 "$TMP/app.c" || true
exit 1
