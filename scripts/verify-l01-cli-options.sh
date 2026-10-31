#!/usr/bin/env bash
set -Eeuo pipefail
umask 077
compiler="${L01_NATIVE_COMPILER:?native compiler required}"
out="${L01_CLI_OUT:-$(mktemp -d)}"
mkdir -p "$out"
passed=0;failed=0
for option in --definitely-invalid --ast-typo --help-typo;do
 root="$out/${option#--}";mkdir -p "$root"
 printf '(println "CLI_NATIVE_OK")\n' > "$root/input.fl"
 before=$(sha256sum "$root/input.fl" | cut -d' ' -f1)
 if (cd "$root";"$compiler" "$option" input.fl output.c) > "$root/actual" 2> "$root/stderr";then rc=0;else rc=$?;fi
 after=$(sha256sum "$root/input.fl" | cut -d' ' -f1)
 cat "$root/actual" "$root/stderr"
 printf 'CLI_OPTION=%s EXIT=%s\n' "$option" "$rc"
 if [[ "$rc" != 0 && "$before" == "$after" && ! -e "$root/output.c" ]];then
  echo CLI_UNKNOWN_OPTION_PRESERVES_INPUT=PASS;passed=$((passed+1))
 else echo CLI_UNKNOWN_OPTION_PRESERVES_INPUT=FAIL;failed=$((failed+1));fi
done
printf '(println "CLI_NATIVE_OK")\n' > "$out/ast-input.fl"
before=$(sha256sum "$out/ast-input.fl" | cut -d' ' -f1)
if "$compiler" --ast "$out/ast-input.fl" > "$out/ast.actual" 2> "$out/ast.stderr" && [[ ! -s "$out/ast.stderr" ]] && rg -q '^AST_JSON:' "$out/ast.actual" && [[ "$before" == "$(sha256sum "$out/ast-input.fl" | cut -d' ' -f1)" ]];then
 echo CLI_AST_PRESERVES_INPUT=PASS;passed=$((passed+1))
else echo CLI_AST_PRESERVES_INPUT=FAIL;failed=$((failed+1));fi
printf 'CLI_OPTION_GROUP_PASS=%s FAIL=%s\n' "$passed" "$failed"
[[ "$failed" == 0 ]]
