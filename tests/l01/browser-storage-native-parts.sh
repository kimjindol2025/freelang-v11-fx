#!/usr/bin/env bash
# Supplemental fixture checks; not a canonical bootstrap or full-suite result.
set -Eeuo pipefail
umask 077
ulimit -c 0
ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)
source "$ROOT/tests/l07/build.sh"
compiler=${B8_STORAGE_COMPILER:?native compiler required}
out=${B8_STORAGE_NATIVE_OUT:?fresh task output required}
sanitizers=${B8_STORAGE_SANITIZER_LIBDIR:?existing sanitizer library directory required}
[[ ! -e "$out" ]]
mkdir -p "$out/cli" "$out/json/root" "$out/reject"
export LD_LIBRARY_PATH="$sanitizers${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export ASAN_OPTIONS=detect_leaks=1:halt_on_error=1
export UBSAN_OPTIONS=halt_on_error=1 LSAN_OPTIONS=exitcode=23

# Same four fixtures, exits, deadlines and oracles as verify-cli-lifetime.sh.
for name in atom-snapshot normal uncaught exit; do
  expected_rc=0
  case "$name" in
    atom-snapshot) fixture="$ROOT/tests/l08-platform/atom-snapshot.fl"; expected="$ROOT/tests/l08-platform/atom-snapshot.expected";;
    exit) fixture="$ROOT/tests/l08-platform/timer-exit.fl"; expected="$ROOT/tests/cli-lifetime/exit.expected"; expected_rc=7;;
    *) fixture="$ROOT/tests/cli-lifetime/$name.fl"; expected="$ROOT/tests/cli-lifetime/$name.expected"; [[ "$name" != uncaught ]] || expected_rc=1;;
  esac
  "$compiler" "$fixture" "$out/cli/$name.c" > "$out/cli/$name.compile.log" 2>&1
  strace -f -e trace=execve -o "$out/cli/$name.build.execve.log" gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -L "$sanitizers" -I "$ROOT/runtime" -o "$out/cli/$name" "$out/cli/$name.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}" > "$out/cli/$name.build.log" 2>&1
  if timeout 90 "$out/cli/$name" '한😀' > "$out/cli/$name.actual" 2> "$out/cli/$name.stderr"; then rc=0; else rc=$?; fi
  [[ "$rc" == "$expected_rc" ]]
  cmp "$expected" "$out/cli/$name.actual"
  if [[ "$name" == uncaught ]]; then cmp "$ROOT/tests/cli-lifetime/uncaught.stderr.expected" "$out/cli/$name.stderr"; else [[ ! -s "$out/cli/$name.stderr" ]]; fi
  printf 'CLI_FIXTURE_%s=PASS\n' "$name"
done
printf 'CLI_FIXTURES=4/4\n'

# Audit only the recorded build execve logs, using Bash rather than awk.
node=0; npm=0; npx=0; js=0; python=0
exec_pattern='execve\("([^"]+)"'
python_pattern='^(python[0-9.]*|pypy[0-9.]*)$|\.py$'
js_pattern='^(node|nodejs|js|qjs|jsc|d8|deno|bun|ts-node|tsx)$|\.[cm]?js$'
for log in "$out/cli/"*.build.execve.log; do
  while IFS= read -r line; do
    [[ "$line" =~ $exec_pattern ]] || continue
    program=${BASH_REMATCH[1]##*/}
    case "$program" in node|nodejs) node=$((node+1));; npm) npm=$((npm+1));; npx) npx=$((npx+1));; esac
    if [[ "$program" =~ $python_pattern ]]; then python=$((python+1)); fi
    if [[ "$program" =~ $js_pattern ]]; then js=$((js+1)); fi
  done < "$log"
done
printf 'BUILD_TRACE_NODE=%s NPM=%s NPX=%s JS=%s PYTHON=%s\n' "$node" "$npm" "$npx" "$js" "$python"
[[ $((node+npm+npx+js+python)) == 0 ]]

"$compiler" "$ROOT/tests/l01/browser-native.fl" "$out/native.c" > "$out/native.compile.log" 2>&1
gcc -O1 -g -w -I "$ROOT/runtime" -o "$out/native" "$out/native.c" "${L07_SRCS[@]}" "${L07_LIBS[@]}" > "$out/native.build.log" 2>&1
"$out/native" > "$out/native.actual" 2> "$out/native.stderr"
[[ ! -s "$out/native.stderr" ]]
cmp "$ROOT/tests/l01/browser-semantics.expected" "$out/native.actual"
printf 'NATIVE_SEMANTICS=37/37\n'

# Preserve all existing compiler rejection inputs and path rejection checks.
cp "$ROOT/tests/l01/"reject-*.fl "$out/reject/"
cp "$ROOT/tests/l01/browser-entry.fl" "$out/reject/"
for spec in private private-alias private-definition native eval parameters bindings recur extension same-source; do
  input="$out/reject/reject-$spec.fl"; output="$out/reject/$spec.js"
  case "$spec" in
    extension) input="$out/reject/browser-entry.fl"; output="$out/reject/invalid.txt";;
    same-source) input="$out/reject/browser-entry.fl"; output="$out/reject/alias.js"; ln -s "$input" "$output";;
  esac
  before=$(sha256sum "$input" | cut -d' ' -f1)
  if "$compiler" --browser-js "$input" "$output" > "$out/reject/$spec.stdout" 2> "$out/reject/$spec.stderr"; then rc=0; else rc=$?; fi
  [[ "$rc" == 1 && "$before" == "$(sha256sum "$input" | cut -d' ' -f1)" ]]
  if [[ "$spec" == same-source ]]; then [[ -L "$output" ]]; else [[ ! -e "$output" ]]; fi
done
printf 'COMPILER_REJECTIONS=10/10\n'

# Same JSON inputs, original expected files, ten WAL replays and 30s deadlines.
body='{"events":[{"op":"image-add","id":"img-한","size":0.1},{"op":"image-delete","id":"img-한"}],"timestamp":1760000000000,"note":"😀"}'
checksum=$(printf '%s' "$body" | sha256sum | cut -d' ' -f1)
units=$(printf '%s' "$body" | iconv -f UTF-8 -t UTF-16LE | wc -c); units=$((units/2))
{
  printf '{"type":"prepare","version":1,"tx":"synthetic","length":%s,"body":%s,"checksum":"%s"}\n' "$units" "$body" "$checksum"
  printf '{"type":"commit","version":1,"tx":"synthetic","length":%s,"checksum":"%s"}\n' "$units" "$checksum"
} > "$out/json/root/fixture.wal"
cp "$out/json/root/fixture.wal" "$out/json/fixture-original.wal"
ln -s "$compiler" "$out/json/compiler"
passed=0
for spec in json-serialize json-parse json-invalid json-wal; do
  l07_build_app "$out/json" "$ROOT/tests/l08-platform/$spec.fl" "$spec" > "$out/json/$spec.build.log" 2>&1
  iterations=(0); [[ "$spec" != json-wal ]] || iterations=({1..10})
  for iteration in "${iterations[@]}"; do
    (cd "$out/json/root"; FL_FILE_BASE="$out/json/root" timeout 30 "$out/json/$spec" "$out/json/root/fixture.wal") > "$out/json/$spec-$iteration.actual" 2> "$out/json/$spec-$iteration.stderr"
    [[ ! -s "$out/json/$spec-$iteration.stderr" ]]
    cmp "$ROOT/tests/l08-platform/$spec.expected" "$out/json/$spec-$iteration.actual"
    passed=$((passed+1))
  done
done
cmp "$out/json/fixture-original.wal" "$out/json/root/fixture.wal"
printf 'NATIVE_JSON=%s/13\n' "$passed"
printf 'NATIVE_PARTS_ONLY=PASS FULL_REGRESSION=UNEXECUTED\n'
