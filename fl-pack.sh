#!/usr/bin/env bash
# fl-pack — FreeLang 네이티브 앱 패키징 도구
#
# 사용법:
#   fl-pack [옵션] [앱_디렉토리]
#
# 옵션:
#   --static      정적 링크 (이식성 최대, 크기 증가)
#   --strip       심볼 제거 (크기 최소화)
#   --tar         .tar.gz 아카이브 생성
#   --deb         .deb 패키지 생성 (experimental)
#   --out <dir>   출력 디렉토리 (기본: ./dist)
#
# 예시:
#   fl-pack                         # 현재 디렉토리 패키징
#   fl-pack --strip --tar           # 스트립 + 압축
#   fl-pack --static --out /tmp     # 정적 빌드

set -euo pipefail

SCRIPT_REAL="$(readlink -f "$0")"
SCRIPT_DIR="$(cd "$(dirname "$SCRIPT_REAL")" && pwd)"

# ── 기본값 ────────────────────────────────────────────
APP_DIR="$(pwd)"
STATIC=0
STRIP=0
TAR=0
DEB=0
OUT_DIR=""

# ── 인수 파싱 ─────────────────────────────────────────
while [[ $# -gt 0 ]]; do
    case "$1" in
        --static) STATIC=1; shift ;;
        --strip)  STRIP=1;  shift ;;
        --tar)    TAR=1;    shift ;;
        --deb)    DEB=1;    shift ;;
        --out)    OUT_DIR="$2"; shift 2 ;;
        -*)       echo "알 수 없는 옵션: $1"; exit 1 ;;
        *)        APP_DIR="$(realpath "$1")"; shift ;;
    esac
done

# ── .projectrc.json 읽기 ──────────────────────────────
RC="$APP_DIR/.projectrc.json"
if [[ ! -f "$RC" ]]; then
    echo "❌ .projectrc.json 없음: $RC"
    echo "   fl-new으로 생성하거나 직접 작성하세요."
    exit 1
fi

# Python으로 JSON 파싱
APP_NAME=$(python3 -c "import json,sys; d=json.load(open('$RC')); print(d.get('name','app'))")
APP_VERSION=$(python3 -c "import json,sys; d=json.load(open('$RC')); print(d.get('version','1.0.0'))")
APP_PORT=$(python3 -c "import json,sys; d=json.load(open('$RC')); p=d.get('ports',[]); print(p[0] if p else 8080)")
APP_ENTRY=$(python3 -c "import json,sys; d=json.load(open('$RC')); print(d.get('entry','server.fl'))")

echo "📦 패키징: $APP_NAME v$APP_VERSION (포트 $APP_PORT)"
echo "   진입점: $APP_ENTRY"

# ── 출력 디렉토리 ─────────────────────────────────────
if [[ -z "$OUT_DIR" ]]; then
    OUT_DIR="$APP_DIR/dist"
fi
PACK_DIR="$OUT_DIR/$APP_NAME-$APP_VERSION"
mkdir -p "$PACK_DIR"

# ── 1단계: 빌드 ───────────────────────────────────────
echo ""
echo "🔨 [1/4] 빌드 중..."
cd "$APP_DIR"
FL_ENTRY="$APP_ENTRY"
BIN_NAME="$APP_NAME"

"$SCRIPT_DIR/fl-build.sh" "$FL_ENTRY" "$BIN_NAME" 2>&1

if [[ ! -f "$APP_DIR/$BIN_NAME" ]]; then
    echo "❌ 빌드 실패: $APP_DIR/$BIN_NAME 없음"
    exit 1
fi

BIN_SIZE=$(stat -c%s "$APP_DIR/$BIN_NAME")
echo "   ✅ 빌드 완료: $BIN_NAME (${BIN_SIZE} bytes)"

# ── 2단계: 선택적 처리 ───────────────────────────────
if [[ $STRIP -eq 1 ]]; then
    echo ""
    echo "✂️  [2/4] 심볼 제거..."
    strip --strip-all "$APP_DIR/$BIN_NAME"
    STRIPPED_SIZE=$(stat -c%s "$APP_DIR/$BIN_NAME")
    echo "   ${BIN_SIZE}B → ${STRIPPED_SIZE}B ($(( (BIN_SIZE - STRIPPED_SIZE) * 100 / BIN_SIZE ))% 감소)"
    BIN_SIZE=$STRIPPED_SIZE
else
    echo ""
    echo "⏭️  [2/4] strip 건너뜀 (--strip 옵션 없음)"
fi

# ── 3단계: 패키지 디렉토리 구성 ──────────────────────
echo ""
echo "📁 [3/4] 패키지 구성..."

# 바이너리
cp "$APP_DIR/$BIN_NAME" "$PACK_DIR/$BIN_NAME"
chmod +x "$PACK_DIR/$BIN_NAME"

# 메타데이터
cp "$RC" "$PACK_DIR/.projectrc.json"

# 실행 스크립트
cat > "$PACK_DIR/run.sh" << RUNSCRIPT
#!/usr/bin/env bash
# $APP_NAME 실행 스크립트
DIR="\$(cd "\$(dirname "\$0")" && pwd)"
export PORT="\${PORT:-$APP_PORT}"
exec "\$DIR/$BIN_NAME"
RUNSCRIPT
chmod +x "$PACK_DIR/run.sh"

# PM2 ecosystem 파일
cat > "$PACK_DIR/ecosystem.config.js" << ECOSYS
module.exports = {
  apps: [{
    name: '$APP_NAME',
    script: './$BIN_NAME',
    cwd: __dirname,
    env: {
      PORT: $APP_PORT,
      FL_DEBUG: process.env.FL_DEBUG || '0'
    },
    interpreter: 'none',
    autorestart: true,
    max_restarts: 10,
    restart_delay: 1000,
    log_date_format: 'YYYY-MM-DD HH:mm:ss'
  }]
};
ECOSYS

# README
cat > "$PACK_DIR/README.md" << README
# $APP_NAME v$APP_VERSION

FreeLang 네이티브 바이너리 패키지

## 실행

\`\`\`bash
# 직접 실행
./run.sh

# PM2로 실행
pm2 start ecosystem.config.js

# 환경변수로 포트 변경
PORT=40999 ./run.sh
\`\`\`

## 디버그 모드

\`\`\`bash
FL_DEBUG=1 ./run.sh          # 요청/응답 로그
FL_DEBUG=2 ./run.sh          # 헤더 + body + SQL
FL_DEBUG=3 ./run.sh          # 최상세
FL_LOG_FILE=app.log ./run.sh # 파일 출력
\`\`\`

## 빌드 정보

- 런타임: FreeLang 네이티브 C
- 바이너리: \`$BIN_NAME\` ($(echo "$BIN_SIZE" | numfmt --to=iec 2>/dev/null || echo "${BIN_SIZE}B"))
- 포트: $APP_PORT
README

echo "   ✅ 구성 완료"
ls -la "$PACK_DIR/"

# ── 4단계: 아카이브 ───────────────────────────────────
if [[ $TAR -eq 1 ]]; then
    echo ""
    echo "🗜️  [4/4] 압축..."
    TARBALL="$OUT_DIR/${APP_NAME}-${APP_VERSION}-linux-amd64.tar.gz"
    tar -czf "$TARBALL" -C "$OUT_DIR" "$APP_NAME-$APP_VERSION"
    TAR_SIZE=$(stat -c%s "$TARBALL")
    echo "   ✅ $TARBALL (${TAR_SIZE} bytes)"
else
    echo ""
    echo "⏭️  [4/4] 압축 건너뜀 (--tar 옵션 없음)"
fi

# ── 완료 ──────────────────────────────────────────────
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "✅ 패키징 완료: $PACK_DIR"
echo ""
echo "   배포 명령:"
echo "   pm2 start $PACK_DIR/ecosystem.config.js"
echo ""
echo "   직접 실행:"
echo "   $PACK_DIR/run.sh"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
