#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_CONCAT_OUT:-$(mktemp -d)}"
compiler="${L08_CONCAT_COMPILER:?}"
mkdir -p "$out"
"$compiler" --extern compiler-lex-meter-start --extern compiler-lex-meter-live --extern compiler-lex-meter-peak "$ROOT/tests/l08-platform/concat-memory.fl" "$out/main.c" > "$out/build.log" 2>&1
gcc -O1 -g -w -D_GNU_SOURCE -I "$ROOT/runtime" -include "$ROOT/tests/l08-platform/compiler-lex-memory.h" -o "$out/main" "$out/main.c" "$ROOT/tests/l08-platform/compiler-lex-memory.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}" -Wl,--wrap=malloc,--wrap=calloc,--wrap=realloc,--wrap=free >> "$out/build.log" 2>&1
if timeout 60 "$out/main" > "$out/actual" 2> "$out/stderr";then rc=0;else rc=$?;fi
cat "$out/actual" "$out/stderr"
printf 'CONCAT_MEMORY_PROGRAM_EXIT=%s\n' "$rc"
[[ "$rc" == 0 && ! -s "$out/stderr" ]]
rg '^CONCAT_.*=PASS$' "$out/actual" > "$out/result"
diff -u "$ROOT/tests/l08-platform/concat-memory.expected" "$out/result"
echo CONCAT_MEMORY_SELF_EXIT=0
