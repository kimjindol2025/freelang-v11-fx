#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_JSON_OUT:-$(mktemp -d)}"
mkdir -p "$out/root"
body='{"events":[{"op":"image-add","id":"img-한","size":0.1},{"op":"image-delete","id":"img-한"}],"timestamp":1760000000000,"note":"😀"}'
checksum=$(printf '%s' "$body" | sha256sum | cut -d' ' -f1)
units=$(printf '%s' "$body" | iconv -f UTF-8 -t UTF-16LE | wc -c)
units=$((units/2))
{
 printf '{"type":"prepare","version":1,"tx":"synthetic","length":%s,"body":%s,"checksum":"%s"}\n' "$units" "$body" "$checksum"
 printf '{"type":"commit","version":1,"tx":"synthetic","length":%s,"checksum":"%s"}\n' "$units" "$checksum"
} > "$out/root/fixture.wal"
cp "$out/root/fixture.wal" "$out/fixture-original.wal"
printf 'FIXTURE_BODY_UTF16_LENGTH=%s\nFIXTURE_BODY_SHA256=%s\n' "$units" "$checksum"
l07_build_compiler "$out" > "$out/compiler-build.log" 2>&1
pass=0;fail=0
for spec in json-serialize json-parse json-invalid json-wal;do
 if l07_build_app "$out" "$ROOT/tests/l08-platform/$spec.fl" "$spec" > "$out/$spec.build.log" 2>&1;then
  iterations=(0);[[ "$spec" != json-wal ]] || iterations=({1..10})
  for iteration in "${iterations[@]}";do
   if (cd "$out/root";env FL_FILE_BASE="$out/root" timeout 30 "$out/$spec" "$out/root/fixture.wal") > "$out/$spec-$iteration.actual" 2> "$out/$spec-$iteration.stderr";then rc=0;else rc=$?;fi
   cat "$out/$spec-$iteration.actual" "$out/$spec-$iteration.stderr"
   printf '%s_%s_PROGRAM_EXIT=%s\n' "$spec" "$iteration" "$rc"
   if [[ "$rc" == 0 && ! -s "$out/$spec-$iteration.stderr" ]] && diff -u "$ROOT/tests/l08-platform/$spec.expected" "$out/$spec-$iteration.actual";then pass=$((pass+1));else fail=$((fail+1));fi
  done
 else rc=$?;cat "$out/$spec.build.log";printf '%s_BUILD_EXIT=%s\n' "$spec" "$rc";fail=$((fail+1));fi
done
if cmp -s "$out/fixture-original.wal" "$out/root/fixture.wal";then echo FIXTURE_BYTES_UNCHANGED=PASS;else echo FIXTURE_BYTES_UNCHANGED=FAIL;fail=$((fail+1));fi
printf 'L08_JSON_PASS=%s FAIL=%s\n' "$pass" "$fail"
[[ "$fail" == 0 ]]
