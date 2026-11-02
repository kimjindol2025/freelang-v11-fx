#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
out="${L08_P3_OUT:-$(mktemp -d)}"
mkdir -p "$out"
if L08_P3_OUT="$out/self" strace -f -e trace=execve -o "$out/self.execve.log" \
    bash "$ROOT/scripts/verify-l08-phase3-self.sh" > "$out/self.log" 2>&1;then rc=0;else rc=$?;fi
cat "$out/self.log"
printf 'PHASE3_SELF_EXIT=%s\n' "$rc"
[[ "$rc" == 0 ]]
# The previous driver retains L08 phases 1/2, L06, L07 and unchanged bootstrap.
if L08_P2_OUT="$out/phase2" bash "$ROOT/scripts/verify-l08-phase2-all.sh" > "$out/phase2.log" 2>&1;then rc=0;else rc=$?;fi
cat "$out/phase2.log"
printf 'PREVIOUS_GATES_EXIT=%s\n' "$rc"
[[ "$rc" == 0 ]]
mapfile -d '' traces < <(find "$out" -type f -name '*.execve.log' -print0)
bash "$ROOT/scripts/audit-l07-execve.sh" "${traces[@]}"
echo L08_C04_C07_C10_GATES=PASS
