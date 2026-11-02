#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_COMPILER_PARSE_OUT:-$(mktemp -d)}"
compiler="${L08_COMPILER_PARSE_COMPILER:?}"
mkdir -p "$out/source"
[[ $(tail -n 1 "$ROOT/self/cgc-main.fl") == '(cgc-run (cli-args))' ]]
head -n -1 "$ROOT/self/cgc-main.fl" > "$out/source/compiler-library.fl"
cp "$ROOT/tests/l08-platform/compiler-parse-memory.fl" "$out/source/main.fl"
(cd "$out/source";"$compiler" --extern compiler-lex-meter-start --extern compiler-lex-meter-live --extern compiler-lex-meter-peak main.fl "$out/main.c") > "$out/build.log" 2>&1
gcc -O1 -g -w -no-pie -D_GNU_SOURCE -I "$ROOT/runtime" -include "$ROOT/tests/l08-platform/compiler-lex-memory.h" -o "$out/main" "$out/main.c" "$ROOT/tests/l08-platform/compiler-parse-memory.c" "${L07_SRCS[@]}" "$ROOT/runtime/cgc-bridge.c" "${L07_LIBS[@]}" -Wl,--wrap=malloc,--wrap=calloc,--wrap=realloc,--wrap=free >> "$out/build.log" 2>&1
if timeout 120 "$out/main" > "$out/actual" 2> "$out/stderr";then rc=0;else rc=$?;fi
cat "$out/actual" "$out/stderr"
printf 'COMPILER_PARSE_PROGRAM_EXIT=%s\n' "$rc"
[[ "$rc" == 0 && ! -s "$out/stderr" ]]
rg '^COMPILER_PARSE_.*=PASS$' "$out/actual" > "$out/result"
diff -u "$ROOT/tests/l08-platform/compiler-parse-memory.expected" "$out/result"
echo COMPILER_PARSE_SELF_EXIT=0
