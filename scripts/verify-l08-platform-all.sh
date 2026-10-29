#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
out="${L08_PLATFORM_OUT:-$(mktemp -d)}"
mkdir -p "$out"
if L08_PLATFORM_OUT="$out/self" strace -f -e trace=execve -o "$out/self.execve.log" bash "$ROOT/scripts/verify-l08-platform-self.sh" > "$out/self.log" 2>&1;then rc=0;else rc=$?;fi
cat "$out/self.log"
printf 'PLATFORM_SELF_EXIT=%s\n' "$rc"
[[ "$rc" == 0 ]]
for phase in fd-basic fd-extended json;do
 case "$phase" in
  fd-basic) setting=L08_FD_OUT;script=verify-l08-fd-self.sh;tag=FD_BASIC;;
  fd-extended) setting=L08_FD_FAULT_OUT;script=verify-l08-fd-faults.sh;tag=FD_EXTENDED;;
  json) setting=L08_JSON_OUT;script=verify-l08-json-self.sh;tag=JSON;;
 esac
 if env "$setting=$out/$phase" strace -f -e trace=execve,clone,clone3 -o "$out/$phase.execve.log" bash "$ROOT/scripts/$script" > "$out/$phase.log" 2>&1;then rc=0;else rc=$?;fi
 cat "$out/$phase.log"
 printf '%s_EXIT=%s\n' "$tag" "$rc"
 [[ "$rc" == 0 ]]
done
if L08_P3_OUT="$out/previous" bash "$ROOT/scripts/verify-l08-phase3-all.sh" > "$out/previous.log" 2>&1;then rc=0;else rc=$?;fi
cat "$out/previous.log"
printf 'PREVIOUS_GATES_EXIT=%s\n' "$rc"
[[ "$rc" == 0 ]]
mapfile -d '' traces < <(find "$out" -type f -name '*.execve.log' -print0)
bash "$ROOT/scripts/audit-l07-execve.sh" "${traces[@]}"
echo L08_PLATFORM_CURRENT_GATES=PASS
