#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_P2_OUT:-$(mktemp -d)}"
mkdir -p "$out"
l07_build_compiler "$out" >"$out/build.log" 2>&1
printf '\341\200X' > "$out/invalid"
printf '\355\240\200' > "$out/surrogate"
printf 'A\000B' > "$out/nul"
pass=0;fail=0
for spec in strings unicode bytes numbers arrays core output literals edges; do
    if l07_build_app "$out" "$ROOT/tests/l08-phase2/$spec.fl" "$spec" >"$out/$spec.build.log" 2>&1; then
        if env FL_STRICT=0 "$out/$spec" "$out/invalid" "$out/surrogate" "$out/nul" "$out/raw-copy" >"$out/$spec.actual" 2>"$out/$spec.stderr"; then rc=0; else rc=$?; fi
        cat "$out/$spec.actual"; cat "$out/$spec.stderr"
        printf '%s_PROGRAM_EXIT=%s\n' "$spec" "$rc"
        if [[ "$rc" == 0 ]] && diff -u "$ROOT/tests/l08-phase2/$spec.expected" "$out/$spec.actual"; then
            echo "PASS $spec";pass=$((pass+1))
        else echo "FAIL $spec";fail=$((fail+1));fi
    else rc=$?;cat "$out/$spec.build.log";echo "${spec}_BUILD_EXIT=$rc";echo "FAIL $spec";fail=$((fail+1));fi
done
if cmp -s "$out/surrogate" "$out/raw-copy";then echo RAW_L07_BYTES=PASS;else echo RAW_L07_BYTES=FAIL;fail=$((fail+1));fi
printf 'L08_P2_PASS=%s FAIL=%s\n' "$pass" "$fail"
[[ "$fail" == 0 ]]
