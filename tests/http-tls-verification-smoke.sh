#!/usr/bin/env bash
set -euo pipefail

root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
runtime="$root/runtime"
test_tmp=$(mktemp -d "${TMPDIR:-/tmp}/freelang-http-tls.XXXXXX")
trap 'rm -rf "$test_tmp"' EXIT

openssl req -x509 -newkey rsa:2048 -nodes -days 1 \
  -keyout "$test_tmp/key.pem" -out "$test_tmp/cert.pem" \
  -subj '/CN=localhost' \
  -addext 'subjectAltName=IP:127.0.0.1' >/dev/null 2>&1

runtime_sources=(
  "$runtime/core.c" "$runtime/collection.c" "$runtime/io.c" "$runtime/json.c"
  "$runtime/math.c" "$runtime/process.c" "$runtime/error.c" "$runtime/http.c"
  "$runtime/websocket.c" "$runtime/http_client.c" "$runtime/sse.c"
  "$runtime/aliases.c" "$runtime/sqlite.c" "$runtime/debug.c" "$runtime/gc.c"
  "$runtime/jit.c" "$runtime/fx-builtin-shim.c" "$runtime/regex.c"
  "$runtime/smtp.c" "$runtime/pdf_ttf.c" "$runtime/pdf_img.c"
  "$runtime/mariadb.c" "$runtime/user-fns.c"
)

gcc -O0 -w -I "$runtime" -o "$test_tmp/http-tls-probe" \
  "$root/tests/test-http-tls-verification.c" "${runtime_sources[@]}" \
  -rdynamic -lpthread -lm -ldl -lsqlite3 -lssl -lcrypto -lcurl
node "$root/tests/http-tls-verification.js" \
  "$test_tmp/http-tls-probe" "$test_tmp/key.pem" "$test_tmp/cert.pem"
