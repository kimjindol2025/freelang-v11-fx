#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_CLOCK_OUT:-$(mktemp -d)}"
mkdir -p "$out"
producer="${L08_NATIVE_COMPILER:-}"
if [[ -z "$producer" ]];then l07_build_compiler "$out" > "$out/build.log" 2>&1;producer="$out/compiler";fi
[[ $(head -c4 "$producer" | od -An -tx1 | tr -d ' \n') == 7f454c46 ]]
"$producer" "$ROOT/tests/l08-phase3/clock.fl" "$out/clock.c"
gcc -O2 -w -I "$ROOT/runtime" -o "$out/clock" "$out/clock.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}"
for zone in UTC Asia/Seoul America/New_York;do
 lower=$(date +%s%3N)
 TZ="$zone" "$out/clock" "$lower" "$((lower+10000))" > "$out/sample"
 upper=$(date +%s%3N)
 current=$(head -n1 "$out/sample")
 [[ "$current" =~ ^[0-9]+$ ]]
 (( current >= lower && current <= upper ))
 tail -n1 "$out/sample" > "$out/result"
 diff -u "$ROOT/tests/l08-phase3/clock.expected" "$out/result"
 printf 'TZ=%s HOST_LOWER_MS=%s FREELANG_NOW_MS=%s HOST_UPPER_MS=%s\n' "$zone" "$lower" "$current" "$upper"
done
echo CLOCK_RANGE=PASS
