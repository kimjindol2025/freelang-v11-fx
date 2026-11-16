#!/usr/bin/env bash
set -Eeuo pipefail
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)
prefix=${1:?isolated codec prefix required}
output=${2:?output executable required}
: "${TMPDIR:?caller must choose task temporary directory}"
gcc -std=c11 -D_POSIX_C_SOURCE=200809L -O2 -Wall -Wextra -I "$prefix/include" \
  "$root/tools/image-worker.c" "$prefix/lib/libpng16.a" "$prefix/lib/libjpeg.a" \
  "$prefix/lib/libgif.a" "$prefix/lib/libz.a" -lm -o "$output"
echo IMAGE_WORKER_BUILD=PASS
