#!/usr/bin/env bash
# Keep the upstream gate unchanged. Its native runs already invoke strace;
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
