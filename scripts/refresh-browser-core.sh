#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
temporary=$(mktemp)
trap 'rm -f "$temporary"' EXIT
{
 echo '/* Generated from self/browser-core.fl; do not edit. */'
 xxd -i -n fl_browser_core "$ROOT/self/browser-core.fl" | sed -e 's/^unsigned char /static const unsigned char /' -e 's/^unsigned int /static const unsigned int /'
 printf 'static const char fl_browser_core_sha256[] = "%s";\n' "$(sha256sum "$ROOT/self/browser-core.fl" | cut -d' ' -f1)"
} > "$temporary"
if [[ "${1:---check}" == --write ]];then
 cp "$temporary" "$ROOT/runtime/browser-core-data.h"
else
 diff -u "$ROOT/runtime/browser-core-data.h" "$temporary"
 echo BROWSER_CORE_EMBEDDING=PASS
fi
