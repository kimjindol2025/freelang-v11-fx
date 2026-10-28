#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
out="${L08_OUT:-$(mktemp -d)}"
mkdir -p "$out"
export L08_OUT="$out"
for phase in self http; do
    if [[ "$phase" == self ]]; then script=verify-l08-c01-c03.sh; else script=verify-l08-http.sh; fi
    if strace -f -e trace=execve -o "$out/$phase.execve.log" bash "$ROOT/scripts/$script" >"$out/$phase.log" 2>&1; then rc=0; else rc=$?; fi
    cat "$out/$phase.log"
    printf '%s_EXIT=%s\n' "$phase" "$rc"
    [[ "$rc" == 0 ]]
done
# The unchanged L06 driver runs L06 HTTP/TLS, L07, legacy regression, then
# verify-native-bootstrap.sh with its existing trace retention and PID isolation.
if L06_OUT="$out/regressions" bash "$ROOT/scripts/verify-l06-all.sh" >"$out/regressions.log" 2>&1; then rc=0; else rc=$?; fi
cat "$out/regressions.log"
printf 'REGRESSIONS_AND_BOOTSTRAP_EXIT=%s\n' "$rc"
[[ "$rc" == 0 ]]
bash "$ROOT/scripts/audit-l07-execve.sh" "$out/"*.execve.log \
    "$out/regressions/"*.execve.log "$out/regressions/bootstrap/traces/"*.execve.log
echo L08_C01_C03_GATES=PASS
