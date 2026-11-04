#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 2 ]]; then
  echo 'usage: println-flush-check.sh BINARY LOG' >&2
  exit 2
fi

binary="$1"
log="$2"
"$binary" > "$log" 2>&1 &
child=$!
cleanup() {
  kill "$child" 2>/dev/null || true
  wait "$child" 2>/dev/null || true
}
trap cleanup EXIT

sleep 0.1
kill -0 "$child"
grep -Fxq 'FLUSH_MARKER_READY' "$log"
echo PRINTLN_FLUSH_REDIRECT_PASS
