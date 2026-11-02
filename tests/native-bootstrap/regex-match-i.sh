#!/usr/bin/env bash
set -Eeuo pipefail
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
compiler=${FX_NATIVE_COMPILER:?}
work=${FX_REGEX_TEST_OUT:-$(mktemp -d)}
mkdir -p "$work"
sources=("$root"/runtime/{core,collection,io,math,error,process,json,aliases,user-fns,gc,http,websocket,sqlite,mariadb,debug,http_client,regex,smtp,sse}.c)
for spec in regex-match-i regex-match-i-invalid regex-match-i-input-type regex-match-i-pattern-type;do
 "$compiler" "$root/tests/native-bootstrap/$spec.fl" "$work/$spec.c"
 gcc -O2 -w -Werror=implicit-function-declaration -I "$root/runtime" -o "$work/$spec" "$work/$spec.c" "${sources[@]}" -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto -lcurl
done
"$work/regex-match-i" > "$work/values.stdout" 2> "$work/values.stderr"
cat "$work/values.stdout" "$work/values.stderr"
diff -u "$root/tests/native-bootstrap/regex-match-i.expected" "$work/values.stdout"
[[ ! -s "$work/values.stderr" ]]
for spec in invalid input-type pattern-type;do
 if "$work/regex-match-i-$spec" > "$work/$spec.log" 2>&1;then rc=0;else rc=$?;fi
 cat "$work/$spec.log"
 [[ $rc == 1 ]]
 if [[ $spec == invalid ]];then expected='regex_match_i: invalid pattern:';else expected='regex_match_i: string arguments required';fi
 grep -Fq "$expected" "$work/$spec.log"
 if grep -Fq WAS_SILENT "$work/$spec.log";then exit 1;fi
 printf 'REGEX_MATCH_I_ERROR_%s=PASS\n' "$spec"
done
printf 'REGEX_MATCH_I_ERRORS=3/3\n'
