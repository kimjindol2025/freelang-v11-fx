#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l06/build.sh"
out="${L06_OUT:-$(mktemp -d)}"
mkdir -p "$out"
if [[ ! -x "$out/compiler" ]]; then l07_build_compiler "$out" >"$out/build.log" 2>&1; fi
l06_app "$out" "$ROOT/tests/l06/server.fl" server >>"$out/build.log" 2>&1
gcc -O2 -Wall -Wextra -o "$out/client" "$ROOT/tests/l06/client.c" -lssl -lcrypto -lpthread >>"$out/build.log" 2>&1
pid=""
stop_server() { if [[ -n "$pid" ]]; then kill "$pid" 2>/dev/null || true; wait "$pid" 2>/dev/null || true; pid=""; fi; }
trap stop_server EXIT
start_server() {
  local label="$1"; shift
  mkdir -p "$out/$label/root"
  chmod 700 "$out/$label/root"
  rm -f "$out/$label/port"
  FL_WORKERS=4 FL_DEBUG=2 FL_FILE_BASE="$out/$label/root" TMPDIR="$out/$label/root/tmp" \
    L06_PORT_FILE="$out/$label/port" L06_HOLD_FILE="$out/$label/hold" \
    "$out/server" "$@" >"$out/$label/server.log" 2>&1 &
  pid=$!
  for attempt in {1..200}; do [[ ! -s "$out/$label/port" ]] || break; sleep .05; done
  test -s "$out/$label/port"
}
start_server http
"$out/client" "$(cat "$out/http/port")" "$out/http/root" | tee "$out/http/client.log"
if rg -F 'L06_FAKE_SECRET' "$out/http/server.log"; then echo LOG_REDACTION=FAIL; exit 1; fi
echo LOG_REDACTION=PASS
"$out/client" "$(cat "$out/http/port")" "$out/http/root" timeouts | tee "$out/http/timeouts.log"
# A disconnected sender must not invoke a handler or leave a temp file.
port=$(cat "$out/http/port")
exec {abort_fd}<>"/dev/tcp/127.0.0.1/$port"
printf 'POST /upload HTTP/1.1\r\nHost: localhost\r\nContent-Type: multipart/form-data; boundary=B\r\nContent-Length: 1000\r\n\r\n--B\r\n' >&"$abort_fd"
exec {abort_fd}>&-
health=$(curl --noproxy '*' -fsS --max-time 5 "http://127.0.0.1:$port/health")
test "$health" = OK
test "$(find "$out/http/root/tmp/fl-uploads" -mindepth 1 -maxdepth 1 -type f | wc -l)" = 0
echo DISCONNECTED_SENDER_RECOVERY=PASS
# An active upload's locked marker survives another instance's recovery.
printf 'A\000B' >"$out/http/upload.bin"
curl --noproxy '*' -sS --max-time 30 -F "image=@$out/http/upload.bin" "http://127.0.0.1:$port/hold" >"$out/http/hold-response" 2>"$out/http/hold-curl.log" &
hold_client=$!
for attempt in {1..200}; do [[ ! -f "$out/http/hold" ]] || break; sleep .05; done
test -f "$out/http/hold"
test "$(find "$out/http/root/tmp/fl-uploads" -type f | wc -l)" = 2
FL_WORKERS=1 FL_FILE_BASE="$out/http/root" TMPDIR="$out/http/root/tmp" \
  L06_PORT_FILE="$out/http/second-port" "$out/server" >"$out/http/second-server.log" 2>&1 &
second_pid=$!
for attempt in {1..200}; do [[ ! -s "$out/http/second-port" ]] || break; sleep .05; done
test -s "$out/http/second-port"
test "$(find "$out/http/root/tmp/fl-uploads" -type f | wc -l)" = 2
echo LIVE_INSTANCE_TEMP_PRESERVED=PASS
kill "$second_pid"; wait "$second_pid"
# Kill only this script's server. Its lock is released, enabling recovery.
kill -KILL "$pid"; wait "$pid" 2>/dev/null || true; pid=""
wait "$hold_client" 2>/dev/null || true
test "$(find "$out/http/root/tmp/fl-uploads" -type f | wc -l)" = 2
start_server http
test "$(find "$out/http/root/tmp/fl-uploads" -type f | wc -l)" = 0
printf 'SIGKILL_RESTART_OWN_TEMP=0\n'
health=$(curl --noproxy '*' -fsS --max-time 5 "http://127.0.0.1:$(cat "$out/http/port")/health")
test "$health" = OK
printf 'RESTART_HTTP=200\n'
printf 'A\000B' | cmp - "$out/http/root/permanent"
echo PERMANENT_COPY_PRESERVED=PASS

stop_server
for config in missing symlink; do
  mkdir -p "$out/storage-$config"
  if [[ "$config" = missing ]]; then
    env -u FL_FILE_BASE -u TMPDIR FL_WORKERS=1 L06_PORT_FILE="$out/storage-$config/port" \
      "$out/server" >"$out/storage-$config/server.log" 2>&1 &
  else
    mkdir -p "$out/storage-$config/real"
    ln -s real "$out/storage-$config/link"
    FL_FILE_BASE="$out/storage-$config/link" TMPDIR="$out/storage-$config/link/tmp" \
      FL_WORKERS=1 L06_PORT_FILE="$out/storage-$config/port" \
      "$out/server" >"$out/storage-$config/server.log" 2>&1 &
  fi
  pid=$!
  for attempt in {1..200}; do [[ ! -s "$out/storage-$config/port" ]] || break; sleep .05; done
  test -s "$out/storage-$config/port"
  status=$(curl --noproxy '*' -sS --max-time 5 -o "$out/storage-$config/body" -w '%{http_code}' \
    -F "image=@$out/http/upload.bin" "http://127.0.0.1:$(cat "$out/storage-$config/port")/upload")
  test "$status" = 503
  test "$(cat "$out/storage-$config/body")" = '{"error":"UPLOAD_STORAGE_UNAVAILABLE"}'
  printf 'STORAGE_%s_HTTP=%s\n' "$config" "$status"
  stop_server
done

if ! command -v openssl >/dev/null; then echo L06_TLS=UNEXECUTED_MISSING_OPENSSL; exit 2; fi
mkdir -p "$out/tls"
openssl req -x509 -newkey rsa:2048 -nodes -days 1 -subj /CN=localhost \
  -keyout "$out/tls/key.pem" -out "$out/tls/cert.pem" >"$out/tls/cert-build.log" 2>&1
start_server tls tls "$out/tls/cert.pem" "$out/tls/key.pem"
"$out/client" "$(cat "$out/tls/port")" "$out/tls/root" tls | tee "$out/tls/client.log"
stop_server
echo L06_HTTP_TLS=PASS
