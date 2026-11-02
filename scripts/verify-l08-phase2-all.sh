#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
out="${L08_P2_OUT:-$(mktemp -d)}"
mkdir -p "$out"
if L08_P2_OUT="$out/self" strace -f -e trace=execve -o "$out/self.execve.log" \
    bash "$ROOT/scripts/verify-l08-phase2-self.sh" >"$out/self.log" 2>&1; then rc=0; else rc=$?; fi
cat "$out/self.log"
printf 'PHASE2_SELF_EXIT=%s\n' "$rc"
[[ "$rc" == 0 ]]
# Preserve the preceding gates, including the unchanged bootstrap verifier.
if L08_OUT="$out/phase1" bash "$ROOT/scripts/verify-l08-all.sh" >"$out/phase1.log" 2>&1; then rc=0; else rc=$?; fi
cat "$out/phase1.log"
printf 'PREVIOUS_GATES_EXIT=%s\n' "$rc"
[[ "$rc" == 0 ]]
bootstrap_traces=("$out/phase1/regressions/bootstrap/traces/"*.execve.log)
if [[ "${FL_BOOTSTRAP_AUDIT:-trace}" == static ]];then
 rg -qx 'BOOTSTRAP_STATIC_AUDIT=PASS' "$out/phase1/regressions/bootstrap/static/static-audit.txt"
 bootstrap_traces=()
fi
bash "$ROOT/scripts/audit-l07-execve.sh" "$out/self.execve.log" \
    "$out/phase1/"*.execve.log "$out/phase1/regressions/"*.execve.log \
    "${bootstrap_traces[@]}"
echo L08_C05_C06_C08_C09_GATES=PASS
