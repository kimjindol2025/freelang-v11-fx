#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L07_OUT:-$(mktemp -d)}"
mkdir -p "$out/fixtures"
l07_build_compiler "$out" >"$out/build.log" 2>&1
gcc -O2 -Wall -Wextra -o "$out/fixtures-bin" "$ROOT/tests/l07/fixtures.c"
"$out/fixtures-bin" "$out/fixtures"
inputs=(empty text nul-first nul-middle nul-last all-bytes non-utf8 boundary-8191
  boundary-8192 boundary-8193 multi-block sentinel readonly hardlink limit too-large)
(cd "$out/fixtures"; sha256sum "${inputs[@]}") >"$out/input-before.sha256"
sha256sum "$out/outside" | cut -d' ' -f1 >"$out/outside-before.sha256"
"$out/compiler" --extern l07-check --extern l07-error --extern l07-same \
  --extern l07-append-matches --extern l07-raw-size --extern l07-fd-count \
  --extern l07-arm --extern l07-bad-value --extern l07-finish \
  "$ROOT/tests/l07/self-test.fl" "$out/self-test.c" >>"$out/build.log" 2>&1
wraps=()
for symbol in fread fwrite ferror fflush fclose malloc fstat ftruncate fdopen; do wraps+=("-Wl,--wrap=$symbol"); done
gcc -O2 -w -D_GNU_SOURCE -I "$ROOT/runtime" -include "$ROOT/tests/l07/support.h" \
  -o "$out/self-test" "$out/self-test.c" "$ROOT/tests/l07/support.c" \
  "${L07_SRCS[@]}" "${L07_LIBS[@]}" "${wraps[@]}" >>"$out/build.log" 2>&1
FL_FILE_BASE="$out/fixtures" timeout 60 "$out/self-test" | tee "$out/self-test.log"
(cd "$out/fixtures"; sha256sum "${inputs[@]}") >"$out/input-after.sha256"
cmp "$out/input-before.sha256" "$out/input-after.sha256"
sha256sum "$out/outside" | cut -d' ' -f1 >"$out/outside-after.sha256"
cmp "$out/outside-before.sha256" "$out/outside-after.sha256"
echo L07_INPUT_AND_OUTSIDE_HASHES=UNCHANGED
# Existing runtime regression uses one hardcoded temporary name; isolate only
# that fixture path, keeping all original assertions and expected values.
sed "s@/tmp/fl_test_runtime.txt@$out/runtime-text.txt@g" "$ROOT/runtime/test_runtime.c" >"$out/runtime-test.c"
gcc -O2 -w -I "$ROOT/runtime" -o "$out/runtime-test" "$out/runtime-test.c" \
  "${L07_SRCS[@]}" "${L07_LIBS[@]}" >>"$out/build.log" 2>&1
env -u FL_FILE_BASE "$out/runtime-test" >"$out/runtime-test.log" 2>&1
cat "$out/runtime-test.log"
echo L07_RUNTIME_REGRESSION=PASS
