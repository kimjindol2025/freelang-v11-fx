#!/usr/bin/env bash
# Phase 1 ⑤ — @effect pure vs net/db (fl-lint [effect-purity])
set -Eeuo pipefail
ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
LINT="$ROOT_DIR/fl-lint.sh"
AIRC="$ROOT_DIR/docs/FX-BUILTIN-EFFECTS.airc"
PY="$ROOT_DIR/scripts/fl-lint-effect-purity.py"
TMP="${TMPDIR:-/tmp}/fx-six-effect-$$"
mkdir -p "$TMP"
cleanup() { rm -rf -- "$TMP"; }
trap cleanup EXIT

cd "$ROOT_DIR"
[ -x "$LINT" ] || chmod +x "$LINT"
[ -f "$PY" ] && [ -f "$AIRC" ] || {
  echo "SIX_EFFECT_ANNO=FAIL missing_py_or_airc"
  exit 1
}

# 1) pure + pure builtins → effect-purity 0 (형식 경고도 없어야 함)
OUT1="$(bash "$LINT" "$ROOT_DIR/tests/six/effect-pure-ok.fl" 2>&1 || true)"
echo "$OUT1" >"$TMP/ok.log"
if echo "$OUT1" | grep -q '\[effect-purity\]'; then
  echo "SIX_EFFECT_ANNO=FAIL unexpected_purity_on_ok"
  echo "$OUT1" >&2
  exit 1
fi
if echo "$OUT1" | grep -q '\[effect-anno\]'; then
  echo "SIX_EFFECT_ANNO=FAIL unexpected_anno_format_on_ok"
  echo "$OUT1" >&2
  exit 1
fi

# 2) pure + http_get → effect-purity 필수
OUT2="$(bash "$LINT" "$ROOT_DIR/tests/six/effect-pure-bad-net.fl" 2>&1 || true)"
echo "$OUT2" >"$TMP/bad-net.log"
echo "--- bad-net ---"
echo "$OUT2" | grep '\[effect-purity\]' || true
if ! echo "$OUT2" | grep -q '\[effect-purity\].*http_get'; then
  echo "SIX_EFFECT_ANNO=FAIL missing_purity_http_get"
  echo "$OUT2" >&2
  exit 1
fi

# 3) pure + db → effect-purity 필수
OUT3="$(bash "$LINT" "$ROOT_DIR/tests/six/effect-pure-bad-db.fl" 2>&1 || true)"
echo "$OUT3" >"$TMP/bad-db.log"
echo "--- bad-db ---"
echo "$OUT3" | grep '\[effect-purity\]' || true
if ! echo "$OUT3" | grep -q '\[effect-purity\].*(db'; then
  # message contains "빌트인 (db)" 
  if ! echo "$OUT3" | grep -q '\[effect-purity\].*\(db\)'; then
    echo "SIX_EFFECT_ANNO=FAIL missing_purity_db"
    echo "$OUT3" >&2
    exit 1
  fi
fi

# 4) 형식만 틀린 주석 → 기존 [effect-anno] (purity 아님)
cat >"$TMP/bad-format.fl" <<'EOF'
;; dialect: fx
;; @effect pure
(defn x [] 1)
EOF
OUT4="$(bash "$LINT" "$TMP/bad-format.fl" 2>&1 || true)"
echo "$OUT4" >"$TMP/bad-format.log"
if ! echo "$OUT4" | grep -q '\[effect-anno\]'; then
  echo "SIX_EFFECT_ANNO=FAIL missing_effect_anno_format"
  echo "$OUT4" >&2
  exit 1
fi

# 5) 직접 py: airc net/db 로드 스모크
python3 "$PY" "$ROOT_DIR/tests/six/effect-pure-bad-net.fl" "$AIRC" >"$TMP/py-net.txt"
COUNT=$(grep -c '\[effect-purity\]' "$TMP/py-net.txt" || true)
if [[ "$COUNT" -lt 1 ]]; then
  echo "SIX_EFFECT_ANNO=FAIL py_direct"
  cat "$TMP/py-net.txt" >&2
  exit 1
fi

echo "SIX_EFFECT_ANNO=PASS"
echo "REGRESSION=tests/six/run-effect-anno.sh"
echo "BAD_NET_SAMPLE=$(grep '\[effect-purity\]' "$TMP/bad-net.log" | head -1)"
echo "BAD_DB_SAMPLE=$(grep '\[effect-purity\]' "$TMP/bad-db.log" | head -1)"
