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
