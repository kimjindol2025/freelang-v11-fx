#!/usr/bin/env bash
# fl-rollback — FreeLang 앱 롤백 도구
#
# 사용법:
#   fl-rollback [앱_디렉토리]         마지막 버전으로 롤백
#   fl-rollback --list [앱_디렉토리]  사용 가능한 버전 목록
#   fl-rollback --to <backup> [dir]   특정 백업으로 롤백
#
# 백업 위치: <앱_디렉토리>/.backups/<앱명>.<타임스탬프>.bak
#
# fl-deploy는 배포 전 자동으로 백업을 생성합니다.
# 긴급 롤백: fl-rollback (1초 내 완료)

set -euo pipefail

SCRIPT_REAL="$(readlink -f "$0")"
SCRIPT_DIR="$(cd "$(dirname "$SCRIPT_REAL")" && pwd)"

PM2="$HOME/.npm-global/bin/pm2"
[[ -x "$PM2" ]] || PM2="$(which pm2 2>/dev/null || echo pm2)"

APP_DIR="$(pwd)"
LIST_MODE=0
TARGET_BACKUP=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --list) LIST_MODE=1; shift ;;
        --to)   TARGET_BACKUP="$2"; shift 2 ;;
        -*)     echo "알 수 없는 옵션: $1"; exit 1 ;;
        *)      APP_DIR="$(realpath "$1")"; shift ;;
    esac
done

RC="$APP_DIR/.projectrc.json"
if [[ ! -f "$RC" ]]; then
    echo "❌ .projectrc.json 없음"
    exit 1
fi

APP_NAME=$(python3 -c "import json; print(json.load(open('$RC'))['name'])")
BIN_PATH="$APP_DIR/$APP_NAME"
BACKUP_DIR="$APP_DIR/.backups"

# ── 목록 출력 ─────────────────────────────────────────
list_backups() {
    if [[ ! -d "$BACKUP_DIR" ]]; then
        echo "ℹ️  백업 없음: $BACKUP_DIR"
        exit 0
    fi

    echo "📋 $APP_NAME 백업 목록:"
    echo ""
    ls -lt "$BACKUP_DIR/${APP_NAME}."*.bak 2>/dev/null | head -10 | while read -r perm n user grp size mnt day time file; do
        fname="$(basename "$file")"
        ts=$(echo "$fname" | sed "s/${APP_NAME}\.//;s/\.bak//")
        echo "  $ts  (${size}B)  $file"
    done || echo "  백업 없음"
}

if [[ $LIST_MODE -eq 1 ]]; then
    list_backups
    exit 0
fi

# ── 롤백 실행 ─────────────────────────────────────────
if [[ ! -d "$BACKUP_DIR" ]]; then
    echo "❌ 백업 디렉토리 없음: $BACKUP_DIR"
    echo "   fl-deploy --build 로 배포하면 자동 생성됩니다."
    exit 1
fi

# 복원할 백업 선택
if [[ -n "$TARGET_BACKUP" ]]; then
    RESTORE_FROM="$TARGET_BACKUP"
    if [[ ! -f "$RESTORE_FROM" ]]; then
        RESTORE_FROM="$BACKUP_DIR/$TARGET_BACKUP"
    fi
else
    # 가장 최신 백업
    RESTORE_FROM=$(ls -t "$BACKUP_DIR/${APP_NAME}."*.bak 2>/dev/null | head -1)
    if [[ -z "$RESTORE_FROM" ]]; then
        echo "❌ 복원할 백업 없음"
        exit 1
    fi
fi

if [[ ! -f "$RESTORE_FROM" ]]; then
    echo "❌ 백업 파일 없음: $RESTORE_FROM"
    exit 1
fi

BNAME=$(basename "$RESTORE_FROM")
TS=$(echo "$BNAME" | sed "s/${APP_NAME}\.//;s/\.bak//")

echo "⚡ 롤백: $APP_NAME"
echo "   복원: $BNAME ($TS)"
echo ""

# 현재 바이너리 백업 (롤백 후 롤포워드용)
if [[ -f "$BIN_PATH" ]]; then
    FWDBAK="$BACKUP_DIR/${APP_NAME}.rollback-$(date +%Y%m%d-%H%M%S).bak"
    cp "$BIN_PATH" "$FWDBAK"
    echo "   현재 버전 보존: $(basename "$FWDBAK")"
fi

# 복원
cp "$RESTORE_FROM" "$BIN_PATH"
chmod +x "$BIN_PATH"
echo "   ✅ 바이너리 복원 완료"

# PM2 reload
echo ""
echo "🔄 PM2 reload..."
if $PM2 list 2>/dev/null | grep -q "$APP_NAME"; then
    $PM2 reload "$APP_NAME" 2>&1 | grep -E "✓|✗|error" || true
    sleep 1

    # 헬스체크
    PORT=$(python3 -c "
import json
d = json.load(open('$RC'))
ports = d.get('ports', [])
print(ports[0] if ports else '')
")
    if [[ -n "$PORT" ]]; then
        for i in $(seq 1 5); do
            if curl -s --max-time 1 "http://localhost:$PORT/health" > /dev/null 2>&1; then
                echo "   헬스체크: ✅ 정상 (${i}초)"
                break
            fi
            [[ $i -eq 5 ]] && echo "   헬스체크: ⚠️  응답 없음 (포트 $PORT)"
            sleep 1
        done
    fi
    $PM2 save --force > /dev/null 2>&1 || true
else
    echo "   ℹ️  PM2에 $APP_NAME 없음 — 직접 시작 필요"
fi

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "✅ 롤백 완료: $APP_NAME → $TS"
echo ""
echo "   롤포워드(원복): fl-rollback --to $(basename "$FWDBAK" 2>/dev/null || echo "이전백업") $APP_DIR"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
