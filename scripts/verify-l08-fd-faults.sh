#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_FD_FAULT_OUT:-$(mktemp -d)}"
mkdir -p "$out/root"
l07_build_compiler "$out" > "$out/compiler-build.log" 2>&1
l07_build_app "$out" "$ROOT/tests/l08-platform/fd-faults.fl" faults > "$out/faults-build.log" 2>&1
gcc -O2 -Wall -Wextra -fPIC -shared "$ROOT/tests/l08-platform/fd-faults.c" -o "$out/faults.so" -ldl
pass=0;fail=0
for mode in write-partial write-zero fsync-eintr fsync-eio close-eintr close-eio read-eintr read-short;do
 printf seed > "$out/root/fault.wal"
 printf '한\0끝' > "$out/root/payload"
 printf 'FAULT_MODE=%s\n' "$mode" > "$out/$mode.expected-stderr"
 [[ "$mode" != close-* ]] || printf 'FD_REUSE_GUARD=PASS\n' >> "$out/$mode.expected-stderr"
 if (cd "$out/root";env FL_FILE_BASE="$out/root" L08_FD_FAULT="$mode" LD_PRELOAD="$out/faults.so" timeout 30 "$out/faults" "$out/root") > "$out/$mode.actual" 2> "$out/$mode.stderr";then rc=0;else rc=$?;fi
 cat "$out/$mode.actual" "$out/$mode.stderr"
 printf '%s_PROGRAM_EXIT=%s\n' "$mode" "$rc"
 if [[ "$rc" == 0 ]] && diff -u "$ROOT/tests/l08-platform/fd-$mode.expected" "$out/$mode.actual" && diff -u "$out/$mode.expected-stderr" "$out/$mode.stderr";then pass=$((pass+1));else fail=$((fail+1));fi
done
printf 'L08_FD_FAULT_PASS=%s FAIL=%s\n' "$pass" "$fail"
[[ "$fail" == 0 ]]
# Additional public API checks, independent of fault injection.
mkdir -p "$out/outside"
printf KEEP > "$out/outside/sentinel"
ln -s "$out/outside/sentinel" "$out/root/link-file"
ln -s "$out/outside" "$out/root/link-dir"
ln -s "$out/root" "$out/base-link"
mkfifo "$out/root/fifo"
truncate -s 67108864 "$out/root/limit.wal"
for spec in fd-edges fd-base-link fd-concurrent;do
 if [[ "$spec" == fd-concurrent ]];then
  "$out/compiler" --extern fd-test-barrier --extern fd-test-thread-count "$ROOT/tests/l08-platform/$spec.fl" "$out/$spec.c" > "$out/$spec.build.log" 2>&1
  gcc -O2 -w -D_GNU_SOURCE -I "$ROOT/runtime" -include "$ROOT/tests/l08-platform/fd-concurrent.h" -o "$out/$spec" "$out/$spec.c" "$ROOT/tests/l08-platform/fd-concurrent.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}" >> "$out/$spec.build.log" 2>&1
 else l07_build_app "$out" "$ROOT/tests/l08-platform/$spec.fl" "$spec" > "$out/$spec.build.log" 2>&1;fi
 base="$out/root"; [[ "$spec" != fd-base-link ]] || base="$out/base-link"
 if (cd "$out/root";env FL_FILE_BASE="$base" timeout 30 "$out/$spec" "$out/root") > "$out/$spec.actual" 2> "$out/$spec.stderr";then rc=0;else rc=$?;fi
 cat "$out/$spec.actual" "$out/$spec.stderr"
 printf '%s_PROGRAM_EXIT=%s\n' "$spec" "$rc"
 if [[ "$rc" == 0 && ! -s "$out/$spec.stderr" ]] && diff -u "$ROOT/tests/l08-platform/$spec.expected" "$out/$spec.actual";then pass=$((pass+1));else fail=$((fail+1));fi
done
{
 for worker in {0..7};do for number in {0..99};do printf '한:%s:%s\n' "$worker" "$number";done;done
} | LC_ALL=C sort > "$out/concurrent.expected"
if [[ -f "$out/root/concurrent.wal" ]];then
 LC_ALL=C sort "$out/root/concurrent.wal" > "$out/concurrent.sorted"
 printf 'CONCURRENT_LINES=%s\n' "$(wc -l < "$out/root/concurrent.wal")"
 if cmp -s "$out/concurrent.expected" "$out/concurrent.sorted";then echo CONCURRENT_RECORDS=PASS;else echo CONCURRENT_RECORDS=FAIL;fail=$((fail+1));fi
else echo CONCURRENT_RECORDS=MISSING;fail=$((fail+1));fi
if [[ $(cat "$out/outside/sentinel") == KEEP ]];then echo OUTSIDE_SENTINEL=PASS;else echo OUTSIDE_SENTINEL=FAIL;fail=$((fail+1));fi
printf 'L08_FD_EXTENDED_PASS=%s FAIL=%s\n' "$pass" "$fail"
[[ "$fail" == 0 ]]
