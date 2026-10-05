#!/usr/bin/env bash
# Script ABI P1-A regression: argv values and Script exit status.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
R="$ROOT/runtime"
TMP_P1="$(mktemp -d "${TMPDIR:-/tmp}/freelang-fx-script-abi-XXXXXX")"
trap 'rm -rf "$TMP_P1"' EXIT

SOURCES=(
  "$R/core.c" "$R/collection.c" "$R/io.c" "$R/json.c" "$R/math.c"
  "$R/process.c" "$R/error.c" "$R/http.c" "$R/websocket.c"
  "$R/http_client.c" "$R/sse.c" "$R/aliases.c" "$R/sqlite.c"
  "$R/debug.c" "$R/gc.c" "$R/jit.c" "$R/fx-builtin-shim.c"
  "$R/regex.c" "$R/smtp.c" "$R/pdf_ttf.c" "$R/pdf_img.c"
  "$R/mariadb.c" "$R/user-fns.c"
)

gcc -O0 -w -I "$R" -o "$TMP_P1/script-abi" "$ROOT/tests/test-script-abi.c" \
  "${SOURCES[@]}" -rdynamic -lpthread -lm -ldl -lsqlite3 -lssl -lcrypto -lcurl

actual="$("$TMP_P1/script-abi" alpha beta)"
if [[ "$actual" != "PASS script-abi argv" ]]; then
  printf 'FAIL argv output=%q\n' "$actual" >&2
  exit 1
fi

set +e
"$TMP_P1/script-abi" --exit-probe >"$TMP_P1/exit.out" 2>"$TMP_P1/exit.err"
exit_status=$?
set -e
if [[ $exit_status -ne 42 || "$(<"$TMP_P1/exit.out")" != "before-exit" || -s "$TMP_P1/exit.err" ]]; then
  printf 'FAIL integer exit status=%s stdout=%q stderr=%q\n' "$exit_status" "$(<"$TMP_P1/exit.out")" "$(<"$TMP_P1/exit.err")" >&2
  exit 1
fi

set +e
"$TMP_P1/script-abi" --invalid-exit-probe >"$TMP_P1/invalid.out" 2>"$TMP_P1/invalid.err"
invalid_status=$?
set -e
if [[ $invalid_status -ne 64 ]] || ! grep -Fqx 'FreeLang Script exit requires integer status 0..255' "$TMP_P1/invalid.err"; then
  printf 'FAIL invalid exit status=%s stderr=%q\n' "$invalid_status" "$(<"$TMP_P1/invalid.err")" >&2
  exit 1
fi

printf 'PASS Script ABI P1-A\n'
