#!/bin/bash
# fx-sqlite-browser 빌드
#   cgc = 설치본(param/map 정상). sqlite_*·server_req_body 는
#   runtime/fx-builtin-shim.c 가 fl_fn_call 호환 클로저로 노출.
set -e
FX=/root/kim/freelang-v11-fx
CGC="${CGC_BIN:-/root/freelang-v11/bin/cgc-bin}"
R="$FX/runtime"
IN="${1:-server.fl}"; OUT="${2:-fx-sqlite-browser}"
C=/tmp/fxsb_$$.c
"$CGC" "$IN" "$C" 2>&1 | grep -viE '^\[FL|^Compiled|^$' || true
gcc -O2 -o "$OUT" "$C" \
  $R/core.c $R/collection.c $R/io.c $R/json.c $R/math.c $R/process.c \
  $R/error.c $R/http.c $R/aliases.c $R/sqlite.c $R/debug.c $R/gc.c $R/jit.c \
  $R/mariadb.c $R/fx-builtin-shim.c \
  -I "$R" -rdynamic -lpthread -lm -ldl -lsqlite3 -w 2>&1 | grep -E 'error:' | grep -v '"error"' | head -12 || true
if [ -x "$OUT" ]; then echo "✅ 빌드 OK: $(du -h "$OUT"|cut -f1)"; else echo "❌ 빌드 실패"; fi
rm -f "$C"
