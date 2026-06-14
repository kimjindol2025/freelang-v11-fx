#!/bin/bash
# fx-upgrade-cgc — cgc-main.fl 수정 후 cgc-bin 자동 교체
#
# 절차: cgc-main.fl → genN.c → genN-bin → genN+1.c → SHA256 비교 → cgc-bin 교체
#
# 사용: fx-upgrade-cgc [cgc-main.fl 경로]

set -e

CGC_MAIN="${1:-/home/kimjin/freelang-v11/self/cgc-main.fl}"
CGC_BIN_PATH="/home/kimjin/freelang-v11/bin/cgc-bin"
CURRENT_BIN="$CGC_BIN_PATH"
RUNTIME_DIR="/home/kimjin/freelang-v11-fx/runtime"

TMPDIR_WORK=$(mktemp -d /tmp/cgc-upgrade-XXXXXX)
GEN_A="$TMPDIR_WORK/gen-a.c"
GEN_B="$TMPDIR_WORK/gen-b.c"
GEN_BIN="$TMPDIR_WORK/gen-bin"

echo "🔧 fx-upgrade-cgc 시작"
echo "   cgc-main.fl : $CGC_MAIN"
echo "   현재 cgc-bin: $CURRENT_BIN"
echo ""

# ── 런타임 소스 목록 ──────────────────────────────────────────────
RUNTIME_SRCS="$RUNTIME_DIR/core.c $RUNTIME_DIR/collection.c $RUNTIME_DIR/io.c \
  $RUNTIME_DIR/math.c $RUNTIME_DIR/error.c $RUNTIME_DIR/process.c \
  $RUNTIME_DIR/json.c $RUNTIME_DIR/aliases.c $RUNTIME_DIR/cgc-bridge.c \
  $RUNTIME_DIR/gc.c $RUNTIME_DIR/http.c $RUNTIME_DIR/sqlite.c \
  $RUNTIME_DIR/mariadb.c $RUNTIME_DIR/debug.c"

# ── Step 1: 현재 cgc-bin으로 cgc-main.fl → gen-a.c ───────────────
echo "⚙️  [1/4] 현재 cgc-bin → gen-a.c ..."
"$CURRENT_BIN" "$CGC_MAIN" "$GEN_A" 2>&1 | grep -v "^\[FL Warn\]" | grep -v "^$" || true
echo "   gen-a.c: $(wc -l < "$GEN_A") 줄"

# ── Step 2: gen-a.c → gen-bin 빌드 ──────────────────────────────
echo "⚙️  [2/4] gcc 빌드 → gen-bin ..."
gcc -O2 -w -I "$RUNTIME_DIR" -o "$GEN_BIN" $GEN_A $RUNTIME_SRCS \
  -lm -lpthread -ldl -lsqlite3 2>&1 | grep -E "error:" | grep -v '"error"' | head -5 || true

if [ ! -f "$GEN_BIN" ]; then
  echo "❌ gen-bin 빌드 실패"
  rm -rf "$TMPDIR_WORK"
  exit 1
fi
echo "   gen-bin: $(du -sh "$GEN_BIN" | cut -f1)"

# ── Step 3: gen-bin으로 cgc-main.fl → gen-b.c ───────────────────
echo "⚙️  [3/4] gen-bin → gen-b.c (고정점 검증) ..."
"$GEN_BIN" "$CGC_MAIN" "$GEN_B" 2>&1 | grep -v "^\[FL Warn\]" | grep -v "^$" || true

# ── Step 4: SHA256 비교 ──────────────────────────────────────────
SHA_A=$(sha256sum "$GEN_A" | cut -d' ' -f1)
SHA_B=$(sha256sum "$GEN_B" | cut -d' ' -f1)

echo "⚙️  [4/4] 고정점 검증..."
echo "   gen-a SHA: $SHA_A"
echo "   gen-b SHA: $SHA_B"

if [ "$SHA_A" = "$SHA_B" ]; then
  echo ""
  echo "✅ 고정점 달성! ($SHA_A)"
  echo "   cgc-bin 교체 중..."
  cp "$GEN_BIN" "$CGC_BIN_PATH"
  chmod +x "$CGC_BIN_PATH"
  echo "✅ cgc-bin 교체 완료"
  echo "   새 SHA: $(sha256sum "$CGC_BIN_PATH" | cut -d' ' -f1)"
else
  echo ""
  echo "❌ 고정점 불일치 — cgc-bin 교체 취소"
  echo "   diff:"
  diff <(head -5 "$GEN_A") <(head -5 "$GEN_B") || true
  rm -rf "$TMPDIR_WORK"
  exit 1
fi

rm -rf "$TMPDIR_WORK"
echo ""
echo "🎉 완료! cgc-main.fl 변경 사항이 cgc-bin에 반영됐습니다."
