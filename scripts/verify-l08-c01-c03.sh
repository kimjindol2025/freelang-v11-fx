#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_OUT:-$(mktemp -d)}"
mkdir -p "$out/source/fixtures"
cp "$ROOT/tests/l08/"*.fl "$out/source/"
cp "$ROOT/tests/l08/fixtures/"*.fl "$out/source/fixtures/"
ln -s loaded.fl "$out/source/fixtures/loaded-link.fl"
l07_build_compiler "$out" >"$out/build.log" 2>&1
pass=0; fail=0
for spec in names redefine scope values default get strict load cycle order local; do
  if l07_build_app "$out" "$out/source/$spec.fl" "$spec" >"$out/$spec.build.log" 2>&1; then
    if [[ "$spec" == strict ]]; then strict=1; else strict=0; fi
    if env FL_STRICT="$strict" "$out/$spec" >"$out/$spec.actual" 2>"$out/$spec.stderr"; then rc=0; else rc=$?; fi
    cat "$out/$spec.actual"
    echo "$spec EXIT=$rc"
    if [[ "$rc" == 0 ]] && diff -u "$ROOT/tests/l08/$spec.expected" "$out/$spec.actual"; then
      echo "PASS $spec"; pass=$((pass+1))
    else echo "FAIL $spec"; fail=$((fail+1)); fi
  else
    cat "$out/$spec.build.log"
    echo "FAIL $spec BUILD_EXIT=1"; fail=$((fail+1))
  fi
done
if "$out/compiler" "$out/source/missing.fl" "$out/missing.c" >"$out/missing.log" 2>&1; then rc=0; else rc=$?; fi
cat "$out/missing.log"
echo "missing EXIT=$rc"
if [[ "$rc" != 0 ]] && [[ ! -s "$out/missing.c" ]]; then echo 'PASS missing'; pass=$((pass+1)); else echo 'FAIL missing'; fail=$((fail+1)); fi
printf 'L08_PASS=%s FAIL=%s\n' "$pass" "$fail"
[[ "$fail" == 0 ]]
