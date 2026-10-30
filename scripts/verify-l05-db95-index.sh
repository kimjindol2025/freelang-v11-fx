#!/usr/bin/env bash
set -Eeuo pipefail
umask 077
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
src="${L05_DB_SOURCE_ROOT:?approved database source directory required}"
compiler="${L05_DB_NATIVE_COMPILER:?native compiler required}"
out="${L05_DB_INDEX_OUT:-$(mktemp -d)}"
[[ -f "$src/tcp-server.fls" ]]
mkdir -p "$out/source/tests" "$out/root/tmp"
ln -s "$src" "$out/source/src"
cp "$ROOT/tests/l08-platform/db95-index.fl" "$out/source/tests/main.fl"
flags=(-O1 -g -w -I "$ROOT/runtime")
if [[ -n "${L05_DB_SANITIZER_LIBDIR:-}" ]];then
 flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer -L "$L05_DB_SANITIZER_LIBDIR")
 export LD_LIBRARY_PATH="$L05_DB_SANITIZER_LIBDIR" ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1
fi
(cd "$out/source/tests";"$compiler" main.fl "$out/main.c") > "$out/build.log" 2>&1
gcc "${flags[@]}" -o "$out/main" "$out/main.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}" >> "$out/build.log" 2>&1
passed=0;failed=0
for attempt in {0..10};do
 phase=read;[[ "$attempt" != 0 ]] || phase=write
 if (cd "$out/root";env -i PATH=/usr/bin:/bin LANG=C.UTF-8 TZ=UTC LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}" ASAN_OPTIONS="${ASAN_OPTIONS:-}" UBSAN_OPTIONS="${UBSAN_OPTIONS:-}" FL_FILE_BASE="$out/root" TMPDIR="$out/root/tmp" AFJ_DB_MODE=development AFJ_DB_BIND_HOST=127.0.0.1 AFJ_DB_WAL="$out/root/db.wal" L05_INDEX_PHASE="$phase" timeout 120 "$out/main") > "$out/$attempt.actual" 2> "$out/$attempt.stderr";then rc=0;else rc=$?;fi
 cat "$out/$attempt.actual" "$out/$attempt.stderr"
 printf 'DB95_INDEX_ATTEMPT=%s PHASE=%s EXIT=%s\n' "$attempt" "$phase" "$rc"
 rg '^DB95_INDEX_' "$out/$attempt.actual" > "$out/$attempt.result" || true
 if [[ "$rc" == 0 && ! -s "$out/$attempt.stderr" ]] && diff -u "$ROOT/tests/l08-platform/db95-index-$phase.expected" "$out/$attempt.result";then passed=$((passed+1));else failed=$((failed+1));fi
 if [[ "$attempt" == 0 ]];then cp "$out/root/db.wal" "$out/writer.wal";else cmp "$out/writer.wal" "$out/root/db.wal";fi
done
printf 'DB95_INDEX_GROUP_PASS=%s FAIL=%s\n' "$passed" "$failed"
[[ "$failed" == 0 ]]
