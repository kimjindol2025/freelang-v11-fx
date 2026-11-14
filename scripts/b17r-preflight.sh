#!/usr/bin/env bash
# Preparation only: no product request, build, or acceptance assertion runs here.
set -Eeuo pipefail
umask 077
task=${B17R_TASK_ROOT:?}
out=${1:?result directory required}
actor=${2:?native, authenticated, anonymous or mixed required}
fail(){ printf 'B17R_PREFLIGHT=FAIL REASON=%s\n' "$1";exit 1; }
for command in bash gcc rg jq sha256sum cmp diff ps flock curl openssl ss google-chrome strace unshare date realpath mktemp;do
 command -v "$command" >/dev/null 2>&1 || fail "TOOL_$command"
done
[[ -d "$task" && -d "${TMPDIR:?}" ]] || fail TASK_OR_TMPDIR
root=$(realpath "$task")
resolved=$(realpath -m "$out")
case "$resolved" in "$root"/*) ;; *) fail RESULT_OUTSIDE_TASK;;esac
mkdir -p "$resolved"
[[ -d "$resolved" && -w "$resolved" ]] || fail RESULT_NOT_WRITABLE
probe=$(mktemp "$resolved/.b17r-preflight.XXXXXX")
printf 'write-check\n' > "$probe"
[[ -s "$probe" ]] || fail RESULT_WRITE
rm -- "$probe"
plan=${B17R_REQUEST_PLAN:?}
[[ -r "$plan" ]] || fail REQUEST_PLAN_MISSING
rows=0
while IFS=$'\t' read -r label role audience;do
 [[ -n "$label" ]] || continue
 case "$role:$audience" in authenticated:admin|anonymous:public|native:none) ;; *) fail REQUEST_PLAN_IDENTITY;;esac
 rows=$((rows+1))
done < "$plan"
((rows>0)) || fail REQUEST_PLAN_EMPTY
case "$actor" in
 native|anonymous) ;;
 authenticated|mixed) cookie=${B17R_SYNTHETIC_COOKIE:-}; [[ ${#cookie} -ge 32 ]] || fail SYNTHETIC_COOKIE_MISSING;;
 *) fail ACTOR_UNDECLARED;;
esac
if [[ "$actor" != native ]];then
 helper=${B17R_ROLE_HELPER:?}
 case "$(realpath "$helper")" in "$root"/*) ;; *) fail ROLE_HELPER_OUTSIDE_TASK;;esac
 source "$helper"
 [[ $(b17r_get_actor /admin/posts) == authenticated ]] || fail ADMIN_ACTOR_MAPPING
 [[ $(b17r_get_actor /p/preflight) == anonymous ]] || fail PUBLIC_ACTOR_MAPPING
 b17r_prepare_headers anonymous
 [[ ${#B17R_HTTP_HEADERS[@]} == 0 ]] || fail ANONYMOUS_COOKIE_HEADER
 if [[ "$actor" == authenticated || "$actor" == mixed ]];then
  b17r_prepare_headers authenticated
  [[ ${#B17R_HTTP_HEADERS[@]} == 2 ]] || fail AUTHENTICATED_COOKIE_HEADER
 fi
 unset B17R_HTTP_HEADERS
fi
if [[ -n ${B17R_BASE:-} ]];then
 [[ "$B17R_BASE" =~ ^http://127[.]0[.]0[.]1:[0-9]+$ ]] || fail BASE_NOT_LOOPBACK
fi
printf 'B17R_PREFLIGHT=PASS ACTOR=%s RESULT_WRITABLE=1 REQUEST_PLAN_ROWS=%s\n' "$actor" "$rows"
