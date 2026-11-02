#!/usr/bin/env bash
set -Eeuo pipefail
root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)
source "$root/tests/l07/build.sh"
compiler=${FX_NATIVE_COMPILER:?};work=${FX_PROCESS_EXIT_OUT:?};mkdir -p "$work"
"$compiler" "$root/tests/native-bootstrap/process-exit.fl" "$work/fl-exit.c"
gcc -O2 -w -Werror=implicit-function-declaration -I "$root/runtime" -o "$work/fl-exit" "$work/fl-exit.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}"
pass=0
run_exit_case(){
 local api=$1 code=$2 expected=$3 oracle=$4 rc
 if [[ $api == FL ]];then
  if DB95_PROCESS_EXIT_CODE="$code" "$work/fl-exit" > "$work/$api-$code.stdout" 2> "$work/$api-$code.stderr";then rc=0;else rc=$?;fi
 else
  if "$work/native-exit" "$code" > "$work/$api-$code.stdout" 2> "$work/$api-$code.stderr";then rc=0;else rc=$?;fi
 fi
 cat "$work/$api-$code.stdout" "$work/$api-$code.stderr"
 printf '\nPROCESS_EXIT_API=%s CODE=%s EXIT=%s EXPECTED=%s\n' "$api" "$code" "$rc" "$expected"
 [[ $rc == "$expected" && ! -s "$work/$api-$code.stderr" ]]
 printf '%b' "$oracle" > "$work/$api-$code.expected"
 diff -u "$work/$api-$code.expected" "$work/$api-$code.stdout"
 pass=$((pass+1))
}
while IFS=$'\t' read -r api code expected oracle;do
 [[ $api == FL ]] || continue
 run_exit_case "$api" "$code" "$expected" "$oracle"
done < "$root/tests/native-bootstrap/process-exit.expected.tsv"
gcc -O2 -w -Werror=implicit-function-declaration -I "$root/runtime" -o "$work/native-exit" "$root/tests/native-bootstrap/process-exit-probe.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}"
while IFS=$'\t' read -r api code expected oracle;do
 [[ $api == NATIVE ]] || continue
 run_exit_case "$api" "$code" "$expected" "$oracle"
done < "$root/tests/native-bootstrap/process-exit.expected.tsv"
printf 'PROCESS_EXIT_PASS=%s FAIL=0 TOTAL=6\n' "$pass"
[[ $pass == 6 ]]
