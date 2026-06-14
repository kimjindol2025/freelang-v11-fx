#!/bin/bash
# verify-fixpoint.sh — FreeLang fx 고정점 연속 검증 (gen-a ~ gen-c)
# 사용: ./verify-fixpoint.sh [cgc-main.fl 경로]
# 역할: 새 기능 추가 후 고정점이 유지되는지 CI 수준으로 확인
#
# 정책: 고정점 PASS 없이 커밋 금지 (사용자 지시 2026-06-15)
# 기록: FIXPOINT_LOG에 날짜/SHA/결과 자동 저장

set -e

CGC_MAIN="${1:-/home/kimjin/freelang-v11/self/cgc-main.fl}"
CGC_BIN="/home/kimjin/freelang-v11/bin/cgc-bin"
RUNTIME_DIR="/home/kimjin/freelang-v11-fx/runtime"
FIXPOINT_LOG="/home/kimjin/freelang-v11-fx/FIXPOINT_LOG.md"

RUNTIME_SRCS="$RUNTIME_DIR/core.c $RUNTIME_DIR/collection.c $RUNTIME_DIR/io.c \
  $RUNTIME_DIR/math.c $RUNTIME_DIR/error.c $RUNTIME_DIR/process.c \
  $RUNTIME_DIR/json.c $RUNTIME_DIR/aliases.c $RUNTIME_DIR/cgc-bridge.c \
  $RUNTIME_DIR/gc.c $RUNTIME_DIR/http.c $RUNTIME_DIR/sqlite.c \
  $RUNTIME_DIR/mariadb.c $RUNTIME_DIR/debug.c"

TMP=$(mktemp -d /tmp/fixpoint-XXXXXX)
trap "rm -rf $TMP" EXIT

compile_bin() {
  local src=$1 out=$2
  gcc -O2 -w -I "$RUNTIME_DIR" -o "$out" "$src" $RUNTIME_SRCS \
    -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto 2>&1 | grep "error:" | head -3 || true
  [ -f "$out" ] || { echo "❌ 빌드 실패: $src"; exit 1; }
}

echo "🔬 FreeLang fx 고정점 검증"
echo "   cgc-main.fl : $CGC_MAIN"
echo "   cgc-bin     : $CGC_BIN"
echo ""

# gen-a: 현재 cgc-bin → C
echo "⚙️  [1/4] cgc-bin → gen-a.c"
"$CGC_BIN" "$CGC_MAIN" "$TMP/gen-a.c" 2>/dev/null

# gen-a-bin: gen-a.c → 실행파일
echo "⚙️  [2/4] gen-a.c → gen-a-bin"
compile_bin "$TMP/gen-a.c" "$TMP/gen-a-bin"

# gen-b: gen-a-bin → C
echo "⚙️  [3/4] gen-a-bin → gen-b.c"
"$TMP/gen-a-bin" "$CGC_MAIN" "$TMP/gen-b.c" 2>/dev/null

# gen-b-bin: gen-b.c → 실행파일
echo "⚙️  [4/4] gen-b.c → gen-b-bin"
compile_bin "$TMP/gen-b.c" "$TMP/gen-b-bin"

# gen-c: gen-b-bin → C (수렴 확인)
echo "⚙️  [5/4] gen-b-bin → gen-c.c"
"$TMP/gen-b-bin" "$CGC_MAIN" "$TMP/gen-c.c" 2>/dev/null

SHA_A=$(sha256sum "$TMP/gen-a.c" | cut -d' ' -f1)
SHA_B=$(sha256sum "$TMP/gen-b.c" | cut -d' ' -f1)
SHA_C=$(sha256sum "$TMP/gen-c.c" | cut -d' ' -f1)
COMMIT=$(cd /home/kimjin/freelang-v11 && git rev-parse --short HEAD 2>/dev/null || echo "unknown")
DATE=$(date '+%Y-%m-%d %H:%M')

echo ""
echo "── SHA 결과 ──────────────────────────────────"
echo "gen-a: ${SHA_A:0:32}"
echo "gen-b: ${SHA_B:0:32}  (1단계 전이)"
echo "gen-c: ${SHA_C:0:32}  (수렴 확인)"
echo ""

log_result() {
  local result=$1
  # FIXPOINT_LOG가 없으면 헤더 생성
  if [ ! -f "$FIXPOINT_LOG" ]; then
    cat > "$FIXPOINT_LOG" << 'HDR'
# FreeLang fx 고정점 검증 로그

| 날짜 | commit | gen-a (16) | gen-b (16) | gen-c (16) | 결과 |
|------|--------|-----------|-----------|-----------|------|
HDR
  fi
  printf "| %s | %s | %s | %s | %s | %s |\n" \
    "$DATE" "$COMMIT" \
    "${SHA_A:0:16}" "${SHA_B:0:16}" "${SHA_C:0:16}" \
    "$result" >> "$FIXPOINT_LOG"
}

if [ "$SHA_A" = "$SHA_B" ] && [ "$SHA_B" = "$SHA_C" ]; then
  echo "✅ 완전 고정점 (gen-a == gen-b == gen-c)"
  echo "   현재 cgc-bin이 이미 최신 cgc-main.fl을 정확히 표현함"
  log_result "✅ 완전"
  exit 0
elif [ "$SHA_B" = "$SHA_C" ]; then
  echo "⚠️  부분 고정점 (gen-b == gen-c, gen-a ≠ gen-b)"
  echo "   → cgc-main.fl에 새 기능이 있고 cgc-bin이 구버전임"
  echo "   → fx-upgrade-cgc로 cgc-bin 교체 후 재검증 필요"
  log_result "⚠️ 부분"
  echo ""
  echo "   cgc-bin을 자동 교체합니까? [y/N]"
  read -r ans
  if [ "$ans" = "y" ] || [ "$ans" = "Y" ]; then
    cp "$TMP/gen-b-bin" "$CGC_BIN" && chmod +x "$CGC_BIN"
    echo "✅ cgc-bin 교체 완료 (${SHA_B:0:16})"
  fi
  exit 0
else
  echo "❌ 고정점 붕괴 — gen-b ≠ gen-c"
  echo "   컴파일러가 자기 자신을 동일하게 생성하지 못함"
  echo "   ⛔ 이 변경은 커밋 불가. 디버그 후 재검증 필요."
  log_result "❌ 붕괴"
  echo ""
  echo "── 차이 (첫 20줄) ──"
  diff <(head -20 "$TMP/gen-b.c") <(head -20 "$TMP/gen-c.c") || true
  exit 1
fi
