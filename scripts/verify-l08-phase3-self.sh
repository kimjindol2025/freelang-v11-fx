#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_P3_OUT:-$(mktemp -d)}"
mkdir -p "$out/source" "$out/root" "$out/outside"
cp "$ROOT/tests/l08-phase3/"*.fl "$out/source/"
l07_build_compiler "$out" > "$out/build.log" 2>&1
gcc -O2 -Wall -Wextra "$ROOT/tests/l08-phase3/fixtures.c" -o "$out/fixtures"
"$out/fixtures" "$out/root"
printf KEEP > "$out/outside/sentinel"
printf PRIVATE > "$out/root/unreadable"; chmod 000 "$out/root/unreadable"
printf READONLY > "$out/root/readonly"; chmod 400 "$out/root/readonly"
ln -s "$out/outside/sentinel" "$out/root/link-file"
ln -s "$out/outside" "$out/root/link-dir"
pass=0;fail=0
for spec in dates timezone regex-patterns regex files images security;do
 if l07_build_app "$out" "$out/source/$spec.fl" "$spec" > "$out/$spec.build.log" 2>&1;then
  variants=(default); [[ "$spec" != timezone ]] || variants=(utc seoul newyork)
  for variant in "${variants[@]}";do
   zone=UTC; [[ "$variant" != seoul ]] || zone=Asia/Seoul; [[ "$variant" != newyork ]] || zone=America/New_York
   expected="$ROOT/tests/l08-phase3/$spec.expected"; [[ "$spec" != timezone ]] || expected="$ROOT/tests/l08-phase3/timezone-$variant.expected"
   if (cd "$out/root"; env TZ="$zone" FL_FILE_BASE="$out/root" "$out/$spec" "$out/root" "$out/outside") > "$out/$spec-$variant.actual" 2> "$out/$spec-$variant.stderr";then rc=0;else rc=$?;fi
   cat "$out/$spec-$variant.actual" "$out/$spec-$variant.stderr";printf '%s-%s_PROGRAM_EXIT=%s\n' "$spec" "$variant" "$rc"
   if [[ "$rc" == 0 ]] && diff -u "$expected" "$out/$spec-$variant.actual";then echo "PASS $spec-$variant";pass=$((pass+1));else echo "FAIL $spec-$variant";fail=$((fail+1));fi
  done
 else rc=$?;cat "$out/$spec.build.log";printf '%s_BUILD_EXIT=%s\n' "$spec" "$rc";fail=$((fail+1));fi
done
for ext in png jpg gif;do
 if cmp -s "$out/root/image.$ext" "$out/root/copies/image.$ext" && cmp -s "$out/root/image.$ext" "$out/root/raw-$ext";then echo "IMAGE_BYTES_$ext=PASS";else echo "IMAGE_BYTES_$ext=FAIL";fail=$((fail+1));fi
done
if [[ $(cat "$out/outside/sentinel") == KEEP ]] && [[ ! -e "$out/outside/created" ]];then echo OUTSIDE_UNCHANGED=PASS;else echo OUTSIDE_UNCHANGED=FAIL;fail=$((fail+1));fi
printf 'L08_P3_PASS=%s FAIL=%s\n' "$pass" "$fail"
[[ "$fail" == 0 ]]
