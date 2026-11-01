#!/usr/bin/env bash
# LSan cannot execute under ptrace. Trace builds, run sanitized programs directly.
platform_trace() {
 local script=${!#} trace="" real_gcc real_strace bin logs
 if [[ -f "$script" ]] && rg -q -- '-fsanitize=address' "$script";then
   while (( $# )); do
    case "$1" in
     -f) shift;;
     -e) shift 2;;
     -o) trace=$2;shift 2;;
     *) break;;
    esac
   done
   [[ -n "$trace" && $# -gt 0 ]] || return 2
   real_gcc=$(command -v gcc) || return
   real_strace=$(command -v strace) || return
   bin="${trace%.execve.log}.build-bin";logs="${trace%.execve.log}.build-traces"
   mkdir -p "$bin" "$logs" || return
   cat > "$bin/gcc" <<'SH'
#!/usr/bin/env bash
exec "$LSAN_REAL_STRACE" -f -e trace=execve -o "$LSAN_BUILD_LOG/gcc-$BASHPID.execve.log" "$LSAN_REAL_GCC" "$@"
SH
   chmod +x "$bin/gcc" || return
   printf 'LSAN_PROGRAM_EXECUTION=OUTSIDE_PTRACE\nBUILD_EXECUTION=STRACE\n' > "${trace%.execve.log}.trace-mode.txt"
   PATH="$bin:$PATH" LSAN_REAL_GCC="$real_gcc" LSAN_REAL_STRACE="$real_strace" LSAN_BUILD_LOG="$logs" "$@"
 else
  command strace "$@"
 fi
}
platform_trace_env() {
 local assignment=$1;shift
 local -x "$assignment"
 platform_trace "$@"
}
