#!/usr/bin/env bash
# Phase1 ⑥ regression — trap-mariadb-count-float archived.
# 1) Contract lock: fetch_rows always fl_str_val for non-null cells.
# 2) Runtime: (= "0" 0) is false; string-compare safe pattern holds.
# Usage: bash tests/traps/run-trap-mariadb-count-string.sh
set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
SRC="$ROOT_DIR/tests/traps/trap-mariadb-count-string.fl"
MDB="$ROOT_DIR/runtime/mariadb.c"
OUT_BIN="${TMPDIR:-/tmp}/fx-trap-mariadb-count-string-$$"
EXPECT=$'eq-str-int=false\neq-str-str=true\nsafe-zero=true\ntype-cnt=string'

cleanup() { rm -f -- "$OUT_BIN"; }
trap cleanup EXIT

# --- C contract: non-null cells → fl_str_val in fetch_rows ---
if [[ ! -f "$MDB" ]]; then
  echo "TRAP_MARIADB_COUNT_STRING=FAIL missing_mariadb_c"
  exit 1
fi

# Extract fetch_rows body roughly and require fl_str_val for cell values.
if ! awk '
  /static FLValue fetch_rows/ { inside=1 }
  inside { print }
  inside && /^}/ { exit }
' "$MDB" | grep -q 'val = fl_str_val(buf)'; then
  echo "TRAP_MARIADB_COUNT_STRING=FAIL fetch_rows_missing_fl_str_val"
  exit 1
fi

# Must not invent typed numeric coerce without headers (opaque MYSQL_FIELD).
if awk '
  /static FLValue fetch_rows/ { inside=1 }
  inside { print }
  inside && /^}/ { exit }
' "$MDB" | grep -E 'fl_int\(|fl_float\(|fl_double\(' >/dev/null 2>&1; then
  echo "TRAP_MARIADB_COUNT_STRING=FAIL fetch_rows_unexpected_numeric_coerce"
  exit 1
fi

cd "$ROOT_DIR"
bash ./fl-build.sh "$SRC" "$OUT_BIN" >/tmp/fx-trap-mariadb-count-string-build.log 2>&1

if [[ ! -x "$OUT_BIN" ]]; then
  echo "TRAP_MARIADB_COUNT_STRING=FAIL build (no binary)"
  tail -30 /tmp/fx-trap-mariadb-count-string-build.log >&2 || true
  exit 1
fi

ACTUAL="$("$OUT_BIN" 2>/tmp/fx-trap-mariadb-count-string-stderr.log | tr -d '\r')"
ACTUAL="$(printf '%s\n' "$ACTUAL" | sed '/^[[:space:]]*$/d')"

if [[ "$ACTUAL" == "$EXPECT" ]]; then
  echo "TRAP_MARIADB_COUNT_STRING=PASS"
  echo "REGRESSION=tests/traps/trap-mariadb-count-string.fl"
  echo "COMMAND=bash tests/traps/run-trap-mariadb-count-string.sh"
  echo "NOTE=mariadb fetch_rows cells are strings; (= cnt 0) needs str compare"
  exit 0
fi

echo "TRAP_MARIADB_COUNT_STRING=FAIL output mismatch"
echo "EXPECTED<<EOF"
printf '%s\n' "$EXPECT"
echo "EOF"
echo "ACTUAL<<EOF"
printf '%s\n' "$ACTUAL"
echo "EOF"
exit 1
