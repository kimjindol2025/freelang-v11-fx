#!/usr/bin/env bash
# Phase1 ⑥ regression — trap-defn-throw-rerun (FX: no re-run on throw).
# Usage: bash tests/traps/run-trap-defn-throw-rerun.sh
set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
SRC="$ROOT_DIR/tests/traps/trap-defn-throw-rerun.fl"
OUT_BIN="${TMPDIR:-/tmp}/fx-trap-defn-throw-rerun-$$"
EXPECT=$'caught=first\ncalls=1'

cleanup() { rm -f -- "$OUT_BIN"; }
trap cleanup EXIT

cd "$ROOT_DIR"
bash ./fl-build.sh "$SRC" "$OUT_BIN" >/tmp/fx-trap-defn-throw-rerun-build.log 2>&1

if [[ ! -x "$OUT_BIN" ]]; then
  echo "TRAP_DEFN_THROW_RERUN=FAIL build (no binary)"
  tail -30 /tmp/fx-trap-defn-throw-rerun-build.log >&2 || true
  exit 1
fi

ACTUAL="$("$OUT_BIN" 2>/tmp/fx-trap-defn-throw-rerun-stderr.log | tr -d '\r')"
ACTUAL="$(printf '%s\n' "$ACTUAL" | sed '/^[[:space:]]*$/d')"

if [[ "$ACTUAL" == "$EXPECT" ]]; then
  echo "TRAP_DEFN_THROW_RERUN=PASS"
  echo "REGRESSION=tests/traps/trap-defn-throw-rerun.fl"
  echo "COMMAND=bash tests/traps/run-trap-defn-throw-rerun.sh"
  exit 0
fi

echo "TRAP_DEFN_THROW_RERUN=FAIL output mismatch"
echo "EXPECTED<<EOF"
printf '%s\n' "$EXPECT"
echo "EOF"
echo "ACTUAL<<EOF"
printf '%s\n' "$ACTUAL"
echo "EOF"
if [[ -s /tmp/fx-trap-defn-throw-rerun-stderr.log ]]; then
  echo "STDERR<<"
  cat /tmp/fx-trap-defn-throw-rerun-stderr.log
fi
exit 1
