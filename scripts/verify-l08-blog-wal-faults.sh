#!/usr/bin/env bash
set -Eeuo pipefail
umask 077
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
blog="${L08_BLOG_SOURCE_ROOT:?approved source-only copy required}"
compiler="${L08_BLOG_NATIVE_COMPILER:?}"
out="${L08_BLOG_WAL_FAULT_OUT:-$(mktemp -d)}"
[[ -f "$blog/app/store.fl" && ! -e "$blog/data/blog.wal" ]]
mkdir -p "$out/source/tests"
ln -s "$blog/app" "$out/source/app"
ln -s "$blog/src" "$out/source/src"
flags=(-O1 -g -w -D_GNU_SOURCE -I "$ROOT/runtime")
if [[ -n "${L08_BLOG_WAL_FAULT_SANITIZER_LIBDIR:-}" ]];then
 flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer -L "$L08_BLOG_WAL_FAULT_SANITIZER_LIBDIR")
 export LD_LIBRARY_PATH="$L08_BLOG_WAL_FAULT_SANITIZER_LIBDIR" ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1
fi
for phase in write read;do
 cp "$ROOT/tests/l08-platform/store-wal-fault-$phase.fl" "$out/source/tests/$phase.fl"
 extra=();support=()
 if [[ "$phase" == write ]];then
  extra=(--extern store-fault-arm --extern store-fault-count --extern store-fault-open-count)
  support=(-include "$ROOT/tests/l08-platform/store-wal-fault.h" "$ROOT/tests/l08-platform/store-wal-fault.c" -Wl,--wrap=write,--wrap=close,--wrap=fsync)
 fi
 (cd "$out/source/tests";"$compiler" "${extra[@]}" "$phase.fl" "$out/$phase.c") > "$out/$phase.build.log" 2>&1
 gcc "${flags[@]}" -o "$out/$phase" "$out/$phase.c" "${support[@]}" "${L07_SRCS[@]}" "${L07_LIBS[@]}" >> "$out/$phase.build.log" 2>&1
done
passed=0;failed=0
for mode in prepare-write prepare-partial prepare-close commit-sync commit-close;do
 root="$out/$mode";mkdir -p "$root/tmp"
 for phase in write read;do
  expected="$ROOT/tests/l08-platform/store-wal-fault-write.expected"
  if [[ "$phase" == read ]];then
   cp "$root/blog.wal" "$root/before-reader.wal"
   category=aborted;[[ "$mode" != commit-* ]] || category=committed
   expected="$ROOT/tests/l08-platform/store-wal-fault-read-$category.expected"
  fi
  if (cd "$root";env -i PATH=/usr/bin:/bin LANG=C.UTF-8 TZ=UTC LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}" ASAN_OPTIONS="${ASAN_OPTIONS:-}" UBSAN_OPTIONS="${UBSAN_OPTIONS:-}" FL_FILE_BASE="$root" TMPDIR="$root/tmp" AFJ_DB_MODE=development AFJ_DB_BIND_HOST=127.0.0.1 AFJ_DB_WAL="$root/blog.wal" L08_WAL_FAULT_MODE="$mode" timeout 120 "$out/$phase") > "$root/$phase.actual" 2> "$root/$phase.stderr";then rc=0;else rc=$?;fi
  printf 'WAL_FAULT_CASE=%s PHASE=%s\n' "$mode" "$phase"
  cat "$root/$phase.actual" "$root/$phase.stderr"
  printf 'WAL_FAULT_PROGRAM_EXIT=%s\n' "$rc"
  rg '^WAL_FAULT_' "$root/$phase.actual" > "$root/$phase.result" || true
  if [[ "$rc" == 0 && ! -s "$root/$phase.stderr" ]] && diff -u "$expected" "$root/$phase.result";then passed=$((passed+1));else failed=$((failed+1));fi
 done
 if cmp -s "$root/blog.wal" "$root/before-reader.wal";then echo WAL_FAULT_READER_UNCHANGED=PASS;else echo WAL_FAULT_READER_UNCHANGED=FAIL;failed=$((failed+1));fi
done
printf 'WAL_FAULT_PASS=%s FAIL=%s\n' "$passed" "$failed"
[[ "$failed" == 0 ]]
