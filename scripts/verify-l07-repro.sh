#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L07_OUT:-$(mktemp -d)}"
mkdir -p "$out"
l07_build_compiler "$out" >"$out/build.log" 2>&1
l07_build_app "$out" "$ROOT/tests/l07/repro.fl" repro >>"$out/build.log" 2>&1
printf 'A\000B' >"$out/input"
printf 'P' >"$out/appended"
printf 'PA\000B' >"$out/append-answer"
"$out/repro" "$out/input" "$out/output" "$out/appended" >"$out/repro.stdout"
{
  cat "$out/repro.stdout"
  printf 'L07_REPRO_WRITE_BYTES=%s\n' "$(wc -c <"$out/output" | tr -d ' ')"
  printf 'L07_REPRO_APPEND_BYTES=%s\n' "$(wc -c <"$out/appended" | tr -d ' ')"
  if cmp -s "$out/input" "$out/output"; then echo L07_REPRO_WRITE_MATCH=PASS; else echo L07_REPRO_WRITE_MATCH=FAIL; fi
  if cmp -s "$out/append-answer" "$out/appended"; then echo L07_REPRO_APPEND_MATCH=PASS; else echo L07_REPRO_APPEND_MATCH=FAIL; fi
} | tee "$out/repro.actual"
diff -u "$ROOT/tests/l07/repro.expected" "$out/repro.actual"
