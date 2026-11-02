#!/usr/bin/env bash
# Caller records ps and holds the shared heavy-test lock for this entire gate.
set -Eeuo pipefail
ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)
out=${E08_OUT:?}
mkdir -p "$out"
run(){
 local tag=$1 log=$2 rc;shift 2
 if "$@" > "$log" 2>&1;then rc=0;else rc=$?;fi
 printf '%s_EXIT=%s\n' "$tag" "$rc" >> "$out/result.txt"
 [[ "$rc" == 0 ]]
}
run SMALL_LEAKS "$out/small-leaks.log" env SMALL_LEAKS_OUT="$out/small-leaks" SMALL_LEAKS_COMPILER="${WORKER_COMPILER:?}" SMALL_LEAKS_SANITIZER_LIBDIR="${WORKER_SANITIZER_LIBDIR:?}" bash "$ROOT/scripts/verify-small-leaks.sh"
run WORKER "$out/worker.log" env WORKER_OUT="$out/worker" bash "$ROOT/scripts/verify-worker-cleanup.sh"
# CLI, real Chrome, L07, 12 leak cases, then the full native suite + bootstrap.
run FULL "$out/full.log" env E07_OUT="$out/full" bash "$ROOT/scripts/verify-e07-completion.sh"
mapfile -d '' traces < <(find "$out" -type f -name '*.execve.log' -print0)
bash "$ROOT/scripts/audit-l07-execve.sh" "${traces[@]}" > "$out/audit.txt"
printf 'E08_COMPLETION=PASS\n' >> "$out/result.txt"
