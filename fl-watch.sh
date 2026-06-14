#!/bin/bash
# fl-watch — 파일 변경 감지 → 자동 재실행
# 사용: fl-watch <file.fl> [args...]

SCRIPT_REAL="$(readlink -f "$0")"
SCRIPT_DIR="$(cd "$(dirname "$SCRIPT_REAL")" && pwd)"
FL_TEST="$SCRIPT_DIR/fl-test.sh"

FL_INPUT="$1"
if [ -z "$FL_INPUT" ]; then
  echo "사용: fl-watch <file.fl> [args...]"
  exit 1
fi
shift

echo "👀 감시 중: $FL_INPUT (Ctrl+C로 종료)"
echo "────────────────────────────────────"

run_once() {
  clear 2>/dev/null || true
  echo "▶ $(date '+%H:%M:%S') — $FL_INPUT"
  echo "────────────────────────────────────"
  bash "$FL_TEST" "$FL_INPUT" "$@" 2>&1
  EXIT=$?
  echo "────────────────────────────────────"
  if [ $EXIT -eq 0 ]; then
    echo "✅ 완료 (exit 0)"
  else
    echo "❌ 오류 (exit $EXIT)"
  fi
}

# 초기 실행
run_once "$@"

# inotifywait 없을 때를 위한 폴링 방식
if command -v inotifywait &>/dev/null; then
  # inotifywait 방식 (실시간)
  while inotifywait -q -e close_write "$FL_INPUT" 2>/dev/null; do
    run_once "$@"
  done
else
  # 폴링 방식 (1초 간격)
  LAST_MOD=$(stat -c %Y "$FL_INPUT" 2>/dev/null || echo 0)
  while true; do
    sleep 1
    CURR_MOD=$(stat -c %Y "$FL_INPUT" 2>/dev/null || echo 0)
    if [ "$CURR_MOD" != "$LAST_MOD" ]; then
      LAST_MOD=$CURR_MOD
      run_once "$@"
    fi
  done
fi
