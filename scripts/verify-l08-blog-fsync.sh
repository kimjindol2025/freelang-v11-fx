#!/usr/bin/env bash
set -Eeuo pipefail
umask 077
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
blog="${L08_BLOG_SOURCE_ROOT:?approved source-only copy required}"
compiler="${L08_BLOG_NATIVE_COMPILER:?}"
out="${L08_BLOG_FSYNC_OUT:-$(mktemp -d)}"
[[ -f "$blog/app/store.fl" && ! -e "$blog/data/blog.wal" ]]
mkdir -p "$out/source/tests" "$out/root/tmp"
ln -s "$blog/app" "$out/source/app"
ln -s "$blog/src" "$out/source/src"
for phase in write read;do
 cp "$ROOT/tests/l08-platform/store-fsync-$phase.fl" "$out/source/tests/$phase.fl"
 extra=()
 [[ "$phase" != write ]] || extra=(--extern store-fsync-fail-next)
 (cd "$out/source/tests";"$compiler" "${extra[@]}" "$phase.fl" "$out/$phase.c") > "$out/$phase.build.log" 2>&1
 if [[ "$phase" == write ]];then
  gcc -O1 -g -w -D_GNU_SOURCE -I "$ROOT/runtime" -include "$ROOT/tests/l08-platform/store-fsync.h" -o "$out/$phase" "$out/$phase.c" "$ROOT/tests/l08-platform/store-fsync.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}" -Wl,--wrap=fsync >> "$out/$phase.build.log" 2>&1
 else
  gcc -O1 -g -w -I "$ROOT/runtime" -o "$out/$phase" "$out/$phase.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}" >> "$out/$phase.build.log" 2>&1
 fi
done
failed=0
for phase in write read;do
 if [[ "$phase" == read ]];then [[ -f "$out/root/blog.wal" ]];cp "$out/root/blog.wal" "$out/writer.wal";fi
 if (cd "$out/root";env -i PATH=/usr/bin:/bin LANG=C.UTF-8 TZ=UTC FL_FILE_BASE="$out/root" TMPDIR="$out/root/tmp" AFJ_DB_MODE=development AFJ_DB_BIND_HOST=127.0.0.1 AFJ_DB_WAL="$out/root/blog.wal" timeout 120 "$out/$phase") > "$out/$phase.actual" 2> "$out/$phase.stderr";then rc=0;else rc=$?;fi
 cat "$out/$phase.actual" "$out/$phase.stderr"
 printf 'DB_FSYNC_%s_EXIT=%s\n' "$phase" "$rc"
 if [[ "$rc" != 0 || -s "$out/$phase.stderr" ]];then failed=1;fi
 rg '^DB_FSYNC_' "$out/$phase.actual" > "$out/$phase.result" || true
 if ! diff -u "$ROOT/tests/l08-platform/store-fsync-$phase.expected" "$out/$phase.result";then failed=1;fi
done
if cmp -s "$out/root/blog.wal" "$out/writer.wal";then echo DB_FSYNC_WAL_READ_ONLY=PASS;else echo DB_FSYNC_WAL_READ_ONLY=FAIL;failed=1;fi
printf 'DB_FSYNC_SELF_EXIT=%s\n' "$failed"
exit "$failed"
