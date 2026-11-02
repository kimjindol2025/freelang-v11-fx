#!/usr/bin/env bash
# Diagnose the full bootstrap equality failure using unchanged complete source forms.
set -Eeuo pipefail
umask 077
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
stage1="${L08_BOOTSTRAP_STAGE1:?}"
stage2="${L08_BOOTSTRAP_STAGE2:?}"
out="${L08_BOOTSTRAP_FORMS_OUT:-$(mktemp -d)}"
mkdir -p "$out/forms" "$out/first" "$out/second"
# This source keeps each top-level form at column zero and nested forms indented.
awk -v dir="$out/forms" '/^\(/{if(file)close(file);file=sprintf("%s/%04d.fl",dir,++count);print count,NR,$0 > (dir "/index.txt")}file{print > file}' "$ROOT/self/cgc-main.fl"
passed=0
for file in "$out"/forms/*.fl;do
 name=${file##*/};name=${name%.fl}
 "$stage1" "$file" "$out/first/$name.c" > "$out/first/$name.stdout" 2> "$out/first/$name.stderr"
 "$stage2" "$file" "$out/second/$name.c" > "$out/second/$name.stdout" 2> "$out/second/$name.stderr"
 if ! cmp -s "$out/first/$name.c" "$out/second/$name.c";then
  printf 'BOOTSTRAP_FORM_DIFFERENT=%s\n' "$name"
  cat "$file"
  diff -u "$out/first/$name.c" "$out/second/$name.c" > "$out/difference.txt" || true
  printf 'BOOTSTRAP_FORM_PASS=%s FAIL=1\n' "$passed"
  exit 1
 fi
 passed=$((passed+1))
done
printf 'BOOTSTRAP_FORM_PASS=%s FAIL=0\n' "$passed"
echo BOOTSTRAP_FORM_OUTPUTS=PASS
