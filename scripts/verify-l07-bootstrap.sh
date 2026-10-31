#!/usr/bin/env bash
# The native gate already invokes strace;
# retain those logs and trace gcc separately, avoiding nested ptrace tracers.
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
out="${L07_OUT:-$(mktemp -d)}"
mkdir -p "$out/bin" "$out/traces"
export L07_TRACE_ARCHIVE="$out/traces"
export L07_REAL_STRACE="$(command -v strace)"
export L07_REAL_GCC="$(command -v gcc)"
cat >"$out/bin/strace" <<'SH'
#!/usr/bin/env bash
set -uo pipefail
log=""; previous=""
for argument in "$@"; do
  if [ "$previous" = -o ]; then log="$argument"; fi
  previous="$argument"
done
"$L07_REAL_STRACE" "$@"
rc=$?
if [ -n "$log" ] && [ -f "$log" ]; then
  cp -- "$log" "$L07_TRACE_ARCHIVE/$(basename -- "$log")"
  # Preserve generated inputs before the unchanged gate's cleanup trap runs.
  directory=$(dirname -- "$log")
  for artifact in stage1.c stage2.c stage3.c stage4.c stage1-bin stage2-bin stage3-bin; do
    if [ -f "$directory/$artifact" ]; then
      cp -- "$directory/$artifact" "$L07_TRACE_ARCHIVE/$artifact"
    fi
  done
fi
exit "$rc"
SH
cat >"$out/bin/gcc" <<'SH'
#!/usr/bin/env bash
exec "$L07_REAL_STRACE" -f -e trace=execve \
  -o "$L07_TRACE_ARCHIVE/gcc-$BASHPID.execve.log" "$L07_REAL_GCC" "$@"
SH
chmod 700 "$out/bin/strace" "$out/bin/gcc"
PATH="$out/bin:$PATH" bash "$ROOT/scripts/verify-native-bootstrap.sh"
test -n "$(find "$out/traces" -type f -name '*.execve.log' -print -quit)"
echo L07_BOOTSTRAP_TRACE_ARCHIVE=RETAINED
