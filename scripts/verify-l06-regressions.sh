#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L06_OUT:-$(mktemp -d)}"
mkdir -p "$out/l07" "$out/legacy/tests"
L07_OUT="$out/l07" bash "$ROOT/scripts/verify-l07-binary-io.sh"
if [[ ! -x "$out/compiler" ]]; then l07_build_compiler "$out" >"$out/legacy-build.log" 2>&1; fi
for spec in jit-test gc-stress-test; do
  "$out/compiler" "$ROOT/$spec.fl" "$out/legacy/$spec.c" >>"$out/legacy-build.log" 2>&1
  name="$spec-bin"; [[ "$spec" != gc-stress-test ]] || name=gc-stress-bin
  gcc -O2 -w -I "$ROOT/runtime" -o "$out/legacy/$name" "$out/legacy/$spec.c" \
    "${L07_SRCS[@]}" "$ROOT/runtime/jit.c" "${L07_LIBS[@]}" >>"$out/legacy-build.log" 2>&1
done
cp "$ROOT/tests/run-tests.sh" "$out/legacy/tests/"
cp "$ROOT/jit-test.fl" "$ROOT/gc-stress-test.fl" "$out/legacy/"
timeout 30 bash "$out/legacy/tests/run-tests.sh"
echo L06_EXISTING_REGRESSIONS=PASS
