#!/usr/bin/env bash
set -Eeuo pipefail
umask 077
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
blog="${L08_BLOG_SOURCE_ROOT:?approved source-only copy required}"
compiler="${L08_BLOG_NATIVE_COMPILER:?}"
out="${L08_BACKUP_INTEGRITY_OUT:-$(mktemp -d)}"
[[ -f "$blog/app/blog-backup.fl" && ! -e "$blog/data/blog.wal" ]]
mkdir -p "$out/source/tests" "$out/original/tmp"
ln -s "$blog/app" "$out/source/app"
ln -s "$blog/src" "$out/source/src"
flags=(-O1 -g -w -I "$ROOT/runtime")
if [[ -n "${L08_BACKUP_INTEGRITY_SANITIZER_LIBDIR:-}" ]];then
 flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer -L "$L08_BACKUP_INTEGRITY_SANITIZER_LIBDIR")
 export LD_LIBRARY_PATH="$L08_BACKUP_INTEGRITY_SANITIZER_LIBDIR" ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1
fi
for phase in seed check;do
 if [[ -n "${L08_BACKUP_INTEGRITY_BIN_DIR:-}" ]];then
  [[ -x "$L08_BACKUP_INTEGRITY_BIN_DIR/$phase" ]]
  ln -s "$L08_BACKUP_INTEGRITY_BIN_DIR/$phase" "$out/$phase"
  continue
 fi
 cp "$ROOT/tests/l08-platform/backup-integrity-$phase.fl" "$out/source/tests/$phase.fl"
 (cd "$out/source/tests";"$compiler" "$phase.fl" "$out/$phase.c") > "$out/$phase.build.log" 2>&1
 gcc "${flags[@]}" -o "$out/$phase" "$out/$phase.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}" >> "$out/$phase.build.log" 2>&1
done
printf 'GIF89a\0\xffRAW-archive' > "$out/original/fixture"
original_hash=$(sha256sum "$out/original/fixture" | cut -d' ' -f1)
passed=0;failed=0
run(){
 local executable="$1" label="$2" expected="$3" rc
 if (cd "$root";env -i PATH=/usr/bin:/bin LANG=C.UTF-8 TZ=UTC LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}" ASAN_OPTIONS="${ASAN_OPTIONS:-}" UBSAN_OPTIONS="${UBSAN_OPTIONS:-}" FL_FILE_BASE="$root" TMPDIR="$root/tmp" AFJ_DB_MODE=development AFJ_DB_BIND_HOST=127.0.0.1 AFJ_DB_WAL="$root/blog.wal" L08_BACKUP_CASE="$label" L08_ORIGINAL_IMAGE_HASH="$original_hash" timeout 120 "$out/$executable") > "$root/actual" 2> "$root/stderr";then rc=0;else rc=$?;fi
 cat "$root/actual" "$root/stderr"
 printf 'BACKUP_INTEGRITY_%s_EXIT=%s\n' "$label" "$rc"
 rg '^BACKUP_INTEGRITY_' "$root/actual" > "$root/result" || true
 ok=0
 if [[ "$rc" == 0 && ! -s "$root/stderr" ]] && diff -u "$expected" "$root/result";then ok=1;fi
}
root="$out/original"
run seed seed "$ROOT/tests/l08-platform/backup-integrity-seed.expected"
if [[ "$ok" == 1 ]];then passed=$((passed+1));else failed=$((failed+1));fi
backup="$root/blog-backups/integrity-proof"
image=66666666666666666666666666666666.gif
manifest_hash=$(sha256sum "$backup/manifest.json" | cut -d' ' -f1)
printf untouched > "$out/outside.gif"
for kind in good legacy-v2 bad hash-null hash-empty hash-key hash-hex hash-mismatch truncated bad-id foreign-blog extra-file;do
 root="$out/$kind";mkdir -p "$root/tmp" "$root/blog-backups"
 cp -a "$backup" "$root/blog-backups/"
 manifest="$root/blog-backups/integrity-proof/manifest.json"
 copied="$root/blog-backups/integrity-proof/images/$image"
 expression=""
 case "$kind" in
  legacy-v2) expression='del(.image_hashes)';;
  hash-null) expression='.image_hashes=null';;
  hash-empty) expression='.image_hashes={}';;
  hash-key) expression='.image_hashes={"77777777777777777777777777777777":.image_hashes["66666666666666666666666666666666"]}';;
  hash-hex) expression='.image_hashes["66666666666666666666666666666666"]="not-a-hash"';;
  hash-mismatch) expression='.image_hashes["66666666666666666666666666666666"]="0000000000000000000000000000000000000000000000000000000000000000"';;
  bad-id) expression='.images[0].id="../../outside" | .images[0].path="/images/../../outside.gif"';;
  foreign-blog) expression='.posts[0].blog_id=2';;
  truncated) truncate -s -1 "$copied";;
  extra-file) cp "$copied" "$root/blog-backups/integrity-proof/images/extra.gif";;
 esac
 if [[ -n "$expression" ]];then jq "$expression" "$manifest" > "$root/changed-manifest.json";mv "$root/changed-manifest.json" "$manifest";fi
 if [[ "$kind" == bad ]];then
  original_size=$(wc -c < "$copied")
  printf X | dd of="$copied" bs=1 seek=7 conv=notrunc status=none
  [[ $(wc -c < "$copied") == "$original_size" && $(sha256sum "$copied" | cut -d' ' -f1) != "$original_hash" ]]
 fi
 printf 'BACKUP_INTEGRITY_CASE_KIND=%s\n' "$kind"
 label=bad;[[ "$kind" != good && "$kind" != legacy-v2 ]] || label=good
 run check "$label" "$ROOT/tests/l08-platform/backup-integrity-$label.expected"
 if [[ "$label" == bad ]] && ! cmp -s "$root/before-import.wal" "$root/blog.wal";then echo BACKUP_INTEGRITY_RAW_WAL_UNCHANGED=FAIL;ok=0;fi
 if [[ "$ok" == 1 ]];then passed=$((passed+1));else failed=$((failed+1));fi
done
[[ $(cat "$out/outside.gif") == untouched ]]
echo BACKUP_INTEGRITY_OUTSIDE_UNCHANGED=PASS
if [[ "$manifest_hash" == "$(sha256sum "$backup/manifest.json" | cut -d' ' -f1)" && "$original_hash" == "$(sha256sum "$backup/images/$image" | cut -d' ' -f1)" ]];then echo BACKUP_INTEGRITY_SOURCE_BACKUP_UNCHANGED=PASS;else echo BACKUP_INTEGRITY_SOURCE_BACKUP_UNCHANGED=FAIL;failed=$((failed+1));fi
printf 'BACKUP_INTEGRITY_PASS=%s FAIL=%s\n' "$passed" "$failed"
[[ "$failed" == 0 ]]
