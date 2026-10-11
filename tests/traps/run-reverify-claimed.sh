#!/usr/bin/env bash
# Re-verify RESOLVED_CLAIMED traps. Exit 0 only if zero FAIL lines and all 10 PASS.
set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
SRC="$ROOT_DIR/tests/traps/reverify-claimed.fl"
OUT_BIN="${TMPDIR:-/tmp}/fx-reverify-claimed-$$"
LOG_BUILD="/tmp/fx-reverify-claimed-build.log"
LOG_RUN="/tmp/fx-reverify-claimed-run.log"

IDS=(
  trap-count-type
  trap-kebab-symbol
  trap-str-includes-int
  trap-throw-invalid-initializer
  trap-server-html-quote
  trap-string-literal-1024
  trap-try-map-literal
  trap-inc-dec-missing
  trap-cache-cgc-missing
  trap-defn-docstring-cgc
)

cleanup() { rm -f -- "$OUT_BIN"; }
trap cleanup EXIT

cd "$ROOT_DIR"
if ! bash ./fl-build.sh "$SRC" "$OUT_BIN" >"$LOG_BUILD" 2>&1; then
  echo "REVERIFY_CLAIMED=FAIL build"
  tail -40 "$LOG_BUILD" >&2 || true
  exit 1
fi

"$OUT_BIN" >"$LOG_RUN" 2>"$LOG_RUN.err" || true
cat "$LOG_RUN"

FAIL_N=0
PASS_N=0
MISSING=0
for id in "${IDS[@]}"; do
  if grep -q "^PASS $id" "$LOG_RUN"; then
    PASS_N=$((PASS_N + 1))
  elif grep -q "^FAIL $id" "$LOG_RUN"; then
    echo "STATUS $id=FAIL"
    FAIL_N=$((FAIL_N + 1))
  else
    echo "STATUS $id=MISSING"
    MISSING=$((MISSING + 1))
  fi
done

echo "REVERIFY_SUMMARY pass=$PASS_N fail=$FAIL_N missing=$MISSING total=${#IDS[@]}"
if [[ -s "$LOG_RUN.err" ]]; then
  echo "STDERR<<"
  cat "$LOG_RUN.err"
fi

if [[ "$FAIL_N" -eq 0 && "$MISSING" -eq 0 && "$PASS_N" -eq "${#IDS[@]}" ]]; then
  echo "REVERIFY_CLAIMED=PASS"
  exit 0
fi
echo "REVERIFY_CLAIMED=FAIL"
exit 1
