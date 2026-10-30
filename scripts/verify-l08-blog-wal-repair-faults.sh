#!/usr/bin/env bash
set -Eeuo pipefail
umask 077
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
blog="${L08_BLOG_SOURCE_ROOT:?approved source-only copy required}"
compiler="${L08_BLOG_NATIVE_COMPILER:?}"
bins="${L08_WAL_TAIL_BIN_DIR:?verified create/resume/verify binaries required}"
refuse="${L08_WAL_REFUSE_BIN:?verified backup refusal executable required}"
out="${L08_WAL_REPAIR_FAULT_OUT:-$(mktemp -d)}"
[[ -f "$blog/app/store.fl" && ! -e "$blog/data/blog.wal" ]]
for phase in create resume verify;do [[ -x "$bins/$phase" ]];done
[[ -x "$refuse" ]]
mkdir -p "$out/source/tests"
ln -s "$blog/app" "$out/source/app"
ln -s "$blog/src" "$out/source/src"
cp "$ROOT/tests/l08-platform/wal-repair-fault.fl" "$out/source/tests/main.fl"
(cd "$out/source/tests";"$compiler" --extern repair-test-fds --extern repair-fault-arm --extern repair-fault-count main.fl "$out/main.c") > "$out/build.log" 2>&1
flags=(-O1 -g -w -D_GNU_SOURCE -I "$ROOT/runtime" -include "$ROOT/tests/l08-platform/repair-io.h" -include "$ROOT/tests/l08-platform/wal-repair-fault.h")
if [[ -n "${L08_WAL_REPAIR_SANITIZER_LIBDIR:-}" ]];then
 flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer -L "$L08_WAL_REPAIR_SANITIZER_LIBDIR")
 export LD_LIBRARY_PATH="$L08_WAL_REPAIR_SANITIZER_LIBDIR" ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1
fi
gcc "${flags[@]}" -o "$out/main" "$out/main.c" "$ROOT/tests/l08-platform/repair-io.c" "$ROOT/tests/l08-platform/wal-repair-fault.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}" -Wl,--wrap=write,--wrap=fsync,--wrap=close,--wrap=ftruncate >> "$out/build.log" 2>&1
passed=0;failed=0
run(){
 local executable="$1" phase="$2" pattern="$3" expected="$4" rc
 if (cd "$root";env -i PATH=/usr/bin:/bin LANG=C.UTF-8 TZ=UTC LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}" ASAN_OPTIONS="${ASAN_OPTIONS:-}" UBSAN_OPTIONS="${UBSAN_OPTIONS:-}" FL_FILE_BASE="$root" TMPDIR="$root/tmp" AFJ_DB_MODE=development AFJ_DB_BIND_HOST=127.0.0.1 AFJ_DB_WAL="$root/blog.wal" L08_REPAIR_BACKUP="$backup" L08_REPAIR_FAULT="$kind" L08_WAL_REFUSE_PHASE="$phase" timeout 120 "$executable") > "$root/$phase.actual" 2> "$root/$phase.stderr";then rc=0;else rc=$?;fi
 printf 'WAL_REPAIR_FAULT_CASE=%s PHASE=%s\n' "$kind" "$phase"
 cat "$root/$phase.actual" "$root/$phase.stderr"
 printf 'WAL_REPAIR_FAULT_PROGRAM_EXIT=%s\n' "$rc"
 rg "$pattern" "$root/$phase.actual" > "$root/$phase.result" || true
 if [[ "$rc" == 0 && ! -s "$root/$phase.stderr" ]] && diff -u "$expected" "$root/$phase.result";then passed=$((passed+1));else failed=$((failed+1));fi
}
for kind in backup-write backup-fsync parent-fsync backup-close truncate wal-fsync wal-close;do
 root="$out/$kind";mkdir -p "$root/tmp"
 (cd "$root";env FL_FILE_BASE="$root" TMPDIR="$root/tmp" AFJ_DB_MODE=development AFJ_DB_BIND_HOST=127.0.0.1 AFJ_DB_WAL="$root/blog.wal" L08_WAL_TAIL_MODE=prepare-partial "$bins/create") > "$root/create.log" 2>&1
 cp "$root/blog.wal" "$root/original.wal"
 original_hash=$(sha256sum "$root/original.wal" | cut -d' ' -f1)
 backup="$root/blog.wal.recovery-$original_hash"
 run "$out/main" fault '^WAL_REPAIR_FAULT_' "$ROOT/tests/l08-platform/wal-repair-fault.expected"
 wanted="$root/original.wal"
 [[ "$kind" != wal-fsync && "$kind" != wal-close ]] || wanted="$root/committed-prefix.wal"
 if cmp -s "$wanted" "$root/blog.wal";then echo WAL_REPAIR_FAULT_WAL_BOUNDARY=PASS;else echo WAL_REPAIR_FAULT_WAL_BOUNDARY=FAIL;failed=$((failed+1));fi
 if [[ "$kind" == backup-write ]];then
  bytes=$(wc -c < "$backup")
  if [[ "$bytes" -gt 0 && "$bytes" -lt $(wc -c < "$root/original.wal") ]] && cmp -n "$bytes" -s "$backup" "$root/original.wal";then echo WAL_REPAIR_FAULT_PARTIAL_BACKUP=PASS;else echo WAL_REPAIR_FAULT_PARTIAL_BACKUP=FAIL;failed=$((failed+1));fi
  partial_hash=$(sha256sum "$backup" | cut -d' ' -f1)
  run "$refuse" writer '^WAL_REFUSE_' "$ROOT/tests/l08-platform/wal-tail-refuse.expected"
  run "$refuse" reader '^WAL_REFUSE_' "$ROOT/tests/l08-platform/wal-tail-refuse-reader.expected"
  if cmp -s "$root/original.wal" "$root/blog.wal" && [[ "$partial_hash" == "$(sha256sum "$backup" | cut -d' ' -f1)" ]];then echo WAL_REPAIR_FAULT_PARTIAL_PRESERVED=PASS;else echo WAL_REPAIR_FAULT_PARTIAL_PRESERVED=FAIL;failed=$((failed+1));fi
 else
  if cmp -s "$backup" "$root/original.wal";then echo WAL_REPAIR_FAULT_ORIGINAL_BACKUP=PASS;else echo WAL_REPAIR_FAULT_ORIGINAL_BACKUP=FAIL;failed=$((failed+1));fi
  run "$bins/resume" resume '^WAL_TAIL_' "$ROOT/tests/l08-platform/wal-tail-resume.expected"
  cp "$root/blog.wal" "$root/after-resume.wal"
  run "$bins/verify" verify '^WAL_TAIL_' "$ROOT/tests/l08-platform/wal-tail-verify.expected"
  prefix_bytes=$(wc -c < "$root/committed-prefix.wal")
  if cmp -n "$prefix_bytes" -s "$root/committed-prefix.wal" "$root/blog.wal" && cmp -s "$root/after-resume.wal" "$root/blog.wal" && cmp -s "$backup" "$root/original.wal";then echo WAL_REPAIR_FAULT_RESTART_PRESERVED=PASS;else echo WAL_REPAIR_FAULT_RESTART_PRESERVED=FAIL;failed=$((failed+1));fi
 fi
done
printf 'WAL_REPAIR_FAULT_PASS=%s FAIL=%s\n' "$passed" "$failed"
[[ "$failed" == 0 ]]
