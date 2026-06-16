#!/bin/bash
# fl-build — FreeLang 네이티브 빌드 스크립트
# 사용법: fl-build.sh <input.fl> [output-binary]
# 결과:   Node.js 없는 단일 ELF 바이너리

set -e

# 심링크 지원: 실제 스크립트 위치 해석
SCRIPT_REAL="$(readlink -f "$0")"
SCRIPT_DIR="$(cd "$(dirname "$SCRIPT_REAL")" && pwd)"
RUNTIME_DIR="$SCRIPT_DIR/runtime"
CGC_BIN="/home/kimjin/freelang-v11/bin/cgc-bin"

FL_INPUT="$1"
if [ -z "$FL_INPUT" ]; then
  echo "사용법: $0 <input.fl> [output-name]"
  exit 1
fi

FL_BASE="$(basename "$FL_INPUT" .fl)"
OUTPUT="${2:-$FL_BASE}"
C_FILE="/tmp/fl_build_$$.c"

echo "🔨 FreeLang 네이티브 빌드"
echo "   입력: $FL_INPUT"
echo "   출력: $OUTPUT"
echo ""

# ─── 1. load 인라인 전처리 ───────────────────────────────────────
# (load "path.fl") 를 파일 내용으로 인라인 치환
PREPROCESSED="/tmp/fl_preprocessed_$$.fl"

python3 << PYEOF
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
        print(f"; [fl-build] 경고: {path} 읽기 실패", file=sys.stderr)
        return ""
    result = []
    for line in content.splitlines():
        # (load "...") 또는 (load '...')
        m = re.match(r'\s*\(load\s+"([^"]+)"\)', line) or \
            re.match(r"\s*\(load\s+'([^']+)'\)", line)
        if m:
            load_path = m.group(1)
            if not os.path.isabs(load_path):
                load_path = os.path.join(base_dir, load_path)
            print(f"; [fl-build] 인라인: {load_path}", file=sys.stderr)
            result.append(f"; --- inlined: {load_path} ---")
            result.append(inline_loads(load_path, visited))
            result.append(f"; --- end inlined: {load_path} ---")
        else:
            result.append(line)
    return "\n".join(result)

output = inline_loads("$FL_INPUT")

# (println "[DEBUG] ...") 제거 (노이즈 줄이기)
lines = output.splitlines()
cleaned = [l for l in lines if not re.match(r'\s*\(println\s+"?\[DEBUG\]', l)]
output = "\n".join(cleaned)

open("$PREPROCESSED", "w").write(output)
print(f"[fl-build] 전처리 완료", file=sys.stderr)
PYEOF

# ─── 2. cgc-bin: FL → C ──────────────────────────────────────────
echo "⚙️  FL → C 컴파일..."
"$CGC_BIN" "$PREPROCESSED" "$C_FILE" 2>&1 | grep -v "^$" || true

# ─── 3. gcc: C → ELF ─────────────────────────────────────────────
echo "⚙️  C → 바이너리 컴파일..."
RUNTIME_SRCS="$RUNTIME_DIR/core.c $RUNTIME_DIR/collection.c $RUNTIME_DIR/io.c \
  $RUNTIME_DIR/json.c $RUNTIME_DIR/math.c $RUNTIME_DIR/process.c \
  $RUNTIME_DIR/error.c $RUNTIME_DIR/http.c $RUNTIME_DIR/aliases.c \
  $RUNTIME_DIR/sqlite.c $RUNTIME_DIR/debug.c $RUNTIME_DIR/gc.c \
  $RUNTIME_DIR/jit.c $RUNTIME_DIR/fx-builtin-shim.c \
  $RUNTIME_DIR/websocket.c $RUNTIME_DIR/http_client.c \
  $RUNTIME_DIR/regex.c $RUNTIME_DIR/smtp.c"

# mariadb.c는 dlopen 방식이라 헤더 불필요 — 항상 포함
if [ -f "$RUNTIME_DIR/mariadb.c" ]; then
  RUNTIME_SRCS="$RUNTIME_SRCS $RUNTIME_DIR/mariadb.c"
  echo "   + MariaDB dlopen 바인딩 포함"
fi

GCC_LOG="/tmp/fl_gcc_$$.log"
if gcc -O2 -Werror=implicit-function-declaration -o "$OUTPUT" $C_FILE $RUNTIME_SRCS \
  -I "$RUNTIME_DIR" \
  -rdynamic -lpthread -lm -ldl -lsqlite3 -lssl -lcrypto -lcurl \
  -w 2>"$GCC_LOG"; then
  rm -f "$GCC_LOG"
else
  echo "❌ gcc 컴파일 실패:"
  cat "$GCC_LOG"
  rm -f "$C_FILE" "$PREPROCESSED" "$GCC_LOG"
  exit 1
fi

# ─── 4. 정리 ─────────────────────────────────────────────────────
rm -f "$C_FILE" "$PREPROCESSED"

echo ""
echo "✅ 빌드 완료: ./$OUTPUT"
echo "   크기: $(du -sh "$OUTPUT" | cut -f1)"
echo "   실행: ./$OUTPUT"
