#!/usr/bin/env bash
# Phase 1 ② — *-result API + lint [result-nil-http]
set -Eeuo pipefail
ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
cd "$ROOT_DIR"
TMP="${TMPDIR:-/tmp}/fx-six-result-$$"
mkdir -p "$TMP"
cleanup() { rm -rf -- "$TMP"; }
trap cleanup EXIT

EXPECT=$'file-err=true\nfile-msg=파일 없음\nnth-err=true\nnth-ok=true\nnth-val=20\nenv-err=true\nhttp-err=true\nhttp-nil-raw=false'

if ! bash "$ROOT_DIR/fl-build.sh" "$ROOT_DIR/tests/six/result-api.fl" "$TMP/result.elf" >"$TMP/build.log" 2>&1; then
  echo "SIX_RESULT_API=FAIL build"
  tail -40 "$TMP/build.log" >&2
  exit 1
fi

ACTUAL="$("$TMP/result.elf" 2>"$TMP/run.err" | tr -d '\r' | sed '/^[[:space:]]*$/d')"
echo "ACTUAL=$ACTUAL"
if [[ "$ACTUAL" != "$EXPECT" ]]; then
  echo "SIX_RESULT_API=FAIL mismatch"
  echo "EXPECTED=$EXPECT"
  cat "$TMP/run.err" >&2 || true
  exit 1
fi

# lint warning for nil?(http_get …)
cat > "$TMP/bad-nil-http.fl" <<'EOF'
;; dialect: fx
(define r (http_get "http://x"))
(if (nil? (http_get "http://x")) (println "bad") (println "ok"))
EOF
LINT_OUT="$(bash "$ROOT_DIR/fl-lint.sh" "$TMP/bad-nil-http.fl" 2>&1 || true)"
echo "$LINT_OUT"
if ! echo "$LINT_OUT" | grep -q '\[result-nil-http\]'; then
  echo "SIX_RESULT_API=FAIL lint_missing_result-nil-http"
  exit 1
fi

echo "SIX_RESULT_API=PASS"
echo "REGRESSION=tests/six/run-result-api.sh"
echo "MONITOR_LIST=docs/FX-RESULT-MONITOR-APPS.md"
