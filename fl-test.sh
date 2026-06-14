#!/bin/bash
# fl-test — FL 코드 즉시 실행 (compile + run, 노이즈 없음)
# 사용: fl-test <file.fl> [args...]
# 옵션: FL_SHOW_C=1 fl-test <file.fl>  → 생성된 C 코드 출력

set -e

SCRIPT_REAL="$(readlink -f "$0")"
SCRIPT_DIR="$(cd "$(dirname "$SCRIPT_REAL")" && pwd)"
RUNTIME_DIR="$SCRIPT_DIR/runtime"
CGC_BIN="/home/kimjin/freelang-v11/bin/cgc-bin"

FL_INPUT="$1"
if [ -z "$FL_INPUT" ]; then
  echo "사용: fl-test <file.fl> [args...]"
  echo "      FL_SHOW_C=1 fl-test <file.fl>  # C 코드 출력"
  exit 1
fi
shift  # 나머지 인자는 프로그램에 전달

C_FILE=$(mktemp /tmp/fl_test_XXXXXX.c)
BIN_FILE=$(mktemp /tmp/fl_test_XXXXXX)

# ── 전처리: (load "...") 인라인 ──────────────────────────────────
PREPROCESSED=$(mktemp /tmp/fl_pre_XXXXXX.fl)
python3 - "$FL_INPUT" "$PREPROCESSED" << 'PYEOF'
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

output = inline_loads(sys.argv[1])
open(sys.argv[2], "w").write(output)
PYEOF

# ── FL → C ───────────────────────────────────────────────────────
COMPILE_ERR=$("$CGC_BIN" "$PREPROCESSED" "$C_FILE" 2>&1 | grep -v "^\[FL Warn\]" | grep -v "^$" | grep -v "^Compiled" || true)
if [ -n "$COMPILE_ERR" ]; then
  echo "❌ 컴파일 오류:"
  echo "$COMPILE_ERR"
  rm -f "$C_FILE" "$BIN_FILE" "$PREPROCESSED"
  exit 1
fi

# C 코드 보기 옵션
if [ "${FL_SHOW_C:-0}" = "1" ]; then
  echo "=== 생성된 C 코드 ==="
  cat "$C_FILE"
  echo "===================="
fi

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
  rm -f "$C_FILE" "$BIN_FILE" "$PREPROCESSED"
  exit 1
fi

rm -f "$C_FILE" "$PREPROCESSED"
chmod +x "$BIN_FILE"

# ── 실행 ─────────────────────────────────────────────────────────
"$BIN_FILE" "$@"
EXIT_CODE=$?
rm -f "$BIN_FILE"
exit $EXIT_CODE
