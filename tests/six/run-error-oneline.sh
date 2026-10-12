#!/usr/bin/env bash
# Phase 1 ① — ERROR-SCHEMA one-liner before legacy Uncaught lines.
set -Eeuo pipefail
ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
cd "$ROOT_DIR"
TMP="${TMPDIR:-/tmp}/fx-six-error-$$"
mkdir -p "$TMP"
cleanup() { rm -rf -- "$TMP"; }
trap cleanup EXIT

build_run() {
  local src="$1" name="$2"
  if ! bash "$ROOT_DIR/fl-build.sh" "$src" "$TMP/$name.elf" >"$TMP/$name.build.log" 2>&1; then
    echo "SIX_ERROR_ONELINE=FAIL build $name"
    tail -20 "$TMP/$name.build.log" >&2
    exit 1
  fi
  set +e
  "$TMP/$name.elf" >"$TMP/$name.out" 2>"$TMP/$name.err"
  local rc=$?
  set -e
  if [[ "$rc" -eq 0 ]]; then
    echo "SIX_ERROR_ONELINE=FAIL expected_nonzero $name"
    exit 1
  fi
  local first
  first="$(head -1 "$TMP/$name.err" | tr -d '\r')"
  echo "CASE=$name"
  echo "FIRST=$first"
  # must be schema: -:… | kind | value | reason | fix
  if ! [[ "$first" =~ ^-:[^|]+\ \|\ [^|]+\ \|\ [^|]+\ \|\ [^|]+\ \|\ .+$ ]]; then
    echo "SIX_ERROR_ONELINE=FAIL bad_schema_line $name"
    cat "$TMP/$name.err" >&2
    exit 1
  fi
  if ! grep -q 'Uncaught error' "$TMP/$name.err"; then
    echo "SIX_ERROR_ONELINE=FAIL missing_legacy_uncaught $name"
    cat "$TMP/$name.err" >&2
    exit 1
  fi
}

build_run "$ROOT_DIR/tests/six/error-oneline-uncaught.fl" uncaught
build_run "$ROOT_DIR/tests/six/error-oneline-div0.fl" div0
build_run "$ROOT_DIR/tests/six/error-oneline-getnil.fl" getnil

# specific fix hints for the five common paths we filled
grep -q 'try/catch' "$TMP/uncaught.err" || { echo FAIL uncaught fix; exit 1; }
grep -q '분모가 0' "$TMP/div0.err" || { echo FAIL div0 fix; exit 1; }
grep -q 'nil 체크' "$TMP/getnil.err" || { echo FAIL getnil fix; exit 1; }

echo "SIX_ERROR_ONELINE=PASS"
echo "REGRESSION=tests/six/run-error-oneline.sh"
