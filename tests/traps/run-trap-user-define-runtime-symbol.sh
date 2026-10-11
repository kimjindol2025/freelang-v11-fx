#!/usr/bin/env bash
# Regression: trap-user-define-runtime-symbol
set -Eeuo pipefail
ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
SRC="$ROOT_DIR/tests/traps/trap-user-define-runtime-symbol.fl"
OUT="${TMPDIR:-/tmp}/fx-trap-user-define-$$"
cd "$ROOT_DIR"
bash ./fl-build.sh "$SRC" "$OUT" >/tmp/fx-trap-user-define-build.log 2>&1 || {
  echo "TRAP_USER_DEFINE_RUNTIME_SYMBOL=FAIL build"
  tail -25 /tmp/fx-trap-user-define-build.log >&2
  exit 1
}
ACTUAL="$("$OUT" 2>/dev/null | tr -d '\r' | sed '/^[[:space:]]*$/d')"
rm -f -- "$OUT"
if [[ "$ACTUAL" == "ok-1" ]]; then
  echo "TRAP_USER_DEFINE_RUNTIME_SYMBOL=PASS"
  echo "REGRESSION=tests/traps/trap-user-define-runtime-symbol.fl"
  exit 0
fi
echo "TRAP_USER_DEFINE_RUNTIME_SYMBOL=FAIL got=$ACTUAL"
exit 1
