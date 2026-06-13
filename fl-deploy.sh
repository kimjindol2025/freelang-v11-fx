#!/usr/bin/env bash
# fl-deploy — FreeLang 앱 PM2 자동 배포 도구
#
# 사용법:
#   fl-deploy [옵션] [앱_디렉토리]
#
# 옵션:
#   --build       배포 전 재빌드 (기본: 기존 바이너리 사용)
#   --reload      PM2 reload (기본: 신규 앱이면 start, 기존이면 reload)
#   --stop        앱 정지
#   --delete      PM2에서 삭제
#   --status      배포 상태만 출력
#   --dry-run     실제 실행 없이 계획만 출력
#
# 예시:
#   fl-deploy                   # 현재 디렉토리 배포
#   fl-deploy --build           # 빌드 후 배포
#   fl-deploy --status          # 상태 확인
#   fl-deploy my-app-dir        # 특정 디렉토리 배포

set -euo pipefail

SCRIPT_REAL="$(readlink -f "$0")"
SCRIPT_DIR="$(cd "$(dirname "$SCRIPT_REAL")" && pwd)"

PM2="$HOME/.npm-global/bin/pm2"
[[ -x "$PM2" ]] || PM2="$(which pm2 2>/dev/null || echo pm2)"

# ── 기본값 ────────────────────────────────────────────
APP_DIR="$(pwd)"
DO_BUILD=0
DO_RELOAD=0
DO_STOP=0
DO_DELETE=0
STATUS_ONLY=0
DRY_RUN=0

while [[ $# -gt 0 ]]; do
    case "$1" in
        --build)   DO_BUILD=1;   shift ;;
        --reload)  DO_RELOAD=1;  shift ;;
        --stop)    DO_STOP=1;    shift ;;
        --delete)  DO_DELETE=1;  shift ;;
        --status)  STATUS_ONLY=1; shift ;;
        --dry-run) DRY_RUN=1;   shift ;;
        -*)        echo "알 수 없는 옵션: $1"; exit 1 ;;
        *)         APP_DIR="$(realpath "$1")"; shift ;;
    esac
done

# ── .projectrc.json 읽기 ──────────────────────────────
RC="$APP_DIR/.projectrc.json"
if [[ ! -f "$RC" ]]; then
    echo "❌ .projectrc.json 없음. fl-new로 프로젝트 초기화하세요."
    exit 1
fi

APP_NAME=$(python3 -c "import json; d=json.load(open('$RC')); print(d.get('name','app'))")
APP_VERSION=$(python3 -c "import json; d=json.load(open('$RC')); print(d.get('version','1.0.0'))")
APP_PORT=$(python3 -c "import json; d=json.load(open('$RC')); p=d.get('ports',[]); print(p[0] if p else 8080)")
APP_ENTRY=$(python3 -c "import json; d=json.load(open('$RC')); print(d.get('entry','server.fl'))")

BIN_NAME="$APP_NAME"
BIN_PATH="$APP_DIR/$BIN_NAME"

# ── 상태 확인 함수 ─────────────────────────────────────
pm2_status() {
    $PM2 jlist 2>/dev/null | python3 -c "
import sys, json
apps = json.load(sys.stdin)
for a in apps:
    if a['name'] == '$APP_NAME':
        s = a.get('pm2_env', {}).get('status', 'unknown')
        pid = a.get('pid', '-')
        mem = a.get('monit', {}).get('memory', 0)
        cpu = a.get('monit', {}).get('cpu', 0)
        print(f'{s}|{pid}|{mem}|{cpu}')
        sys.exit(0)
print('not_found')
" 2>/dev/null || echo "not_found"
}

# ── 상태만 출력 ───────────────────────────────────────
if [[ $STATUS_ONLY -eq 1 ]]; then
    STATUS=$(pm2_status)
    if [[ "$STATUS" == "not_found" ]]; then
        echo "ℹ️  $APP_NAME: PM2에 없음"
    else
        IFS='|' read -r ST PID MEM CPU <<< "$STATUS"
        MEM_MB=$(( ${MEM:-0} / 1024 / 1024 ))
        echo "📊 $APP_NAME v$APP_VERSION"
        echo "   상태: $ST  PID: $PID  메모리: ${MEM_MB}MB  CPU: ${CPU}%"
        echo "   포트: $APP_PORT"
        # 헬스체크
        if curl -s --max-time 2 "http://localhost:$APP_PORT/health" > /dev/null 2>&1; then
            echo "   헬스: ✅ 정상"
        else
            echo "   헬스: ⚠️  /health 응답 없음"
        fi
    fi
    exit 0
fi

# ── 정지/삭제 ────────────────────────────────────────
if [[ $DO_STOP -eq 1 ]]; then
    echo "⏹  $APP_NAME 정지..."
    [[ $DRY_RUN -eq 0 ]] && $PM2 stop "$APP_NAME" 2>/dev/null || true
    echo "✅ 정지됨"
    exit 0
fi

if [[ $DO_DELETE -eq 1 ]]; then
    echo "🗑️  $APP_NAME PM2에서 삭제..."
    [[ $DRY_RUN -eq 0 ]] && $PM2 delete "$APP_NAME" 2>/dev/null || true
    echo "✅ 삭제됨"
    exit 0
fi

echo "🚀 배포: $APP_NAME v$APP_VERSION → 포트 $APP_PORT"
[[ $DRY_RUN -eq 1 ]] && echo "   [dry-run 모드 — 실행 없음]"

# ── 빌드 ─────────────────────────────────────────────
if [[ $DO_BUILD -eq 1 ]]; then
    echo ""
    echo "🔨 빌드 중..."
    if [[ $DRY_RUN -eq 0 ]]; then
        cd "$APP_DIR"
        "$SCRIPT_DIR/fl-build.sh" "$APP_ENTRY" "$BIN_NAME"
    else
        echo "   [dry] fl-build.sh $APP_ENTRY $BIN_NAME"
    fi
fi

# ── 바이너리 확인 ─────────────────────────────────────
if [[ ! -f "$BIN_PATH" ]]; then
    echo "❌ 바이너리 없음: $BIN_PATH"
    echo "   먼저 빌드하세요: fl-deploy --build"
    exit 1
fi

BIN_SIZE=$(stat -c%s "$BIN_PATH")
echo "   바이너리: $BIN_PATH (${BIN_SIZE}B)"

# ── 포트 충돌 확인 ────────────────────────────────────
if ss -tlnp 2>/dev/null | grep -q ":$APP_PORT " ; then
    PORT_PROC=$(ss -tlnp 2>/dev/null | grep ":$APP_PORT " | awk '{print $NF}' | head -1)
    echo "   포트 $APP_PORT: 이미 사용 중 ($PORT_PROC)"
fi

# ── PM2 상태 확인 ─────────────────────────────────────
STATUS=$(pm2_status)

if [[ "$STATUS" == "not_found" ]]; then
    echo ""
    echo "➕ 신규 등록..."
    if [[ $DRY_RUN -eq 0 ]]; then
        $PM2 start "$BIN_PATH" \
            --name "$APP_NAME" \
            --cwd "$APP_DIR" \
            --interpreter none \
            --env "PORT=$APP_PORT,FL_DEBUG=${FL_DEBUG:-0}"
        sleep 1
    else
        echo "   [dry] pm2 start $BIN_PATH --name $APP_NAME --cwd $APP_DIR"
    fi
else
    IFS='|' read -r ST PID MEM CPU <<< "$STATUS"
    echo "   기존 상태: $ST (PID $PID)"
    echo ""
    echo "🔄 reload..."
    if [[ $DRY_RUN -eq 0 ]]; then
        # 바이너리 교체 후 reload
        $PM2 reload "$APP_NAME" 2>/dev/null || $PM2 restart "$APP_NAME"
        sleep 1
    else
        echo "   [dry] pm2 reload $APP_NAME"
    fi
fi

# ── 헬스체크 ─────────────────────────────────────────
if [[ $DRY_RUN -eq 0 ]]; then
    echo ""
    echo "🏥 헬스체크 (최대 10초)..."
    for i in $(seq 1 10); do
        if curl -s --max-time 1 "http://localhost:$APP_PORT/health" > /dev/null 2>&1; then
            echo "   ✅ $i초 후 응답"
            break
        fi
        if [[ $i -eq 10 ]]; then
            echo "   ⚠️  /health 응답 없음 (포트 $APP_PORT)"
            echo "   힌트: GET /health 라우트가 없으면 정상"
        fi
        sleep 1
    done
fi

# ── pm2 save ─────────────────────────────────────────
if [[ $DRY_RUN -eq 0 ]]; then
    $PM2 save --force > /dev/null 2>&1 || true
fi

# ── 최종 상태 ─────────────────────────────────────────
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
if [[ $DRY_RUN -eq 0 ]]; then
    STATUS2=$(pm2_status)
    IFS='|' read -r ST2 PID2 MEM2 CPU2 <<< "$STATUS2"
    MEM2_MB=$(( ${MEM2:-0} / 1024 / 1024 ))
    echo "✅ 배포 완료"
    echo "   앱: $APP_NAME  상태: $ST2  PID: $PID2  메모리: ${MEM2_MB}MB"
    echo "   포트: http://localhost:$APP_PORT"
else
    echo "✅ dry-run 완료 (실제 변경 없음)"
fi
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
