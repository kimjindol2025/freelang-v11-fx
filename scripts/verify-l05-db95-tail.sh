#!/usr/bin/env bash
set -Eeuo pipefail
umask 077
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
src="${L05_DB_SOURCE_ROOT:?approved database source directory required}"
compiler="${L05_DB_NATIVE_COMPILER:?native compiler required}"
entry="${L05_DB_ENTRY:-tcp-server.fls}"
case "$entry" in tcp-server.fls|native-store.fl) ;; *) exit 1;;esac
out="${L05_DB_TAIL_OUT:-$(mktemp -d)}"
[[ -f "$src/$entry" ]]
mkdir -p "$out/source/tests"
ln -s "$src" "$out/source/src"
sed "s@\"../src/tcp-server.fls\"@\"../src/$entry\"@" "$ROOT/tests/l08-platform/db95-tail.fl" > "$out/source/tests/main.fl"
flags=(-O1 -g -w -I "$ROOT/runtime")
if [[ -n "${L05_DB_SANITIZER_LIBDIR:-}" ]];then
 flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer -L "$L05_DB_SANITIZER_LIBDIR")
 export LD_LIBRARY_PATH="$L05_DB_SANITIZER_LIBDIR" ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1
fi
(cd "$out/source/tests";"$compiler" main.fl "$out/main.c") > "$out/build.log" 2>&1
gcc "${flags[@]}" -o "$out/main" "$out/main.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}" >> "$out/build.log" 2>&1
run(){
 local phase="$1" label="$2" rc
 if (cd "$root";env -i PATH=/usr/bin:/bin LANG=C.UTF-8 TZ=UTC LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}" ASAN_OPTIONS="${ASAN_OPTIONS:-}" UBSAN_OPTIONS="${UBSAN_OPTIONS:-}" FL_FILE_BASE="$root" TMPDIR="$root/tmp" AFJ_DB_MODE=development AFJ_DB_BIND_HOST=127.0.0.1 AFJ_DB_WAL="$root/db.wal" L05_TAIL_PHASE="$phase" timeout 120 "$out/main") > "$root/$label.actual" 2> "$root/$label.stderr";then rc=0;else rc=$?;fi
 cat "$root/$label.actual" "$root/$label.stderr"
 printf 'DB95_TAIL_CASE=%s PHASE=%s EXIT=%s\n' "$kind" "$label" "$rc"
 rg '^DB95_TAIL_' "$root/$label.actual" > "$root/$label.result" || true
 ok=0
 if [[ "$phase" == seed || "$phase" == interrupted ]];then
  if [[ "$rc" == 0 && ! -s "$root/$label.stderr" ]] && rg -q '^DB95_TAIL_(SEED|INTERRUPTED)=PASS$' "$root/$label.result";then ok=1;fi
 else
  if [[ "$rc" == 0 && ! -s "$root/$label.stderr" ]] && diff -u "$ROOT/tests/l08-platform/db95-tail.expected" "$root/$label.result";then ok=1;fi
 fi
}
passed=0;failed=0
for kind in prepare-partial commit-partial no-newline invalid-utf8 nul;do
 root="$out/$kind";mkdir -p "$root/tmp"
 run seed seed;[[ "$ok" == 1 ]]
 cp "$root/db.wal" "$root/seed.wal"
 size=$(wc -c < "$root/db.wal")
 case "$kind" in
  prepare-partial) run interrupted prepare;[[ "$ok" == 1 ]];total=$(wc -c < "$root/db.wal");truncate -s "$((size+(total-size)/2))" "$root/db.wal";;
  commit-partial) run interrupted prepare;[[ "$ok" == 1 ]];printf '{"type":"commit","tx":"l05-interrupted"' >> "$root/db.wal";;
  no-newline) truncate -s "$((size-1))" "$root/db.wal";;
  invalid-utf8) printf '\xff\xfe{"type":"prepare"' >> "$root/db.wal";;
  nul) printf '\0{"type":"prepare"' >> "$root/db.wal";;
 esac
 cp "$root/db.wal" "$root/original-tail.wal"
 run resume resume
 if [[ "$ok" == 1 ]];then passed=$((passed+1));else failed=$((failed+1));fi
 cp "$root/db.wal" "$root/resumed.wal"
 for attempt in {1..10};do
  run read "read-$attempt"
  if [[ "$ok" == 1 ]] && cmp -s "$root/resumed.wal" "$root/db.wal";then passed=$((passed+1));else failed=$((failed+1));fi
 done
done
printf 'DB95_TAIL_GROUP_PASS=%s FAIL=%s\n' "$passed" "$failed"
[[ "$failed" == 0 ]]
