#!/usr/bin/env bash
set -euo pipefail

FX_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SCRIPT_ROOT="${SCRIPT_ROOT:-/home/kim/freelang-script}"
CGC_BIN="${FX_CGC_BIN:-/home/kim/kim/platform/freelang-afj/bin/cgc-bin}"
TMP_P1B="$(mktemp -d "${TMPDIR:-/tmp}/freelang-fx-import-XXXXXX")"
trap 'rm -rf "$TMP_P1B"' EXIT

PACK="$TMP_P1B/fx-script-pack"
gcc -O0 -Wall -Wextra -o "$PACK" "$FX_ROOT/tools/fx-script-pack.c" -lcrypto

PROJECT="$TMP_P1B/project"
mkdir -p "$PROJECT/module" "$PROJECT/lib"
cp "$SCRIPT_ROOT/tests/fx-conformance/07-module.fls" "$PROJECT/main.fls"
cp "$SCRIPT_ROOT/tests/fx-conformance/module/math.fls" "$PROJECT/module/math.fls"
"$PACK" --root "$PROJECT" --lock "$PROJECT/freelang.lock.json" --update-lock "$PROJECT/main.fls" "$PROJECT/packed.fls"
"$PACK" --root "$PROJECT" --lock "$PROJECT/freelang.lock.json" --verify-lock "$PROJECT/main.fls" "$PROJECT/verified.fls"
cmp "$PROJECT/packed.fls" "$PROJECT/verified.fls"

R="$FX_ROOT/runtime"
RUNTIME_SOURCES=(
  "$R/core.c" "$R/collection.c" "$R/io.c" "$R/json.c" "$R/math.c" "$R/process.c" "$R/error.c" "$R/http.c" "$R/websocket.c" "$R/http_client.c" "$R/sse.c" "$R/aliases.c" "$R/sqlite.c" "$R/debug.c" "$R/gc.c" "$R/jit.c" "$R/fx-builtin-shim.c" "$R/regex.c" "$R/smtp.c" "$R/pdf_ttf.c" "$R/pdf_img.c" "$R/mariadb.c" "$R/user-fns.c"
)
"$CGC_BIN" "$PROJECT/packed.fls" "$PROJECT/main.c" >/dev/null
gcc -O0 -w -I "$R" -o "$PROJECT/main.elf" "$PROJECT/main.c" "${RUNTIME_SOURCES[@]}" -rdynamic -lpthread -lm -ldl -lsqlite3 -lssl -lcrypto -lcurl
test "$("$PROJECT/main.elf")" = "42"

cat >"$PROJECT/once-main.fls" <<'EOF'
(import "./lib/once.fls")
(import "./lib/once.fls")
(println (add 20 22))
EOF
cat >"$PROJECT/lib/once.fls" <<'EOF'
(println "loaded")
(defn add [left right] (+ left right))
EOF
"$PACK" --root "$PROJECT" --lock "$PROJECT/once.lock.json" --update-lock "$PROJECT/once-main.fls" "$PROJECT/once.packed.fls"
"$CGC_BIN" "$PROJECT/once.packed.fls" "$PROJECT/once.c" >/dev/null
gcc -O0 -w -I "$R" -o "$PROJECT/once.elf" "$PROJECT/once.c" "${RUNTIME_SOURCES[@]}" -rdynamic -lpthread -lm -ldl -lsqlite3 -lssl -lcrypto -lcurl
test "$("$PROJECT/once.elf")" = $'loaded\n42'

echo >>"$PROJECT/module/math.fls"
if "$PACK" --root "$PROJECT" --lock "$PROJECT/freelang.lock.json" --verify-lock "$PROJECT/main.fls" "$PROJECT/stale.fls" 2>"$PROJECT/stale.err"; then
  echo "expected lock mismatch" >&2; exit 1
fi
grep -Fq 'E_LOCKFILE_MISMATCH' "$PROJECT/stale.err"

if "$PACK" --root "$PROJECT" --lock "$PROJECT/missing.lock.json" --verify-lock "$PROJECT/main.fls" "$PROJECT/missing.packed.fls" 2>"$PROJECT/missing.err"; then
  echo "expected missing lock failure" >&2; exit 1
fi
grep -Fq 'E_LOCKFILE_MISMATCH' "$PROJECT/missing.err"

cat >"$PROJECT/cycle-a.fls" <<'EOF'
(import "./cycle-b.fls")
EOF
cat >"$PROJECT/cycle-b.fls" <<'EOF'
(import "./cycle-a.fls")
EOF
if "$PACK" --root "$PROJECT" --lock "$PROJECT/cycle.lock.json" --update-lock "$PROJECT/cycle-a.fls" "$PROJECT/cycle.packed.fls" 2>"$PROJECT/cycle.err"; then
  echo "expected cycle failure" >&2; exit 1
fi
grep -Fq 'E_CIRCULAR_IMPORT' "$PROJECT/cycle.err"

cat >"$PROJECT/escape.fls" <<'EOF'
(import "../outside.fls")
EOF
if "$PACK" --root "$PROJECT" --lock "$PROJECT/escape.lock.json" --update-lock "$PROJECT/escape.fls" "$PROJECT/escape.packed.fls" 2>"$PROJECT/escape.err"; then
  echo "expected escape failure" >&2; exit 1
fi
grep -Fq 'E_IMPORT_ESCAPE' "$PROJECT/escape.err"
printf 'PASS Script import P1-B\n'
