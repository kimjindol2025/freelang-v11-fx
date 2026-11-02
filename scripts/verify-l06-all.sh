#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
out="${L06_OUT:-$(mktemp -d)}"
mkdir -p "$out"
export L06_OUT="$out"
run() {
  local phase="$1" script="$2" rc
  printf 'BEGIN %s\n' "$phase"
  if strace -f -e trace=execve -o "$out/$phase.execve.log" bash "$ROOT/$script" >"$out/$phase.log" 2>&1; then rc=0; else rc=$?; fi
  cat "$out/$phase.log"
  printf '%s_EXIT=%s\n' "$phase" "$rc" | tee "$out/$phase.exit"
  test "$rc" = 0
}
run REPRO scripts/verify-l06-repro.sh
run HTTP scripts/verify-l06-http.sh
run REGRESSIONS scripts/verify-l06-regressions.sh
printf 'BEGIN BOOTSTRAP\n'
# The unchanged upstream gate scans all visible processes. Isolate only this
# verification job so unrelated users' build arguments cannot cause a false fail.
if unshare --user --map-root-user --pid --fork --mount-proc \
    env L07_OUT="$out/bootstrap" bash "$ROOT/scripts/verify-l07-bootstrap.sh" >"$out/BOOTSTRAP.log" 2>&1; then rc=0; else rc=$?; fi
cat "$out/BOOTSTRAP.log"
printf 'BOOTSTRAP_EXIT=%s\n' "$rc" | tee "$out/BOOTSTRAP.exit"
test "$rc" = 0
bootstrap_traces=("$out/bootstrap/traces/"*.execve.log)
if [[ "${FL_BOOTSTRAP_AUDIT:-trace}" == static ]];then
 rg -qx 'BOOTSTRAP_STATIC_AUDIT=PASS' "$out/bootstrap/static/static-audit.txt"
 bootstrap_traces=()
fi
bash "$ROOT/scripts/audit-l07-execve.sh" "$out"/*.execve.log "${bootstrap_traces[@]}" | tee "$out/trace-audit.log"
echo L06_NATIVE_GATES=PASS
