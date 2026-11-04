#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 3 ]]; then
  echo 'usage: rem-zero-check.sh BINARY STDOUT STDERR' >&2
  exit 2
fi

binary="$1"
stdout="$2"
stderr="$3"
set +e
"$binary" > "$stdout" 2> "$stderr"
result=$?
set -e
[[ "$result" -eq 1 ]]
grep -Fxq 'error: mod by zero' "$stderr"
echo REM_ZERO_DIVISOR_PASS
