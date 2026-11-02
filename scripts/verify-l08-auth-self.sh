#!/usr/bin/env bash
set -Eeuo pipefail
umask 077
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_AUTH_OUT:-$(mktemp -d)}"
mkdir -p "$out/root"
gcc -O2 -Wall -Wextra "$ROOT/tests/l08-platform/auth-fixtures.c" -o "$out/fixtures" -lcrypto
gcc -O2 -Wall -Wextra -fPIC -shared "$ROOT/tests/l08-platform/auth-interpose.c" -o "$out/interpose.so" -ldl
"$out/fixtures" create "$out/root"
l07_build_compiler "$out" > "$out/compiler-build.log" 2>&1
pass=0;fail=0
for spec in auth-basics auth-verify auth-csrf auth-live auth-rng;do
 if l07_build_app "$out" "$ROOT/tests/l08-platform/$spec.fl" "$spec" > "$out/$spec.build.log" 2>&1;then
  envs=(FL_FILE_BASE="$out/root")
  [[ "$spec" != auth-csrf ]] || envs+=(LD_PRELOAD="$out/interpose.so" L08_AUTH_FIXED_CLOCK=1)
  [[ "$spec" != auth-rng ]] || envs+=(LD_PRELOAD="$out/interpose.so" L08_AUTH_RANDOM_FAIL=1)
  : > "$out/$spec.expected-stderr"
  [[ "$spec" != auth-rng ]] || printf 'AUTH_RNG_FAULT=ACTIVE\nAUTH_RNG_FAULT=ACTIVE\n' > "$out/$spec.expected-stderr"
  if (cd "$out/root";timeout 30 env "${envs[@]}" "$out/$spec" "$out/root") > "$out/$spec.actual" 2> "$out/$spec.stderr";then rc=0;else rc=$?;fi
  cat "$out/$spec.actual" "$out/$spec.stderr"
  printf '%s_PROGRAM_EXIT=%s\n' "$spec" "$rc"
  if [[ "$rc" == 0 ]] && diff -u "$ROOT/tests/l08-platform/$spec.expected" "$out/$spec.actual" && diff -u "$out/$spec.expected-stderr" "$out/$spec.stderr";then pass=$((pass+1));else fail=$((fail+1));fi
 else rc=$?;cat "$out/$spec.build.log";printf '%s_BUILD_EXIT=%s\n' "$spec" "$rc";fail=$((fail+1));fi
done
if "$out/fixtures" validate "$out/root";then pass=$((pass+1));else fail=$((fail+1));fi
[[ "$fail" != 0 ]] || echo AUTH_PASSWORD_VERIFICATION=PASS
printf 'L08_AUTH_SELF_PASS=%s FAIL=%s\n' "$pass" "$fail"
[[ "$fail" == 0 ]]
