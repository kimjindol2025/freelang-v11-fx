#!/usr/bin/env bash
set -Eeuo pipefail
umask 077
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_REPAIR_IO_OUT:-$(mktemp -d)}"
mkdir -p "$out"
compiler="${L08_REPAIR_IO_COMPILER:-$out/compiler}"
if [[ -z "${L08_REPAIR_IO_COMPILER:-}" ]];then l07_build_compiler "$out" > "$out/compiler-build.log" 2>&1;fi
flags=(-O1 -g -w -D_GNU_SOURCE -I "$ROOT/runtime" -include "$ROOT/tests/l08-platform/repair-io.h")
if [[ -n "${L08_REPAIR_IO_SANITIZER_LIBDIR:-}" ]];then
 flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer -L "$L08_REPAIR_IO_SANITIZER_LIBDIR")
 export LD_LIBRARY_PATH="$L08_REPAIR_IO_SANITIZER_LIBDIR" ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1
fi
extra=(--extern repair-test-setup --extern repair-test-byte --extern repair-test-fds --extern repair-test-mode --extern repair-test-umask)
failed=0
for spec in repair-bytes repair-fd;do
 mkdir -p "$out/$spec/root" "$out/$spec/outside"
 "$compiler" "${extra[@]}" "$ROOT/tests/l08-platform/$spec.fl" "$out/$spec/main.c" > "$out/$spec/build.log" 2>&1
 gcc "${flags[@]}" -o "$out/$spec/main" "$out/$spec/main.c" "$ROOT/tests/l08-platform/repair-io.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}" >> "$out/$spec/build.log" 2>&1
 if (cd "$out/$spec/root";env FL_FILE_BASE="$out/$spec/root" L08_REPAIR_OUTSIDE="$out/$spec/outside" timeout 60 "$out/$spec/main") > "$out/$spec/actual" 2> "$out/$spec/stderr";then rc=0;else rc=$?;fi
 cat "$out/$spec/actual" "$out/$spec/stderr"
 printf 'REPAIR_IO_%s_EXIT=%s\n' "$spec" "$rc"
 if [[ "$rc" != 0 || -s "$out/$spec/stderr" ]];then failed=$((failed+1));continue;fi
 if ! diff -u "$ROOT/tests/l08-platform/$spec.expected" "$out/$spec/actual";then failed=$((failed+1));fi
 [[ $(cat "$out/$spec/outside/sentinel") == outside && ! -e "$out/$spec/outside/new" && ! -e "$out/$spec/escape" ]] || failed=$((failed+1))
done
printf 'REPAIR_IO_GROUP_PASS=%s FAIL=%s\n' "$((2-failed))" "$failed"
[[ "$failed" == 0 ]]
