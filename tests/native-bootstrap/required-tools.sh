#!/usr/bin/env bash
set -Eeuo pipefail
root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)
out=$(mktemp -d)
trap 'rm -rf -- "$out"' EXIT
mkdir "$out/bin"
ln -s "$(command -v gcc)" "$out/bin/gcc"
if env PATH="$out/bin" "$BASH" "$root/scripts/verify-native-bootstrap.sh" > "$out/actual" 2>&1;then rc=0;else rc=$?;fi
if [[ "$rc" == 1 && "$(cat "$out/actual")" == REQUIRED_TOOL=FAIL:rg ]];then
 echo BOOTSTRAP_REQUIRED_TOOLS=PASS
else
 cat "$out/actual"
 printf 'BOOTSTRAP_REQUIRED_TOOLS=FAIL EXIT=%s\n' "$rc"
 exit 1
fi
