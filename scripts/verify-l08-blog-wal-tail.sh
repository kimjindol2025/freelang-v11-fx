#!/usr/bin/env bash
set -Eeuo pipefail
umask 077
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
blog="${L08_BLOG_SOURCE_ROOT:?approved source-only copy required}"
compiler="${L08_BLOG_NATIVE_COMPILER:?}"
out="${L08_BLOG_WAL_TAIL_OUT:-$(mktemp -d)}"
[[ -f "$blog/app/store.fl" && ! -e "$blog/data/blog.wal" ]]
mkdir -p "$out/source/tests"
ln -s "$blog/app" "$out/source/app"
ln -s "$blog/src" "$out/source/src"
for phase in create resume verify;do
 if [[ -n "${L08_BLOG_WAL_TAIL_BIN_DIR:-}" ]];then
  [[ -x "$L08_BLOG_WAL_TAIL_BIN_DIR/$phase" ]]
  ln -s "$L08_BLOG_WAL_TAIL_BIN_DIR/$phase" "$out/$phase"
  continue
 fi
 cp "$ROOT/tests/l08-platform/wal-tail-$phase.fl" "$out/source/tests/$phase.fl"
 extra=();support=()
 if [[ "$phase" == create ]];then
  extra=(--extern store-fault-arm --extern store-fault-count --extern store-fault-open-count)
  support=(-include "$ROOT/tests/l08-platform/store-wal-fault.h" "$ROOT/tests/l08-platform/store-wal-fault.c" -Wl,--wrap=write,--wrap=close,--wrap=fsync)
 fi
 (cd "$out/source/tests";"$compiler" "${extra[@]}" "$phase.fl" "$out/$phase.c") > "$out/$phase.build.log" 2>&1
 gcc -O1 -g -w -D_GNU_SOURCE -I "$ROOT/runtime" -o "$out/$phase" "$out/$phase.c" "${support[@]}" "${L07_SRCS[@]}" "${L07_LIBS[@]}" >> "$out/$phase.build.log" 2>&1
done
passed=0;failed=0
run(){
 local phase="$1" label="$2" rc fixture_mode="$mode"
 if [[ "$phase" == create && ( "$mode" == invalid-utf8 || "$mode" == nul-tail ) ]];then fixture_mode=complete-no-newline;fi
 if (cd "$root";env -i PATH=/usr/bin:/bin LANG=C.UTF-8 TZ=UTC FL_FILE_BASE="$root" TMPDIR="$root/tmp" AFJ_DB_MODE=development AFJ_DB_BIND_HOST=127.0.0.1 AFJ_DB_WAL="$root/blog.wal" L08_WAL_TAIL_MODE="$fixture_mode" timeout 120 "$out/$phase") > "$root/$label.actual" 2> "$root/$label.stderr";then rc=0;else rc=$?;fi
 printf 'WAL_TAIL_CASE=%s PHASE=%s\n' "$mode" "$label"
 cat "$root/$label.actual" "$root/$label.stderr"
 printf 'WAL_TAIL_PROGRAM_EXIT=%s\n' "$rc"
 rg '^WAL_TAIL_' "$root/$label.actual" > "$root/$label.result" || true
 if [[ "$rc" == 0 && ! -s "$root/$label.stderr" ]] && diff -u "$ROOT/tests/l08-platform/wal-tail-$phase.expected" "$root/$label.result";then passed=$((passed+1));else failed=$((failed+1));fi
}
for backup_state in absent matching;do
for mode in prepare-partial commit-partial complete-no-newline invalid-utf8 nul-tail;do
 root="$out/$backup_state/$mode";mkdir -p "$root/tmp"
 run create create
 if [[ "$mode" == complete-no-newline ]];then
  [[ $(tail -c 1 "$root/blog.wal" | od -An -tu1 | tr -d ' ') == 10 ]]
  truncate -s -1 "$root/blog.wal"
 fi
 if [[ "$mode" == invalid-utf8 ]];then printf '\xff\xc3\xf0\x9f' >> "$root/blog.wal";fi
 if [[ "$mode" == nul-tail ]];then printf '{"bad":"\0\xff' >> "$root/blog.wal";fi
 cp "$root/blog.wal" "$root/before-resume.wal"
 original_hash=$(sha256sum "$root/before-resume.wal" | cut -d' ' -f1)
 if [[ "$backup_state" == matching ]];then
  cp "$root/before-resume.wal" "$root/blog.wal.recovery-$original_hash"
 fi
 run resume resume
 backup="$root/blog.wal.recovery-$original_hash"
 if [[ -f "$backup" ]] && cmp -s "$backup" "$root/before-resume.wal";then echo WAL_TAIL_ORIGINAL_BACKUP=PASS;else echo WAL_TAIL_ORIGINAL_BACKUP=FAIL;failed=$((failed+1));fi
 prefix_bytes=$(wc -c < "$root/committed-prefix.wal")
 if cmp -n "$prefix_bytes" -s "$root/committed-prefix.wal" "$root/blog.wal";then echo WAL_TAIL_COMMITTED_PREFIX=PASS;else echo WAL_TAIL_COMMITTED_PREFIX=FAIL;failed=$((failed+1));fi
 cp "$root/blog.wal" "$root/after-resume.wal"
 for iteration in {1..10};do
  run verify "verify-$iteration"
  if ! cmp -s "$root/blog.wal" "$root/after-resume.wal";then echo WAL_TAIL_READER_UNCHANGED=FAIL;failed=$((failed+1));fi
 done
 if [[ -f "$backup" ]] && cmp -s "$backup" "$root/before-resume.wal";then echo WAL_TAIL_BACKUP_UNCHANGED=PASS;else echo WAL_TAIL_BACKUP_UNCHANGED=FAIL;failed=$((failed+1));fi
done
done
printf 'WAL_TAIL_PASS=%s FAIL=%s\n' "$passed" "$failed"
[[ "$failed" == 0 ]]
