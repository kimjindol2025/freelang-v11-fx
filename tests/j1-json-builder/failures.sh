#!/usr/bin/env bash
# Caller records ps and holds the heavy-test lock; run sanitizer outside strace.
set -Eeuo pipefail
ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)
source "$ROOT/tests/l07/build.sh"
out=${J1_FAILURE_OUT:?};libdir=${J1_SANITIZER_LIBDIR:?};mkdir -p "$out"
gcc -O1 -g -w -fsanitize=address,undefined -fno-omit-frame-pointer -L "$libdir" -I "$ROOT/runtime" -o "$out/failures" "$ROOT/tests/j1-json-builder/failures.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}"
export LD_LIBRARY_PATH="$libdir${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 LSAN_OPTIONS=exitcode=23
timeout 60 "$out/failures" > "$out/stdout" 2> "$out/stderr"
cat "$out/stdout" "$out/stderr"
diff -u "$ROOT/tests/j1-json-builder/failures.expected" "$out/stdout"
[[ ! -s "$out/stderr" ]]
echo JSON_FAILURE_ASAN_UBSAN_LSAN=PASS
