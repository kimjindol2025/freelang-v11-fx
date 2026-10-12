#!/bin/bash
# fl-build — FreeLang 네이티브 빌드 스크립트
# 사용법: fl-build.sh <input.fl> [output-binary]
# 결과:   Node.js 없는 단일 ELF 바이너리

set -e

# 심링크 지원: 실제 스크립트 위치 해석
SCRIPT_REAL="$(readlink -f "$0")"
SCRIPT_DIR="$(cd "$(dirname "$SCRIPT_REAL")" && pwd)"
RUNTIME_DIR="$SCRIPT_DIR/runtime"

# CGC 경로: 환경변수 → 로컬 후보 → 구 경로. 없으면 즉시 실패.
if [ -z "${CGC_BIN:-}" ]; then
  for candidate in \
    "$SCRIPT_DIR/../freelang-afj/bin/cgc-bin" \
    "/home/kim/kim/platform/freelang-afj/bin/cgc-bin" \
    "/home/kimjin/freelang-v11/bin/cgc-bin"
  do
    if [ -x "$candidate" ]; then
      CGC_BIN="$candidate"
      break
    fi
  done
fi
if [ -z "${CGC_BIN:-}" ] || [ ! -x "$CGC_BIN" ]; then
  echo "❌ cgc-bin 없음. CGC_BIN=/path/to/cgc-bin 을 지정하세요." >&2
  exit 1
fi

# --no-net: openssl/curl 없이 stub
# --bundle=core,net,db,mail : opt-in 선택 링크 (없으면 풀셋). docs/FX-BUNDLE-MAP.md
# --strict-names: kebab API → cgc 거부 (기본은 경고만). docs/FX-NAME-CARD.md
NO_NET=0
BUNDLE_SPEC=""
STRICT_NAMES=0
ARGS=()
for arg in "$@"; do
  case "$arg" in
    --no-net) NO_NET=1 ;;
    --strict-names) STRICT_NAMES=1 ;;
    --bundle=*) BUNDLE_SPEC="${arg#--bundle=}" ;;
    --bundle)
      echo "❌ --bundle=core,net,db,mail 형식으로 쓰세요" >&2
      exit 1
      ;;
    *) ARGS+=("$arg") ;;
  esac
done

FL_INPUT="${ARGS[0]}"
if [ -z "$FL_INPUT" ]; then
  echo "사용법: $0 <input.fl> [output-name] [--no-net] [--bundle=core,net,db,mail] [--strict-names]"
  exit 1
fi

bundle_has() {
  # empty BUNDLE_SPEC → 풀셋 (모든 묶음 yes)
  if [ -z "$BUNDLE_SPEC" ]; then return 0; fi
  local want="$1"
  echo ",$BUNDLE_SPEC," | grep -qi ",${want},"
}

FL_BASE="$(basename "$FL_INPUT" .fl)"
OUTPUT="${ARGS[1]:-$FL_BASE}"
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

# ─── 2. 사전 검사 ────────────────────────────────────────────────
if [ -z "${CHECK_PARENS:-}" ]; then
  for candidate in \
    "$SCRIPT_DIR/../freelang-v11/scripts/check-parens.py" \
    "/home/kim/kim/platform/freelang-v11/scripts/check-parens.py" \
    "/home/kimjin/freelang-v11/scripts/check-parens.py"
  do
    if [ -f "$candidate" ]; then
      CHECK_PARENS="$candidate"
      break
    fi
  done
fi
if [ -n "${CHECK_PARENS:-}" ] && [ -f "$CHECK_PARENS" ]; then
  echo "🔍 괄호/브래킷 검사..."
  if ! python3 "$CHECK_PARENS" "$PREPROCESSED" 2>&1; then
    echo "❌ 사전 검사 실패 — 컴파일 중단"
    rm -f "$PREPROCESSED"
    exit 1
  fi
fi

# ─── 3. cgc-bin: FL → C ──────────────────────────────────────────
echo "⚙️  FL → C 컴파일..."
CGC_ARGS=()
if [ "$STRICT_NAMES" = "1" ]; then
  CGC_ARGS+=(--strict-names)
  echo "   + --strict-names (kebab API 거부)"
fi
set +e
COMPILE_OUT=$("$CGC_BIN" "${CGC_ARGS[@]}" "$PREPROCESSED" "$C_FILE" 2>&1)
CGC_EC=$?
set -e
echo "$COMPILE_OUT" | grep -v "^$" || true

# naming / W1/W2/W3 경고 강조
NAME_WARN_COUNT=$(echo "$COMPILE_OUT" | grep -c " | naming | " || true)
WARN_COUNT=$(echo "$COMPILE_OUT" | grep -c "\[W[123]\]" || true)
if [ "$NAME_WARN_COUNT" -gt 0 ] || [ "$WARN_COUNT" -gt 0 ]; then
  echo ""
  echo "⚠️  컴파일 경고 naming=${NAME_WARN_COUNT} W=${WARN_COUNT}:"
  echo "$COMPILE_OUT" | grep -E ' \| naming \| |\[W[123]\]' || true
  echo ""
fi

if [ "$CGC_EC" -ne 0 ]; then
  echo "❌ cgc 컴파일 실패 (exit $CGC_EC)" >&2
  rm -f "$PREPROCESSED"
  exit 1
fi

# ─── 3. gcc: C → ELF ─────────────────────────────────────────────
echo "⚙️  C → 바이너리 컴파일..."
if [ -n "$BUNDLE_SPEC" ]; then
  echo "   + --bundle=$BUNDLE_SPEC (기본 풀셋 아님 · docs/FX-BUNDLE-MAP.md)"
fi

# net
WANT_NET=0
if bundle_has net; then WANT_NET=1; fi
if [ "$NO_NET" = "1" ]; then WANT_NET=0; fi

if [ "$WANT_NET" = "1" ]; then
  NET_SRCS="$RUNTIME_DIR/http.c $RUNTIME_DIR/websocket.c $RUNTIME_DIR/http_client.c $RUNTIME_DIR/sse.c"
  NET_LIBS="-lssl -lcrypto -lcurl"
  EXTRA_CFLAGS=""
else
  echo "   + net stub (http/ws/client/crypto)"
  NET_SRCS="$RUNTIME_DIR/http-stub.c $RUNTIME_DIR/websocket-stub.c $RUNTIME_DIR/http_client-stub.c $RUNTIME_DIR/crypto-stub.c $RUNTIME_DIR/sse.c"
  NET_LIBS=""
  EXTRA_CFLAGS="-DFL_NO_CRYPTO"
fi

# db
DB_SRCS=""
DB_LIBS=""
if bundle_has db; then
  DB_SRCS="$RUNTIME_DIR/sqlite.c"
  DB_LIBS="-lsqlite3"
  if [ -f "$RUNTIME_DIR/mariadb.c" ]; then
    DB_SRCS="$DB_SRCS $RUNTIME_DIR/mariadb.c"
    echo "   + MariaDB dlopen 바인딩 포함"
  fi
else
  echo "   + db stub (sqlite/mariadb)"
  DB_SRCS="$RUNTIME_DIR/db-stub.c"
fi

# mail
MAIL_SRCS=""
if bundle_has mail; then
  MAIL_SRCS="$RUNTIME_DIR/smtp.c"
else
  echo "   + mail stub (smtp)"
  MAIL_SRCS="$RUNTIME_DIR/mail-stub.c"
fi

# core + 기타(regex/pdf/…)는 지도상 항상 — 번들에서 빼지 않음
RUNTIME_SRCS="$RUNTIME_DIR/core.c $RUNTIME_DIR/collection.c $RUNTIME_DIR/io.c \
  $RUNTIME_DIR/json.c $RUNTIME_DIR/math.c $RUNTIME_DIR/process.c \
  $RUNTIME_DIR/error.c $NET_SRCS $RUNTIME_DIR/aliases.c \
  $DB_SRCS $RUNTIME_DIR/debug.c $RUNTIME_DIR/gc.c \
  $RUNTIME_DIR/jit.c $RUNTIME_DIR/fx-builtin-shim.c \
  $RUNTIME_DIR/regex.c $MAIL_SRCS $RUNTIME_DIR/pdf_ttf.c $RUNTIME_DIR/pdf_img.c"

# user-fns.c — fl-source-manager 관리 (있으면 자동 포함)
if [ -f "$RUNTIME_DIR/user-fns.c" ]; then
  RUNTIME_SRCS="$RUNTIME_SRCS $RUNTIME_DIR/user-fns.c"
fi

GCC_LOG="/tmp/fl_gcc_$$.log"
if gcc -O2 -Werror=implicit-function-declaration -o "$OUTPUT" $C_FILE $RUNTIME_SRCS \
  -I "$RUNTIME_DIR" $EXTRA_CFLAGS \
  -rdynamic -lpthread -lm -ldl $DB_LIBS $NET_LIBS \
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
