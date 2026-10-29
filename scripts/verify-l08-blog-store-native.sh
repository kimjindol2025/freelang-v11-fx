#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_BLOG_STORE_OUT:?}"
blog="${L08_BLOG_SOURCE_ROOT:?source-only approved copy required}"
compiler="${L08_BLOG_NATIVE_COMPILER:?}"
flags=(-O2 -w -I "$ROOT/runtime")
runtime_env=()
if [[ -n ${L08_BLOG_STORE_SANITIZER_LIBDIR:-} ]];then
 flags=(-O1 -g -w -fsanitize=address,undefined -fno-omit-frame-pointer -I "$ROOT/runtime" -L "$L08_BLOG_STORE_SANITIZER_LIBDIR")
 runtime_env=("LD_LIBRARY_PATH=$L08_BLOG_STORE_SANITIZER_LIBDIR" ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1)
fi
[[ -f "$blog/tests/stage1-write.fl" && -f "$blog/tests/stage1-verify.fl" && ! -e "$blog/data/blog.wal" ]]
mkdir -p "$out/root"
for name in stage1-write stage1-verify;do
 (cd "$blog/tests";"$compiler" "$name.fl" "$out/$name.c") > "$out/$name-build.log" 2>&1
 gcc "${flags[@]}" -o "$out/$name" "$out/$name.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}" >> "$out/$name-build.log" 2>&1
done
run(){
 local name="$1" label="$2" rc
 if (cd "$out/root";env -i PATH=/usr/bin:/bin LANG=C.UTF-8 TZ=UTC "${runtime_env[@]}" FL_FILE_BASE="$out/root" AFJ_DB_MODE=development AFJ_DB_BIND_HOST=127.0.0.1 AFJ_DB_WAL="$out/root/store.wal" timeout 30 "$out/$name") > "$out/$label.actual" 2> "$out/$label.stderr";then rc=0;else rc=$?;fi
 cat "$out/$label.actual" "$out/$label.stderr"
 printf '%s_NATIVE_EXIT=%s\n' "$label" "$rc"
 [[ "$rc" == 0 && ! -s "$out/$label.stderr" ]]
 local expected="$ROOT/tests/l08-platform/blog-$name.expected" lines
 lines=$(wc -l < "$expected")
 tail -n "$lines" "$out/$label.actual" > "$out/$label.result"
 head -n "-$lines" "$out/$label.actual" > "$out/$label.prelude"
 diff -u "$expected" "$out/$label.result"
 [[ -s "$out/$label.prelude" ]]
 # Original blog runner permits the loaded db95 modules' self-test output.
 if rg -n -v '^(afj-db95 .+ PASS|PASS SQL .+)$' "$out/$label.prelude";then return 1;fi
 if [[ "$label" != writer ]];then cmp "$out/writer.prelude" "$out/$label.prelude";fi
}
run stage1-write writer
cp "$out/root/store.wal" "$out/original-fixture.wal"
for iteration in {1..10};do run stage1-verify "reader-$iteration";done
cmp "$out/root/store.wal" "$out/original-fixture.wal"
echo DB95_NATIVE_WAL_READ_ONLY=PASS
printf 'DB95_NATIVE_WAL_SHA256=%s\n' "$(sha256sum "$out/root/store.wal" | cut -d' ' -f1)"
echo 'DB95_NATIVE_STORE_PASS=11 FAIL=0'
