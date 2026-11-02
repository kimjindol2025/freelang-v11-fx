#!/usr/bin/env bash
# Sourced by the DB95 bootstrap. Static token audit is not an execution counter.
set -Eeuo pipefail
awk '
function flush(){
 if(token ~ /^(fl_system|fl_exec|fl_process_run|system|popen|execl|execlp|execle|execv|execvp|execve|execvpe|posix_spawn|posix_spawnp)$/){print FILENAME ":" FNR ":" token;bad++}
 token=""
}
FNR==1 {mode=0;escape=0;token=""}
{
 for(i=1;i<=length($0);i++){
  c=substr($0,i,1);nextc=substr($0,i+1,1)
  if(mode==3){if(c=="*"&&nextc=="/"){mode=0;i++}continue}
  if(mode==1||mode==2){if(escape)escape=0;else if(c=="\\")escape=1;else if((mode==1&&c=="\"")||(mode==2&&c==sprintf("%c",39)))mode=0;continue}
  if(c=="/"&&nextc=="/"){flush();break}
  if(c=="/"&&nextc=="*"){flush();mode=3;i++;continue}
  if(c=="\""||c==sprintf("%c",39)){flush();mode=c=="\""?1:2;continue}
  if(c ~ /[[:alnum:]_]/)token=token c;else flush()
 }
 flush()
}
END {if(mode!=0){print "STATIC_C_LEXER_UNTERMINATED=FAIL";exit 1}if(bad)exit 1}
' "$ROOT_DIR/bootstrap/stage0.c" "$TMP_DIR"/stage{1,2,3,4}.c > "$TMP_DIR/static-forbidden-tokens.txt"
{
 echo BOOTSTRAP_STATIC_FORBIDDEN_PROCESS_TOKENS=0
 echo BOOTSTRAP_STATIC_SCOPE=SEED_AND_GENERATED_COMPILER_TRANSLATION_UNITS
 echo BOOTSTRAP_DYNAMIC_EXEC_COUNTS=UNOBSERVED
 sha256sum "$ROOT_DIR/bootstrap/stage0.c" "$CGC_MAIN" "$ROOT_DIR/tests/native-bootstrap/array-loop.fl" "$ROOT_DIR/scripts/verify-db95-bootstrap-static.sh" "$ROOT_DIR/scripts/audit-db95-bootstrap-static.sh"
 echo BOOTSTRAP_STATIC_AUDIT=PASS
} > "$TMP_DIR/static-audit.txt"
cat "$TMP_DIR/static-audit.txt"
