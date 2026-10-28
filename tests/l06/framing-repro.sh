#!/usr/bin/env bash
set -Eeuo pipefail
port="$1"
for case_name in TE_CL_OVERSIZE DUPLICATE_CL_OVERSIZE; do
  exec {fd}<>"/dev/tcp/127.0.0.1/$port"
  printf 'POST /upload HTTP/1.1\r\nHost: localhost\r\nContent-Type: multipart/form-data; boundary=B\r\nContent-Length: 67108865\r\n' >&"$fd"
  if [[ "$case_name" = TE_CL_OVERSIZE ]]; then
    printf 'Transfer-Encoding: chunked\r\n\r\n' >&"$fd"
  else
    printf 'Content-Length: 0\r\n\r\n' >&"$fd"
  fi
  IFS=' ' read -r protocol status reason <&"$fd"
  printf '%s_HTTP=%s\n' "$case_name" "$status"
  cat <&"$fd" >/dev/null
  exec {fd}>&-
done
