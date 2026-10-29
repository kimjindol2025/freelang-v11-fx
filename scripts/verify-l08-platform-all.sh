#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
out="${L08_PLATFORM_OUT:-$(mktemp -d)}"
mkdir -p "$out"
if L08_PLATFORM_OUT="$out/self" strace -f -e trace=execve -o "$out/self.execve.log" bash "$ROOT/scripts/verify-l08-platform-self.sh" > "$out/self.log" 2>&1;then rc=0;else rc=$?;fi
cat "$out/self.log"
printf 'PLATFORM_SELF_EXIT=%s\n' "$rc"
[[ "$rc" == 0 ]]
if L08_SNAPSHOT_OUT="$out/atom-snapshot" L08_SNAPSHOT_COMPILER="$out/self/compiler" L08_SNAPSHOT_SANITIZER_LIBDIR="${L08_TIMER_SANITIZER_LIBDIR:-}" strace -f -e trace=execve -o "$out/atom-snapshot.execve.log" bash "$ROOT/scripts/verify-l08-atom-snapshot.sh" > "$out/atom-snapshot.log" 2>&1;then rc=0;else rc=$?;fi
cat "$out/atom-snapshot.log";printf 'ATOM_SNAPSHOT_EXIT=%s\n' "$rc";[[ "$rc" == 0 ]]
if L08_TAIL_OUT="$out/tail" L08_TAIL_NATIVE_COMPILER="$out/self/compiler" L08_TAIL_SANITIZER_LIBDIR="${L08_TIMER_SANITIZER_LIBDIR:-}" strace -f -e trace=execve -o "$out/tail.execve.log" bash "$ROOT/scripts/verify-l08-tail-self.sh" > "$out/tail.log" 2>&1;then rc=0;else rc=$?;fi
cat "$out/tail.log";printf 'TAIL_SELF_EXIT=%s\n' "$rc";[[ "$rc" == 0 ]]
if L08_DEFINE_OUT="$out/define-sequence" L08_DEFINE_COMPILER="$out/self/compiler" strace -f -e trace=execve -o "$out/define-sequence.execve.log" bash "$ROOT/scripts/verify-l08-define-sequence.sh" > "$out/define-sequence.log" 2>&1;then rc=0;else rc=$?;fi
cat "$out/define-sequence.log";printf 'DEFINE_SEQUENCE_EXIT=%s\n' "$rc";[[ "$rc" == 0 ]]
if L08_EXTERN_OUT="$out/extern" L08_EXTERN_COMPILER="$out/self/compiler" strace -f -e trace=execve -o "$out/extern.execve.log" bash "$ROOT/scripts/verify-l08-extern-self.sh" > "$out/extern.log" 2>&1;then rc=0;else rc=$?;fi
cat "$out/extern.log";printf 'EXTERN_SELF_EXIT=%s\n' "$rc";[[ "$rc" == 0 ]]
if L08_ATOM_UPDATE_OUT="$out/atom-update" L08_ATOM_UPDATE_COMPILER="$out/self/compiler" L08_ATOM_UPDATE_SANITIZER_LIBDIR="${L08_TIMER_SANITIZER_LIBDIR:-}" strace -f -e trace=execve,clone,clone3 -o "$out/atom-update.execve.log" bash "$ROOT/scripts/verify-l08-atom-update.sh" > "$out/atom-update.log" 2>&1;then rc=0;else rc=$?;fi
cat "$out/atom-update.log";printf 'ATOM_UPDATE_EXIT=%s\n' "$rc";[[ "$rc" == 0 ]]
if L08_ATOM_LOCK_OUT="$out/atom-lock" L08_ATOM_LOCK_COMPILER="$out/self/compiler" L08_ATOM_LOCK_SANITIZER_LIBDIR="${L08_TIMER_SANITIZER_LIBDIR:-}" strace -f -e trace=execve,clone,clone3 -o "$out/atom-lock.execve.log" bash "$ROOT/scripts/verify-l08-atom-lock.sh" > "$out/atom-lock.log" 2>&1;then rc=0;else rc=$?;fi
cat "$out/atom-lock.log";printf 'ATOM_LOCK_EXIT=%s\n' "$rc";[[ "$rc" == 0 ]]
if L08_SORT_CONTEXT_OUT="$out/sort-context" L08_SORT_CONTEXT_COMPILER="$out/self/compiler" L08_SORT_CONTEXT_SANITIZER_LIBDIR="${L08_TIMER_SANITIZER_LIBDIR:-}" strace -f -e trace=execve,clone,clone3 -o "$out/sort-context.execve.log" bash "$ROOT/scripts/verify-l08-sort-context.sh" > "$out/sort-context.log" 2>&1;then rc=0;else rc=$?;fi
cat "$out/sort-context.log";printf 'SORT_CONTEXT_EXIT=%s\n' "$rc";[[ "$rc" == 0 ]]
if L08_HTTP_ATOM_OUT="$out/http-atom" L08_HTTP_ATOM_COMPILER="$out/self/compiler" L08_HTTP_ATOM_SANITIZER_LIBDIR="${L08_TIMER_SANITIZER_LIBDIR:-}" strace -f -e trace=execve,clone,clone3 -o "$out/http-atom.execve.log" bash "$ROOT/scripts/verify-l08-http-atom.sh" > "$out/http-atom.log" 2>&1;then rc=0;else rc=$?;fi
cat "$out/http-atom.log";printf 'HTTP_ATOM_EXIT=%s\n' "$rc";[[ "$rc" == 0 ]]
if L08_ATOM_LIFETIME_OUT="$out/atom-lifetime" L08_ATOM_LIFETIME_COMPILER="$out/self/compiler" L08_ATOM_LIFETIME_SANITIZER_LIBDIR="${L08_TIMER_SANITIZER_LIBDIR:-}" strace -f -e trace=execve,clone,clone3 -o "$out/atom-lifetime.execve.log" bash "$ROOT/scripts/verify-l08-atom-lifetime.sh" > "$out/atom-lifetime.log" 2>&1;then rc=0;else rc=$?;fi
cat "$out/atom-lifetime.log";printf 'ATOM_LIFETIME_EXIT=%s\n' "$rc";[[ "$rc" == 0 ]]
if L08_ARENA_CACHE_OUT="$out/arena-cache" L08_ARENA_CACHE_COMPILER="$out/self/compiler" L08_ARENA_CACHE_SANITIZER_LIBDIR="${L08_TIMER_SANITIZER_LIBDIR:-}" strace -f -e trace=execve -o "$out/arena-cache.execve.log" bash "$ROOT/scripts/verify-l08-arena-cache.sh" > "$out/arena-cache.log" 2>&1;then rc=0;else rc=$?;fi
cat "$out/arena-cache.log";printf 'ARENA_CACHE_EXIT=%s\n' "$rc";[[ "$rc" == 0 ]]
for phase in timer tcp fd-basic fd-extended json auth http feed http-client;do
 case "$phase" in
  timer) setting=L08_TIMER_OUT;script=verify-l08-timer-self.sh;tag=TIMER_PARTIAL;;
  tcp) setting=L08_TCP_OUT;script=verify-l08-tcp-self.sh;tag=TCP;;
  fd-basic) setting=L08_FD_OUT;script=verify-l08-fd-self.sh;tag=FD_BASIC;;
  fd-extended) setting=L08_FD_FAULT_OUT;script=verify-l08-fd-faults.sh;tag=FD_EXTENDED;;
  json) setting=L08_JSON_OUT;script=verify-l08-json-self.sh;tag=JSON;;
  auth) setting=L08_AUTH_OUT;script=verify-l08-auth-self.sh;tag=AUTH_PARTIAL;;
  http) setting=L08_HTTP_PLATFORM_OUT;script=verify-l08-http-platform.sh;tag=HTTP_PLATFORM;;
  feed) setting=L08_FEED_OUT;script=verify-l08-feed-self.sh;tag=FEED;;
  http-client) setting=L08_HTTP_CLIENT_OUT;script=verify-l08-http-client-self.sh;tag=HTTP_CLIENT;;
 esac
 if env "$setting=$out/$phase" strace -f -e trace=execve,clone,clone3 -o "$out/$phase.execve.log" bash "$ROOT/scripts/$script" > "$out/$phase.log" 2>&1;then rc=0;else rc=$?;fi
 cat "$out/$phase.log"
 printf '%s_EXIT=%s\n' "$tag" "$rc"
 [[ "$rc" == 0 ]]
 if [[ "$phase" == timer ]];then
  if [[ -n ${L08_TIMER_SANITIZER_LIBDIR:-} ]];then
   if L08_TIMER_NATIVE_OUT="$out/timer" L08_TIMER_SANITIZER_OUT="$out/timer-sanitizers" strace -f -e trace=execve -o "$out/timer-sanitizers.execve.log" bash "$ROOT/scripts/verify-l08-timer-sanitizers.sh" > "$out/timer-sanitizers.log" 2>&1;then sanitizer_rc=0;else sanitizer_rc=$?;fi
   cat "$out/timer-sanitizers.log";printf 'TIMER_SANITIZER_EXIT=%s\n' "$sanitizer_rc";[[ "$sanitizer_rc" == 0 ]]
  else echo TIMER_SANITIZERS=UNEXECUTED_MISSING_LIBDIR;fi
 fi
done
if L08_P3_OUT="$out/previous" bash "$ROOT/scripts/verify-l08-phase3-all.sh" > "$out/previous.log" 2>&1;then rc=0;else rc=$?;fi
cat "$out/previous.log"
printf 'PREVIOUS_GATES_EXIT=%s\n' "$rc"
[[ "$rc" == 0 ]]
mapfile -d '' traces < <(find "$out" -type f -name '*.execve.log' -print0)
bash "$ROOT/scripts/audit-l07-execve.sh" "${traces[@]}"
echo L08_PLATFORM_CURRENT_GATES=PASS
