#!/usr/bin/env bash
# Regression: trap-extern-flvalue-defn
set -Eeuo pipefail
ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
SRC="$ROOT_DIR/tests/traps/trap-extern-flvalue-defn.fl"
OUT="${TMPDIR:-/tmp}/fx-trap-extern-flvalue-$$"
cd "$ROOT_DIR"
bash ./fl-build.sh "$SRC" "$OUT" >/tmp/fx-trap-extern-flvalue-build.log 2>&1 || {
  echo "TRAP_EXTERN_FLVALUE_DEFN=FAIL build"
  tail -30 /tmp/fx-trap-extern-flvalue-build.log >&2
  exit 1
}
ACTUAL="$("$OUT" 2>/dev/null | tr -d '\r' | sed '/^[[:space:]]*$/d' | tail -1)"
rm -f -- "$OUT"
if [[ "$ACTUAL" == "ok-42" ]]; then
  echo "TRAP_EXTERN_FLVALUE_DEFN=PASS"
  echo "REGRESSION=tests/traps/trap-extern-flvalue-defn.fl"
  exit 0
fi
echo "TRAP_EXTERN_FLVALUE_DEFN=FAIL got=$ACTUAL"
exit 1
