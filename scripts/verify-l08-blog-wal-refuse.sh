#!/usr/bin/env bash
set -Eeuo pipefail
umask 077
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
blog="${L08_BLOG_SOURCE_ROOT:?approved source-only copy required}"
compiler="${L08_BLOG_NATIVE_COMPILER:?}"
create="${L08_WAL_TAIL_CREATE_BIN:?native create fixture required}"
out="${L08_WAL_REFUSE_OUT:-$(mktemp -d)}"
[[ -f "$blog/app/store.fl" && ! -e "$blog/data/blog.wal" ]]
mkdir -p "$out/source/tests" "$out/outside"
ln -s "$blog/app" "$out/source/app"
ln -s "$blog/src" "$out/source/src"
cp "$ROOT/tests/l08-platform/wal-tail-refuse.fl" "$out/source/tests/main.fl"
(cd "$out/source/tests";"$compiler" --extern repair-test-fds main.fl "$out/main.c") > "$out/build.log" 2>&1
gcc -O1 -g -w -D_GNU_SOURCE -I "$ROOT/runtime" -include "$ROOT/tests/l08-platform/repair-io.h" -o "$out/main" "$out/main.c" "$ROOT/tests/l08-platform/repair-io.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}" >> "$out/build.log" 2>&1
printf outside > "$out/outside/sentinel"
passed=0;failed=0
for kind in wrong short long symlink;do
 root="$out/$kind";mkdir -p "$root/tmp"
 export FL_FILE_BASE="$root" TMPDIR="$root/tmp" AFJ_DB_MODE=development AFJ_DB_BIND_HOST=127.0.0.1 AFJ_DB_WAL="$root/blog.wal" L08_WAL_TAIL_MODE=prepare-partial
 (cd "$root";"$create") > "$root/create.log" 2>&1
 cp "$root/blog.wal" "$root/original.wal"
 hash=$(sha256sum "$root/blog.wal" | cut -d' ' -f1)
 backup="$root/blog.wal.recovery-$hash"
 case "$kind" in
  wrong) printf do-not-overwrite > "$backup";;
  short) cp "$root/blog.wal" "$backup";truncate -s -1 "$backup";;
  long) cp "$root/blog.wal" "$backup";printf x >> "$backup";;
  symlink) ln -s "$out/outside/sentinel" "$backup";;
 esac
 before=$(sha256sum "$backup" | cut -d' ' -f1)
 for phase in writer reader;do
  export L08_WAL_REFUSE_PHASE="$phase"
  if (cd "$root";timeout 120 "$out/main") > "$root/$phase.actual" 2> "$root/$phase.stderr";then rc=0;else rc=$?;fi
  printf 'WAL_REFUSE_CASE=%s PHASE=%s\n' "$kind" "$phase"
  cat "$root/$phase.actual" "$root/$phase.stderr"
  printf 'WAL_REFUSE_PROGRAM_EXIT=%s\n' "$rc"
  rg '^WAL_REFUSE_' "$root/$phase.actual" > "$root/$phase.result" || true
  expected="$ROOT/tests/l08-platform/wal-tail-refuse.expected"
  [[ "$phase" != reader ]] || expected="$ROOT/tests/l08-platform/wal-tail-refuse-reader.expected"
  if [[ "$rc" == 0 && ! -s "$root/$phase.stderr" ]] && diff -u "$expected" "$root/$phase.result";then passed=$((passed+1));else failed=$((failed+1));fi
  if cmp -s "$root/original.wal" "$root/blog.wal" && [[ "$before" == "$(sha256sum "$backup" | cut -d' ' -f1)" && $(cat "$out/outside/sentinel") == outside ]];then echo WAL_REFUSE_FILES_UNCHANGED=PASS;else echo WAL_REFUSE_FILES_UNCHANGED=FAIL;failed=$((failed+1));fi
 done
done
printf 'WAL_REFUSE_PASS=%s FAIL=%s\n' "$passed" "$failed"
[[ "$failed" == 0 ]]
