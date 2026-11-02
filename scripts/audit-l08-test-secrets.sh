#!/usr/bin/env bash
set -Eeuo pipefail
trace="${1:?execve trace required}";shift
[[ -f "$trace" && "$#" -gt 0 ]]
# Read only explicitly supplied synthetic curl configs; never print their values.
awk '
FILENAME != ARGV[ARGC-1] {
 if(match($0,/Cookie: blog_session=([^"]+)/,a)){values[a[1]]=1;cookie=1}
 if(match($0,/Bearer ([^"]+)/,a)){values[a[1]]=1;bearer=1}
 if(match($0,/csrf=([^"]+)/,a)){values[a[1]]=1;csrf=1}
 next
}
{for(value in values)if(index($0,value))hits++}
END {
 printf "SYNTHETIC_SECRET_TYPES=%d\n",cookie+bearer+csrf
 printf "SYNTHETIC_SECRETS_IN_EXECVE=%d\n",hits
 exit(hits!=0||cookie+bearer+csrf!=3)
}' "$@" "$trace"
