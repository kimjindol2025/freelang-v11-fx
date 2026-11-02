#!/usr/bin/env bash
# Run the existing sanitized regression binaries outside ptrace for LSan.
set -Eeuo pipefail
ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)
native=${CLI_LEAK_NATIVE_OUT:?}
out=${CLI_LEAK_OUT:?}
libdir=${CLI_LEAK_SANITIZER_LIBDIR:?}
mkdir -p "$out"
export LD_LIBRARY_PATH="$libdir${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export ASAN_OPTIONS=detect_leaks=1:halt_on_error=1
export UBSAN_OPTIONS=halt_on_error=1 LSAN_OPTIONS=exitcode=23
pass=0
for case_spec in \
 atom-snapshot/atom-snapshot:atom-snapshot \
 atom-snapshot/atom-snapshot-memory:- \
 atom-update/atom-update:atom-update \
 atom-update/atom-update-alias:atom-update-alias \
 atom-lock/atom-lock:atom-lock \
 sort-context/main:sort-context \
 atom-lifetime/atom-lifetime:atom-lifetime \
 atom-lifetime/atom-cycles:atom-cycles \
 arena-cache/main:arena-cache \
 timer-sanitizers/timer-memory:- \
 timer-sanitizers/timer-serialization:- \
 timer-sanitizers/timer-arena-oom:- \
 timer-sanitizers/timer:timer; do
 relative=${case_spec%:*}; oracle=${case_spec#*:}; name=${relative//\//-}
 binary="$native/$relative"
 [[ -x "$binary" ]]
 readelf -d "$binary" | rg 'Shared library: \[libasan\.so' >/dev/null
 if timeout 120 "$binary" > "$out/$name.stdout" 2> "$out/$name.stderr"; then rc=0; else rc=$?; fi
 cat "$out/$name.stdout" "$out/$name.stderr"
 printf '%s_LSAN_EXIT=%s\n' "$name" "$rc"
 [[ "$rc" == 0 && ! -s "$out/$name.stderr" ]]
 if [[ "$oracle" != - ]]; then
  diff -u "$ROOT/tests/l08-platform/$oracle.expected" "$out/$name.stdout"
 else
  # Existing C tests assert memory boundaries themselves; do not derive an oracle from a run.
  rg -q '=PASS' "$out/$name.stdout"
  ! rg -q '=FAIL' "$out/$name.stdout"
 fi
 pass=$((pass+1))
done
binary="$native/timer-sanitizers/timer-http"
readelf -d "$binary" | rg 'Shared library: \[libasan\.so' >/dev/null
if L08_TIMER_NO_VISIT_OUT="$out/timer-http" L08_TIMER_NO_VISIT_SERVER="$binary" bash "$ROOT/scripts/verify-l08-timer-no-visit.sh" > "$out/timer-http.log" 2>&1; then rc=0; else rc=$?; fi
cat "$out/timer-http.log" "$out/timer-http/server.stderr"
printf 'TIMER_HTTP_LSAN_EXIT=%s\n' "$rc"
[[ "$rc" == 0 ]]
# http.c logs route/start/shutdown diagnostics on stderr even on success.
! rg -q 'Sanitizer|runtime error:|SUMMARY:' "$out/timer-http/server.stderr"
pass=$((pass+1))
printf 'REGRESSION_LEAK_CASES=%s/14\n' "$pass"
