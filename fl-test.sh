#!/bin/bash
# fl-test — FL 코드 즉시 실행 (compile + run, 노이즈 없음)
# 사용: fl-test [옵션] <file.fl> [args...]
# 옵션:
#   --std        fx-std.fl 자동 로드 (load 없이 기본 라이브러리 사용)
#   --time       실행 시간 측정
#   --show-c     생성된 C 코드 출력 (FL_SHOW_C=1 과 동일)
#   --check      빌드만 하고 실행 안 함 (문법 검사용)

SCRIPT_REAL="$(readlink -f "$0")"
SCRIPT_DIR="$(cd "$(dirname "$SCRIPT_REAL")" && pwd)"
RUNTIME_DIR="$SCRIPT_DIR/runtime"
CGC_BIN="/home/kimjin/freelang-v11/bin/cgc-bin"
FX_STD="$SCRIPT_DIR/fx-std.fl"

# ── 옵션 파싱 ─────────────────────────────────────────────────────
USE_STD=0
SHOW_TIME=0
SHOW_C="${FL_SHOW_C:-0}"
CHECK_ONLY=0

while [[ "$1" =~ ^-- ]]; do
  case "$1" in
    --std)     USE_STD=1;  shift ;;
    --time)    SHOW_TIME=1; shift ;;
    --show-c)  SHOW_C=1;   shift ;;
    --check)   CHECK_ONLY=1; shift ;;
    *) break ;;
  esac
done

FL_INPUT="$1"
if [ -z "$FL_INPUT" ]; then
  echo "사용: fl-test [--std] [--time] [--show-c] [--check] <file.fl> [args...]"
  exit 1
fi
shift

C_FILE=$(mktemp /tmp/fl_test_XXXXXX.c)
BIN_FILE=$(mktemp /tmp/fl_test_XXXXXX)
PREPROCESSED=$(mktemp /tmp/fl_pre_XXXXXX.fl)

cleanup() { rm -f "$C_FILE" "$BIN_FILE" "$PREPROCESSED"; }
trap cleanup EXIT

# ── 전처리: (load "...") 인라인 + --std prepend ──────────────────
STD_PREPEND=""
if [ "$USE_STD" = "1" ] && [ -f "$FX_STD" ]; then
  STD_PREPEND="$FX_STD"
fi

python3 - "$FL_INPUT" "$PREPROCESSED" "$STD_PREPEND" << 'PYEOF'
import re, os, sys

def inline_loads(path, visited=None):
    if visited is None:
        visited = set()
    abs_path = os.path.abspath(path)
    if abs_path in visited:
        return ""
    visited.add(abs_path)
    base_dir = os.path.dirname(abs_path)
    try:
        content = open(abs_path).read()
    except:
        return ""
    result = []
    for line in content.splitlines():
        m = re.match(r'\s*\(load\s+"([^"]+)"\)', line)
        if m:
            load_path = m.group(1)
            if not os.path.isabs(load_path):
                load_path = os.path.join(base_dir, load_path)
            result.append(inline_loads(load_path, visited))
        else:
            result.append(line)
    return "\n".join(result)

std_path = sys.argv[3] if len(sys.argv) > 3 else ""
parts = []
if std_path:
    parts.append(inline_loads(std_path))
parts.append(inline_loads(sys.argv[1]))
open(sys.argv[2], "w").write("\n".join(parts))
PYEOF

# ── FL → C ───────────────────────────────────────────────────────
COMPILE_ERR=$("$CGC_BIN" "$PREPROCESSED" "$C_FILE" 2>&1 \
  | grep -v "^\[FL Warn\]" | grep -v "^$" | grep -v "^Compiled" || true)
if [ -n "$COMPILE_ERR" ]; then
  echo "❌ 컴파일 오류:"
  echo "$COMPILE_ERR"
  exit 1
fi

if [ "$SHOW_C" = "1" ]; then
  echo "=== 생성된 C 코드 ==="
  cat "$C_FILE"
  echo "===================="
fi

[ "$CHECK_ONLY" = "1" ] && { echo "✅ 문법 OK"; exit 0; }

# ── C → 바이너리 ─────────────────────────────────────────────────
RUNTIME_SRCS="$RUNTIME_DIR/core.c $RUNTIME_DIR/collection.c $RUNTIME_DIR/io.c \
  $RUNTIME_DIR/math.c $RUNTIME_DIR/error.c $RUNTIME_DIR/process.c \
  $RUNTIME_DIR/json.c $RUNTIME_DIR/aliases.c \
  $RUNTIME_DIR/gc.c $RUNTIME_DIR/http.c $RUNTIME_DIR/sqlite.c \
  $RUNTIME_DIR/mariadb.c $RUNTIME_DIR/debug.c"

BUILD_ERR=$(gcc -O0 -w -I "$RUNTIME_DIR" -o "$BIN_FILE" "$C_FILE" $RUNTIME_SRCS \
  -lm -lpthread -ldl -lsqlite3 2>&1 | grep "error:" | head -5 || true)
if [ -n "$BUILD_ERR" ]; then
  echo "❌ 빌드 오류:"
  echo "$BUILD_ERR"
  exit 1
fi

chmod +x "$BIN_FILE"

# ── 실행 ("중지됨" 메시지 억제) ──────────────────────────────────
if [ "$SHOW_TIME" = "1" ]; then
  START_NS=$(date +%s%N)
fi

# job control 끄기 → SIGABRT 발생해도 "중지됨" 메시지 없음
set +m
"$BIN_FILE" "$@" &
CHILD_PID=$!
wait "$CHILD_PID" 2>/dev/null
EXIT_CODE=$?

if [ "$SHOW_TIME" = "1" ]; then
  END_NS=$(date +%s%N)
  ELAPSED=$(( (END_NS - START_NS) / 1000000 ))
  echo "⏱  ${ELAPSED}ms"
fi

exit $EXIT_CODE
