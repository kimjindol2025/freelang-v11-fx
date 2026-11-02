#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_FD_OUT:-$(mktemp -d)}"
mkdir -p "$out/root"
l07_build_compiler "$out" > "$out/compiler-build.log" 2>&1
for spec in fd-writer fd-reader;do l07_build_app "$out" "$ROOT/tests/l08-platform/$spec.fl" "$spec" > "$out/$spec.build.log" 2>&1;done
pass=0;fail=0
for iteration in {0..10};do
 spec=fd-reader; [[ "$iteration" != 0 ]] || spec=fd-writer
 if (cd "$out/root";env FL_FILE_BASE="$out/root" timeout 30 "$out/$spec" "$out/root") > "$out/$spec-$iteration.actual" 2> "$out/$spec-$iteration.stderr";then rc=0;else rc=$?;fi
 cat "$out/$spec-$iteration.actual" "$out/$spec-$iteration.stderr"
 printf '%s_%s_PROGRAM_EXIT=%s\n' "$spec" "$iteration" "$rc"
 if [[ "$rc" == 0 && ! -s "$out/$spec-$iteration.stderr" ]] && diff -u "$ROOT/tests/l08-platform/$spec.expected" "$out/$spec-$iteration.actual";then pass=$((pass+1));else fail=$((fail+1));fi
done
printf 'L08_FD_BASIC_PASS=%s FAIL=%s\n' "$pass" "$fail"
[[ "$fail" == 0 ]]
