#!/usr/bin/env bash
# D3: reject generated C that calls undeclared symbols. No -w reliance.
# Usage: check-generated-abi.sh <generated.c> [runtime_dir]
set -Eeuo pipefail
gen="${1:?generated C file}"
runtime_dir="${2:-$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../runtime" && pwd -P)}"
[[ -f "$gen" ]] || { echo "ABI_INPUT=FAIL:$gen" >&2; exit 2; }

python3 - "$gen" "$runtime_dir" <<'PY'
import re,sys,glob,os
gen,rt=sys.argv[1],sys.argv[2]
src=open(gen,errors='replace').read()
headers=''.join(open(p,errors='replace').read() for p in glob.glob(rt+'/*.h'))
keywords={'if','while','for','switch','return','sizeof','do','else','case','catch','typeof','__extension__','defined'}
# C library / compiler builtins that need no project header
builtins={'malloc','calloc','realloc','free','memcpy','memset','memmove','strlen','strcmp','strncmp','strcpy',
 'strtoll','strtod','snprintf','sprintf','printf','fprintf','fopen','fclose','fread','fwrite','exit','abort',
 'qsort','abs','labs','pow','fabs','floor','ceil','sqrt','time','rand','setjmp','longjmp','va_start','va_arg','va_end'}
called=set(re.findall(r'\b([A-Za-z_][A-Za-z0-9_]*)\s*\(',src))
defined=set(re.findall(r'\b(?:FLValue|void|int|static\s+FLValue|static\s+int|double|char\*?)\s+([A-Za-z_][A-Za-z0-9_]*)\s*\([^;]*\)\s*\{',src))
known_decl=set()
for pat in [r'\b(?:FLValue|void|int|double|char|long|size_t|FLClosure\*?)\s+([A-Za-z_][A-Za-z0-9_]*)\s*\(',
            r'#define\s+([A-Za-z_][A-Za-z0-9_]*)\s*[\(\ ]']:
    known_decl.update(re.findall(pat,headers))
unknown=sorted(called-keywords-builtins-defined-known_decl)
# filter obvious control/statement macros still left
unknown=[u for u in unknown if not u.startswith('__')]
if unknown:
    print('ABI_UNDECLARED_COUNT=%d' % len(unknown))
    for u in unknown[:80]: print('ABI_UNDECLARED='+u)
    sys.exit(1)
print('ABI_GENERATED_SYMBOLS=PASS calls=%d' % len(called))
PY
