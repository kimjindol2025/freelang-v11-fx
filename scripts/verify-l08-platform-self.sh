#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_PLATFORM_OUT:-$(mktemp -d)}"
mkdir -p "$out"
l07_build_compiler "$out" > "$out/compiler-build.log" 2>&1
pass=0;fail=0
for spec in environment exit-default exit-seven;do
 if l07_build_app "$out" "$ROOT/tests/l08-platform/$spec.fl" "$spec" > "$out/$spec.build.log" 2>&1;then
  if env -u L08_ABSENT_TEST L08_EMPTY_TEST= L08_VALUE_TEST=한글 timeout 30 "$out/$spec" > "$out/$spec.actual" 2> "$out/$spec.stderr";then rc=0;else rc=$?;fi
  cat "$out/$spec.actual" "$out/$spec.stderr"
  printf '%s_PROGRAM_EXIT=%s\n' "$spec" "$rc"
  expected_rc=0; [[ "$spec" != exit-seven ]] || expected_rc=7
  if [[ "$spec" == environment ]];then
   if [[ "$rc" == "$expected_rc" ]] && [[ ! -s "$out/$spec.stderr" ]] && diff -u "$ROOT/tests/l08-platform/environment.expected" "$out/$spec.actual";then pass=$((pass+1));else fail=$((fail+1));fi
  elif [[ "$rc" == "$expected_rc" && ! -s "$out/$spec.actual" && ! -s "$out/$spec.stderr" ]];then pass=$((pass+1));else fail=$((fail+1));fi
 else
  rc=$?;cat "$out/$spec.build.log";printf '%s_BUILD_EXIT=%s\n' "$spec" "$rc";fail=$((fail+1))
 fi
done
printf 'L08_C18_PASS=%s FAIL=%s\n' "$pass" "$fail"
[[ "$fail" == 0 ]]
