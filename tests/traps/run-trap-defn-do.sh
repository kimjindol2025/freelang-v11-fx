#!/usr/bin/env bash
# Regression gate for trap-defn-do (Phase 0 → RESOLVED).
# Usage: bash tests/traps/run-trap-defn-do.sh
set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
SRC="$ROOT_DIR/tests/traps/trap-defn-do.fl"
OUT_BIN="${TMPDIR:-/tmp}/fx-trap-defn-do-$$"
EXPECT=$'step-a\nstep-b\n42'

cleanup() { rm -f -- "$OUT_BIN"; }
trap cleanup EXIT

cd "$ROOT_DIR"
bash ./fl-build.sh "$SRC" "$OUT_BIN" >/tmp/fx-trap-defn-do-build.log 2>&1

if [[ ! -x "$OUT_BIN" ]]; then
  echo "TRAP_DEFN_DO=FAIL build (no binary)"
  tail -20 /tmp/fx-trap-defn-do-build.log >&2 || true
  exit 1
fi

ACTUAL="$("$OUT_BIN" 2>/tmp/fx-trap-defn-do-stderr.log | tr -d '\r')"
# Drop trailing blank lines for compare
ACTUAL="$(printf '%s\n' "$ACTUAL" | sed '/^[[:space:]]*$/d')"

if [[ "$ACTUAL" == "$EXPECT" ]]; then
  echo "TRAP_DEFN_DO=PASS"
  echo "REGRESSION=tests/traps/trap-defn-do.fl"
  echo "COMMAND=bash tests/traps/run-trap-defn-do.sh"
  exit 0
fi

echo "TRAP_DEFN_DO=FAIL output mismatch"
echo "EXPECTED<<EOF"
printf '%s\n' "$EXPECT"
echo "EOF"
echo "ACTUAL<<EOF"
printf '%s\n' "$ACTUAL"
echo "EOF"
if [[ -s /tmp/fx-trap-defn-do-stderr.log ]]; then
  echo "STDERR<<"
  cat /tmp/fx-trap-defn-do-stderr.log
fi
exit 1
