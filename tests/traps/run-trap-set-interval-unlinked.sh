#!/usr/bin/env bash
# Phase1 ⑥ regression — trap-set-interval-swallow relocated.
# Locks: FX fl-build of set_interval/sleep_ms fails link (undefined reference).
# Usage: bash tests/traps/run-trap-set-interval-unlinked.sh
set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
SRC="$ROOT_DIR/tests/traps/trap-set-interval-unlinked.fl"
OUT_BIN="${TMPDIR:-/tmp}/fx-trap-set-interval-unlinked-$$"
LOG="/tmp/fx-trap-set-interval-unlinked-build.log"

cleanup() { rm -f -- "$OUT_BIN"; }
trap cleanup EXIT

cd "$ROOT_DIR"
set +e
bash ./fl-build.sh "$SRC" "$OUT_BIN" >"$LOG" 2>&1
RC=$?
set -e

if [[ -x "$OUT_BIN" ]]; then
  echo "TRAP_SET_INTERVAL_UNLINKED=FAIL unexpected_link_success"
  echo "NOTE=set_interval must remain unlinked on FX native until a deliberate runtime lands"
  exit 1
fi

if ! grep -q 'undefined reference to `set_interval' "$LOG"; then
  echo "TRAP_SET_INTERVAL_UNLINKED=FAIL missing_undefined_set_interval"
  tail -40 "$LOG" >&2 || true
  exit 1
fi

# sleep_ms may also be undefined (emit name ≠ fl_sleep_ms); either way link fails.
echo "TRAP_SET_INTERVAL_UNLINKED=PASS"
echo "REGRESSION=tests/traps/trap-set-interval-unlinked.fl"
echo "COMMAND=bash tests/traps/run-trap-set-interval-unlinked.sh"
echo "NOTE=link fails with undefined reference to set_interval (FX host swallow N/A)"
exit 0
