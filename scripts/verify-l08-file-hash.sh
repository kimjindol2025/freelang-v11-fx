#!/usr/bin/env bash
set -Eeuo pipefail
umask 077
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_HASH_OUT:-$(mktemp -d)}"
compiler="${L08_HASH_COMPILER:?native compiler required}"
mkdir -p "$out/root" "$out/outside"
flags=(-O1 -g -w -D_GNU_SOURCE -I "$ROOT/runtime" -include "$ROOT/tests/l08-platform/file-hash.h")
if [[ -n "${L08_HASH_SANITIZER_LIBDIR:-}" ]];then
 flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer -L "$L08_HASH_SANITIZER_LIBDIR")
 export LD_LIBRARY_PATH="$L08_HASH_SANITIZER_LIBDIR" ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1
fi
"$compiler" --extern hash-test-arm --extern hash-test-fds --extern hash-test-rss "$ROOT/tests/l08-platform/file-hash.fl" "$out/main.c" > "$out/build.log" 2>&1
gcc "${flags[@]}" -o "$out/main" "$out/main.c" "$ROOT/tests/l08-platform/file-hash.c" -Wl,--wrap=read,--wrap=close "${L07_SRCS[@]}" "${L07_LIBS[@]}" >> "$out/build.log" 2>&1
: > "$out/root/empty"
for ((i=0;i<256;i++));do printf -v oct '%03o' "$i";printf '%b' "\\$oct";done > "$out/root/all-bytes"
printf '한🙂\0\xff' > "$out/root/utf8"
printf 'all\0bytes' > "$out/root/nul-path"
truncate -s 67108864 "$out/root/large"
truncate -s 67108865 "$out/root/too-large"
printf outside > "$out/outside/sentinel"
ln -s "$out/outside/sentinel" "$out/root/link"
ln -s "$out/outside" "$out/root/parent-link"
mkfifo "$out/root/pipe"
export L08_HASH_ALL_SHA="$(sha256sum "$out/root/all-bytes" | cut -d' ' -f1)"
export L08_HASH_UTF8_SHA="$(sha256sum "$out/root/utf8" | cut -d' ' -f1)"
export L08_HASH_LARGE_SHA="$(sha256sum "$out/root/large" | cut -d' ' -f1)"
export FL_FILE_BASE="$out/root" L08_HASH_TARGET="$out/root/hash-target"
passed=0;failed=0
for mode in normal eintr short read close grow rewrite truncate;do
 cp "$out/root/all-bytes" "$out/root/hash-target"
 if (cd "$out/root";env L08_HASH_CASE="$mode" timeout 60 "$out/main") > "$out/$mode.actual" 2> "$out/$mode.stderr";then rc=0;else rc=$?;fi
 cat "$out/$mode.actual" "$out/$mode.stderr"
 printf 'FILE_HASH_CASE=%s EXIT=%s\n' "$mode" "$rc"
 if [[ "$mode" == normal ]];then cp "$ROOT/tests/l08-platform/file-hash.expected" "$out/$mode.expected"
 else printf 'FILE_HASH_%s=PASS\nFILE_HASH_NO_FD_LEAK=PASS\n' "$mode" > "$out/$mode.expected";fi
 if [[ "$rc" == 0 && ! -s "$out/$mode.stderr" ]] && diff -u "$out/$mode.expected" "$out/$mode.actual";then passed=$((passed+1));else failed=$((failed+1));fi
done
[[ $(cat "$out/outside/sentinel") == outside ]]
printf 'FILE_HASH_GROUP_PASS=%s FAIL=%s\n' "$passed" "$failed"
[[ "$failed" == 0 ]]
