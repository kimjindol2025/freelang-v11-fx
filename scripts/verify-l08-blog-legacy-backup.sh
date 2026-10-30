#!/usr/bin/env bash
# Run existing v1 backup fixtures natively; retain all synthetic originals.
set -Eeuo pipefail
umask 077
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
blog="${L08_BLOG_SOURCE_ROOT:?approved source-only copy required}"
compiler="${L08_BLOG_NATIVE_COMPILER:?}"
out="${L08_LEGACY_BACKUP_OUT:-$(mktemp -d)}"
[[ -f "$blog/app/blog-backup.fl" && ! -e "$blog/data/blog.wal" ]]
mkdir -p "$out/source/tmp" "$out/target/tmp"
for phase in export import;do
 (cd "$blog/tests";"$compiler" "b4-legacy-$phase.fl" "$out/$phase.c") > "$out/$phase.build.log" 2>&1
 gcc -O1 -g -w -I "$ROOT/runtime" -o "$out/$phase" "$out/$phase.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}" >> "$out/$phase.build.log" 2>&1
done
run(){
 local binary="$1" root="$2" label="$3" rc
 if (cd "$root";env -i PATH=/usr/bin:/bin LANG=C.UTF-8 TZ=UTC FL_FILE_BASE="$root" TMPDIR="$root/tmp" AFJ_DB_MODE=development AFJ_DB_BIND_HOST=127.0.0.1 AFJ_DB_WAL="$root/blog.wal" timeout 120 "$out/$binary") > "$out/$label.actual" 2> "$out/$label.stderr";then rc=0;else rc=$?;fi
 cat "$out/$label.actual" "$out/$label.stderr"
 printf 'NATIVE_LEGACY_%s_EXIT=%s\n' "$label" "$rc"
 run_rc="$rc"
}
run export "$out/source" export
[[ "$run_rc" == 0 && ! -s "$out/export.stderr" ]]
[[ $(tail -n 1 "$out/export.actual") == 'B4_LEGACY_FIXTURE version=1 posts=1 images=1' ]]
source_wal_hash=$(sha256sum "$out/source/blog.wal" | cut -d' ' -f1)
source_backup="$out/source/blog-backups/b4-legacy"
manifest_hash=$(sha256sum "$source_backup/manifest.json" | cut -d' ' -f1)
image=55555555555555555555555555555555.gif
image_hash=$(sha256sum "$source_backup/images/$image" | cut -d' ' -f1)
mkdir -p "$out/target/blog-backups"
cp -a "$source_backup" "$out/target/blog-backups/"
run import "$out/target" import
[[ "$run_rc" == 0 && ! -s "$out/import.stderr" ]]
[[ $(tail -n 1 "$out/import.actual") == 'B4_LEGACY_RESTORED version=1 posts=1 images=1' ]]
target_wal_hash=$(sha256sum "$out/target/blog.wal" | cut -d' ' -f1)
restored_hash=$(sha256sum "$out/target/blog-images/$image" | cut -d' ' -f1)
[[ "$restored_hash" == "$image_hash" ]]
printf 'NATIVE_LEGACY_IMAGE_SHA256=%s\n' "$restored_hash"
run import "$out/target" nonempty
[[ "$run_rc" == 1 ]]
rg -q 'BLOG_BACKUP_TARGET_NOT_EMPTY_OR_MISSING' "$out/nonempty.stderr"
[[ "$target_wal_hash" == "$(sha256sum "$out/target/blog.wal" | cut -d' ' -f1)" ]]
[[ "$restored_hash" == "$(sha256sum "$out/target/blog-images/$image" | cut -d' ' -f1)" ]]
[[ "$source_wal_hash" == "$(sha256sum "$out/source/blog.wal" | cut -d' ' -f1)" ]]
[[ "$manifest_hash" == "$(sha256sum "$source_backup/manifest.json" | cut -d' ' -f1)" ]]
[[ "$image_hash" == "$(sha256sum "$source_backup/images/$image" | cut -d' ' -f1)" ]]
echo NATIVE_LEGACY_NONEMPTY_REFUSAL_UNCHANGED=PASS
echo NATIVE_LEGACY_ORIGINALS_PRESERVED=PASS
echo 'NATIVE_LEGACY_BACKUP_PASS=3 FAIL=0'
