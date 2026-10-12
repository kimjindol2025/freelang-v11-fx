#!/usr/bin/env bash
# Phase 1 ④ — cgc kebab API naming warn (closed list) + --strict-names
# Builds a fresh compiler from self/cgc-main.fl (installed cgc-bin may be stale).
set -Eeuo pipefail
ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
RUNTIME_DIR="${RUNTIME_DIR:-$ROOT_DIR/runtime}"
LIST_AIRC="$ROOT_DIR/docs/FX-KEBAB-API-LIST.airc"
TMP="${TMPDIR:-/tmp}/fx-six-kebab-$$"
mkdir -p "$TMP"
cleanup() { rm -rf -- "$TMP"; }
trap cleanup EXIT

find_seed_cgc() {
  if [[ -n "${CGC_BIN:-}" && -x "${CGC_BIN}" ]]; then
    printf '%s\n' "$CGC_BIN"; return 0
  fi
  local c
  for c in \
    "$ROOT_DIR/../freelang-afj/bin/cgc-bin" \
    "$ROOT_DIR/bin/cgc-bin"; do
    if [[ -x "$c" ]]; then printf '%s\n' "$c"; return 0; fi
  done
  return 1
}

SEED_CGC="$(find_seed_cgc)" || {
  echo "SIX_KEBAB_NAMES=FAIL no seed cgc-bin"
  exit 1
}

# 컴파일러 링크용이라 fl-build.sh 앱 목록과 다름 (cgc-bridge 포함, jit/shim/pdf 제외)
RUNTIME_SRCS=(
  "$RUNTIME_DIR/core.c" "$RUNTIME_DIR/collection.c" "$RUNTIME_DIR/io.c"
  "$RUNTIME_DIR/math.c" "$RUNTIME_DIR/error.c" "$RUNTIME_DIR/process.c"
  "$RUNTIME_DIR/json.c" "$RUNTIME_DIR/aliases.c" "$RUNTIME_DIR/user-fns.c"
  "$RUNTIME_DIR/cgc-bridge.c" "$RUNTIME_DIR/gc.c" "$RUNTIME_DIR/http.c"
  "$RUNTIME_DIR/websocket.c" "$RUNTIME_DIR/sqlite.c" "$RUNTIME_DIR/mariadb.c"
  "$RUNTIME_DIR/debug.c" "$RUNTIME_DIR/http_client.c" "$RUNTIME_DIR/regex.c"
  "$RUNTIME_DIR/smtp.c" "$RUNTIME_DIR/sse.c"
)

link_compiler() {
  local src="$1" out="$2" log="$3"
  if ! gcc -O2 -w -I "$RUNTIME_DIR" -o "$out" "$src" "${RUNTIME_SRCS[@]}" \
      -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto -lcurl >"$log" 2>&1; then
    echo "SIX_KEBAB_NAMES=FAIL link_compiler"
    tail -40 "$log" >&2 || true
    exit 1
  fi
}

# --- sync: airc ↔ cgc-main.fl cgc-kebab-api-list ---
if ! python3 - "$LIST_AIRC" "$ROOT_DIR/self/cgc-main.fl" <<'PY'
import re, sys
airc, cgc = sys.argv[1], sys.argv[2]
airc_names=[]
for line in open(airc):
    line=line.strip()
    if not line or line.startswith(';') or line.startswith('(kebab') or line==')':
        continue
    airc_names.append(line)
text=open(cgc).read()
m=re.search(r'\(define cgc-kebab-api-list\s*\[(.*?)\]\)', text, re.S)
if not m:
    print('no cgc-kebab-api-list in cgc-main.fl', file=sys.stderr); sys.exit(1)
cgc_names=re.findall(r'"([^"]+)"', m.group(1))
if airc_names != cgc_names:
    print('airc vs cgc-main mismatch', file=sys.stderr)
    print('airc', airc_names, file=sys.stderr)
    print('cgc ', cgc_names, file=sys.stderr)
    sys.exit(1)
if len(airc_names) != 34:
    print(f'expected 34 got {len(airc_names)}', file=sys.stderr); sys.exit(1)
print(f'LIST_SYNC=PASS count={len(airc_names)}')
open('/tmp/fx-kebab-api-names.txt','w').write('\n'.join(airc_names)+'\n')
PY
then
  echo "SIX_KEBAB_NAMES=FAIL list_sync"
  exit 1
fi

mapfile -t API_NAMES < /tmp/fx-kebab-api-names.txt

cd "$ROOT_DIR"
echo "SEED_CGC=$SEED_CGC"
if ! "$SEED_CGC" "$ROOT_DIR/self/cgc-main.fl" "$TMP/fresh-cgc.c" >"$TMP/seed-compile.log" 2>&1; then
  echo "SIX_KEBAB_NAMES=FAIL seed_compile"
  tail -40 "$TMP/seed-compile.log" >&2 || true
  exit 1
fi
link_compiler "$TMP/fresh-cgc.c" "$TMP/fresh-cgc-bin" "$TMP/link-cgc.log"
FRESH="$TMP/fresh-cgc-bin"

# 1) 기본 모드: 소규모 fixture 경고 (기존)
set +e
OUT1="$("$FRESH" "$ROOT_DIR/tests/six/kebab-names.fl" "$TMP/kebab.c" 2>&1)"
EC1=$?
set -e
echo "$OUT1" >"$TMP/out1.log"
echo "--- kebab fixture ---"
echo "$OUT1" | grep ' | naming | ' || true
for name in http-get server-get json-stringify; do
  if ! echo "$OUT1" | grep -q " | naming | ${name} | "; then
    echo "SIX_KEBAB_NAMES=FAIL missing_warn_${name}"
    echo "$OUT1" >&2
    exit 1
  fi
  if ! echo "$OUT1" | grep " | naming | ${name} | " | grep -qE 'http_get|server_get|json_stringify'; then
    echo "SIX_KEBAB_NAMES=FAIL bad_fix_for_${name}"
    exit 1
  fi
done
if [[ "$EC1" -ne 0 ]]; then
  echo "SIX_KEBAB_NAMES=FAIL unexpected_fail_without_strict ec=$EC1"
  exit 1
fi
if [[ ! -f "$TMP/kebab.c" ]]; then
  echo "SIX_KEBAB_NAMES=FAIL no_output_without_strict"
  exit 1
fi

# 2) --strict-names → 거부 (기존)
rm -f "$TMP/strict.c"
set +e
OUT2="$("$FRESH" --strict-names "$ROOT_DIR/tests/six/kebab-names.fl" "$TMP/strict.c" 2>&1)"
EC2=$?
set -e
echo "$OUT2" >"$TMP/out2.log"
if [[ "$EC2" -eq 0 ]]; then
  echo "SIX_KEBAB_NAMES=FAIL strict_should_reject"
  echo "$OUT2" >&2
  exit 1
fi
if [[ -f "$TMP/strict.c" ]]; then
  echo "SIX_KEBAB_NAMES=FAIL strict_wrote_c"
  exit 1
fi
if ! echo "$OUT2" | grep -q 'strict-names:'; then
  echo "SIX_KEBAB_NAMES=FAIL strict_msg_missing"
  echo "$OUT2" >&2
  exit 1
fi

# 3) underscore 기본 모드 → naming 0 (기존)
set +e
OUT3="$("$FRESH" "$ROOT_DIR/tests/six/kebab-names-underscore.fl" "$TMP/under.c" 2>&1)"
EC3=$?
set -e
if [[ "$EC3" -ne 0 ]]; then
  echo "SIX_KEBAB_NAMES=FAIL underscore_compile"
  echo "$OUT3" >&2
  exit 1
fi
if echo "$OUT3" | grep -q ' | naming | '; then
  echo "SIX_KEBAB_NAMES=FAIL unexpected_naming_on_underscore"
  echo "$OUT3" >&2
  exit 1
fi

# 3b) underscore + --strict-names → exit 0, C 생성 (Q3 고정)
rm -f "$TMP/under-strict.c"
set +e
OUT3B="$("$FRESH" --strict-names "$ROOT_DIR/tests/six/kebab-names-underscore.fl" "$TMP/under-strict.c" 2>&1)"
EC3B=$?
set -e
if [[ "$EC3B" -ne 0 ]]; then
  echo "SIX_KEBAB_NAMES=FAIL underscore_strict_ec=$EC3B"
  echo "$OUT3B" >&2
  exit 1
fi
if [[ ! -f "$TMP/under-strict.c" ]]; then
  echo "SIX_KEBAB_NAMES=FAIL underscore_strict_no_c"
  exit 1
fi
if echo "$OUT3B" | grep -q ' | naming | '; then
  echo "SIX_KEBAB_NAMES=FAIL underscore_strict_naming"
  echo "$OUT3B" >&2
  exit 1
fi

# 4) fx-world: http-get 경고 필수; 사용자 json-ok / http-get-body 는 0
set +e
OUT4="$("$FRESH" "$ROOT_DIR/fx-world/server.fl" "$TMP/fx-world.c" 2>&1)"
EC4=$?
set -e
echo "$OUT4" >"$TMP/fx-world-warn.log"
echo "--- fx-world naming (sample) ---"
echo "$OUT4" | grep ' | naming | ' | head -20 || true
if ! echo "$OUT4" | grep -q ' | naming | http-get | '; then
  echo "SIX_KEBAB_NAMES=FAIL fx-world_missing_http-get"
  echo "$OUT4" | head -40 >&2
  exit 1
fi
if ! echo "$OUT4" | grep ' | naming | http-get | ' | grep -q 'http_get 사용'; then
  echo "SIX_KEBAB_NAMES=FAIL fx-world_http-get_fix"
  exit 1
fi
if echo "$OUT4" | grep -q ' | naming | json-ok | '; then
  echo "SIX_KEBAB_NAMES=FAIL fx-world_false_positive_json-ok"
  exit 1
fi
if echo "$OUT4" | grep -q ' | naming | http-get-body | '; then
  echo "SIX_KEBAB_NAMES=FAIL fx-world_false_positive_http-get-body"
  exit 1
fi
if [[ "$EC4" -ne 0 ]]; then
  echo "SIX_KEBAB_NAMES=FAIL fx-world_compile_ec=$EC4"
  exit 1
fi

# 5) 가) 목록 34개 전부 → naming 정확히 34줄 (이름 집합 일치)
set +e
OUT5="$("$FRESH" "$ROOT_DIR/tests/six/kebab-names-all34.fl" "$TMP/all34.c" 2>&1)"
EC5=$?
set -e
echo "$OUT5" >"$TMP/out5-all34.log"
WARN5=$(echo "$OUT5" | grep ' | naming | ' || true)
COUNT5=$(echo "$WARN5" | grep -c ' | naming | ' || true)
echo "ALL34_WARN_COUNT=$COUNT5"
if [[ "$EC5" -ne 0 ]]; then
  echo "SIX_KEBAB_NAMES=FAIL all34_compile_ec=$EC5"
  echo "$OUT5" | tail -40 >&2
  exit 1
fi
if [[ "$COUNT5" -ne 34 ]]; then
  echo "SIX_KEBAB_NAMES=FAIL all34_warn_count=$COUNT5 want=34"
  echo "$WARN5" >&2
  exit 1
fi
for name in "${API_NAMES[@]}"; do
  if ! echo "$WARN5" | grep -q " | naming | ${name} | "; then
    echo "SIX_KEBAB_NAMES=FAIL all34_missing_${name}"
    exit 1
  fi
done

# 6) 나) 사용자 함수 픽스처 → naming 0
set +e
OUT6="$("$FRESH" "$ROOT_DIR/tests/six/kebab-names-user.fl" "$TMP/user.c" 2>&1)"
EC6=$?
set -e
echo "$OUT6" >"$TMP/out6-user.log"
if [[ "$EC6" -ne 0 ]]; then
  echo "SIX_KEBAB_NAMES=FAIL user_compile_ec=$EC6"
  echo "$OUT6" >&2
  exit 1
fi
if echo "$OUT6" | grep -q ' | naming | '; then
  echo "SIX_KEBAB_NAMES=FAIL user_unexpected_naming"
  echo "$OUT6" >&2
  exit 1
fi

echo "SIX_KEBAB_NAMES=PASS"
echo "REGRESSION=tests/six/run-kebab-names.sh"
echo "LIST_COUNT=34"
echo "FX_WORLD_NAMING_SAMPLE=$(echo "$OUT4" | grep ' | naming | ' | head -5 | tr '\n' '; ')"
