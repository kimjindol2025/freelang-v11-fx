#!/usr/bin/env bash
# Caller checks ps and holds the shared heavy-test lock.
set -Eeuo pipefail
ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)
source "$ROOT/scripts/native-lsan-trace.sh"
out=${E07_OUT:?};compiler=${WORKER_COMPILER:?};libdir=${WORKER_SANITIZER_LIBDIR:?}
blog=${WORKER_BLOG_SOURCE:?}
[[ ! -e "$blog/data/blog.wal" && -f "$blog/web/block-model.fl" ]]
mkdir -p "$out/maintain" "$out/pre/timer"
run() {
 local tag=$1 log=$2 rc;shift 2
 if "$@" > "$log" 2>&1;then rc=0;else rc=$?;fi
 printf '%s_EXIT=%s\n' "$tag" "$rc" >> "$out/result.txt"
 [[ "$rc" == 0 ]]
}
export RUNTIME_DIR="$ROOT/runtime" CGC_MAIN="$ROOT/self/cgc-main.fl"
export ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 LSAN_OPTIONS=exitcode=23
run CLI "$out/maintain/cli.log" env CLI_LIFETIME_OUT="$out/maintain/cli" CLI_LIFETIME_COMPILER="$compiler" CLI_LIFETIME_SANITIZER_LIBDIR="$libdir" bash "$ROOT/scripts/verify-cli-lifetime.sh"
for scenario in timers timer-exception semantics events dom blog-model;do
 run "CHROME_$scenario" "$out/maintain/$scenario.log" env L01_TARGET_SCENARIO="$scenario" L01_TARGET_BLOG_SOURCE="$blog" L01_TARGET_COMPILER="$compiler" L01_TARGET_OUT="$out/maintain/$scenario" strace -f -e trace=execve -o "$out/maintain/$scenario.execve.log" bash "$ROOT/scripts/verify-l01-browser-target.sh"
done
run CLI_OPTIONS "$out/maintain/options.log" env L01_NATIVE_COMPILER="$compiler" L01_CLI_OUT="$out/maintain/options" strace -f -e trace=execve -o "$out/maintain/options.execve.log" bash "$ROOT/scripts/verify-l01-cli-options.sh"
run CLOCK "$out/maintain/clock.log" env L08_NATIVE_COMPILER="$compiler" L08_CLOCK_OUT="$out/maintain/clock" strace -f -e trace=execve -o "$out/maintain/clock.execve.log" bash "$ROOT/scripts/verify-l08-clock.sh"
run L07 "$out/maintain/l07.log" env L07_OUT="$out/maintain/l07" unshare --user --map-root-user --pid --fork --mount-proc setpriv --bounding-set=-dac_override,-dac_read_search strace -f -e trace=execve -o "$out/maintain/l07.execve.log" bash "$ROOT/scripts/verify-l07-binary-io.sh"
# Original scripts retain their fixed oracles; LSan stays enabled outside ptrace.
for group in atom-update atom-lock sort-context atom-lifetime arena-cache;do
 case "$group" in
  atom-update) prefix=L08_ATOM_UPDATE;;
  atom-lock) prefix=L08_ATOM_LOCK;;
  sort-context) prefix=L08_SORT_CONTEXT;;
  atom-lifetime) prefix=L08_ATOM_LIFETIME;;
  arena-cache) prefix=L08_ARENA_CACHE;;
 esac
 export "${prefix}_OUT=$out/pre/$group" "${prefix}_COMPILER=$compiler" "${prefix}_SANITIZER_LIBDIR=$libdir"
 run "PRE_$group" "$out/pre/$group.log" platform_trace -f -e trace=execve -o "$out/pre/$group.execve.log" bash "$ROOT/scripts/verify-l08-$group.sh"
done
ln -s "$compiler" "$out/pre/timer/compiler"
run PRE_TIMER "$out/pre/timer-sanitizers.log" env L08_TIMER_SANITIZER_LIBDIR="$libdir" L08_TIMER_NATIVE_OUT="$out/pre/timer" L08_TIMER_SANITIZER_OUT="$out/pre/timer-sanitizers" bash "$ROOT/scripts/verify-l08-timer-sanitizers.sh"
# The five groups cover 2+1+1+2+1 cases; timer covers 5. Bootstrap follows all 12.
printf 'REMAINING_LEAK_CASES=12/12\n' >> "$out/result.txt"
export L08_TIMER_SANITIZER_LIBDIR="$libdir" L01_TARGET_BLOG_SOURCE="$blog"
run NATIVE_ALL "$out/all.log" env L08_PLATFORM_OUT="$out/all" unshare --user --map-root-user --pid --fork --mount-proc setpriv --bounding-set=-dac_override,-dac_read_search bash "$ROOT/scripts/verify-l08-platform-all.sh"
mapfile -d '' traces < <(find "$out" -type f -name '*.execve.log' -print0)
bash "$ROOT/scripts/audit-l07-execve.sh" "${traces[@]}" > "$out/audit.txt"
printf 'E07_COMPLETION=PASS\n' >> "$out/result.txt"
