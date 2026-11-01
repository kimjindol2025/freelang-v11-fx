#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_EXTERN_OUT:-$(mktemp -d)}"
mkdir -p "$out"
if [[ -n "${L08_EXTERN_COMPILER:-}" ]];then
 compiler="$L08_EXTERN_COMPILER"
else
 l07_build_compiler "$out" > "$out/compiler-build.log" 2>&1
 compiler="$out/compiler"
fi
src="$ROOT/tests/l08-platform/extern-c.fl"
build_app() {
 local name="$1" generated="$2"; shift 2
 gcc -O1 -g -w -D_GNU_SOURCE -I "$ROOT/runtime" \
  -include "$ROOT/tests/l08-platform/extern-c.h" "$@" -o "$out/$name" \
  "$generated" "$ROOT/tests/l08-platform/extern-c.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}"
}
expect_missing() {
 local phase="$1" rc=0
 "$compiler" "$src" "$out/$phase.c" > "$out/$phase.compile.log" 2>&1
 build_app "$phase" "$out/$phase.c" > "$out/$phase.build.log" 2>&1
 "$out/$phase" > "$out/$phase.stdout" 2> "$out/$phase.stderr" || rc=$?
 cat "$out/$phase.stderr"
 printf '%s_EXIT=%s\n' "$phase" "$rc"
 [[ "$rc" == 1 ]]
 rg -q 'Function not found: native-ext-value' "$out/$phase.stderr"
}
expect_missing no-declaration-before
"$compiler" --extern native-ext-value --extern native-ext-zero "$src" "$out/declared.c" > "$out/declared.compile.log" 2>&1
build_app declared "$out/declared.c" > "$out/declared.build.log" 2>&1
"$out/declared" > "$out/declared.stdout" 2> "$out/declared.stderr"
cat "$out/declared.stdout"
diff -u "$ROOT/tests/l08-platform/extern-c.expected" "$out/declared.stdout"
[[ ! -s "$out/declared.stderr" ]]
echo EXTERN_DECLARED_EXIT=0
expect_missing no-declaration-after
for case_name in missing invalid;do
 args=(--extern)
 [[ "$case_name" != invalid ]] || args+=(bad/name "$src" "$out/invalid.c")
 rc=0
 "$compiler" "${args[@]}" > "$out/$case_name.stdout" 2> "$out/$case_name.stderr" || rc=$?
 cat "$out/$case_name.stderr"
 printf 'EXTERN_%s_NAME_EXIT=%s\n' "$case_name" "$rc"
 [[ "$rc" == 1 ]]
 rg -q "compiler: $case_name --extern name" "$out/$case_name.stderr"
 [[ ! -e "$out/invalid.c" ]]
done
rc=0
gcc -O1 -w -D_GNU_SOURCE -I "$ROOT/runtime" \
 -include "$ROOT/tests/l08-platform/extern-c.h" -o "$out/unlinked" \
 "$out/declared.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}" > "$out/unlinked.log" 2>&1 || rc=$?
printf 'EXTERN_MISSING_IMPLEMENTATION_LINK_EXIT=%s\n' "$rc"
[[ "$rc" != 0 ]]
rg -q 'undefined reference.*native_ext_' "$out/unlinked.log"
echo EXTERN_MISSING_IMPLEMENTATION=PASS
"$compiler" --ast "$src" > "$out/ast.stdout" 2> "$out/ast.stderr"
rg -q '^AST_JSON:' "$out/ast.stdout"
[[ ! -s "$out/ast.stderr" ]]
echo EXTERN_AST_COMPATIBILITY=PASS
if [[ "${L08_EXTERN_SANITIZE:-0}" == 1 ]];then
 san_flags=(-fsanitize=address,undefined -fno-omit-frame-pointer)
 if [[ -n "${L08_EXTERN_SANITIZER_LIBDIR:-}" ]];then
  san_flags+=(-L "$L08_EXTERN_SANITIZER_LIBDIR")
  export LD_LIBRARY_PATH="$L08_EXTERN_SANITIZER_LIBDIR${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
 fi
 build_app declared-sanitized "$out/declared.c" "${san_flags[@]}" > "$out/sanitized.build.log" 2>&1
 ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 "$out/declared-sanitized" > "$out/sanitized.stdout" 2> "$out/sanitized.stderr"
 diff -u "$ROOT/tests/l08-platform/extern-c.expected" "$out/sanitized.stdout"
 [[ ! -s "$out/sanitized.stderr" ]]
 echo EXTERN_SANITIZER_EXIT=0
fi
echo L08_EXTERN_SELF=PASS
