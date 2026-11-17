# Native terminal and literal contract

Status: the fresh Native fixed point, existing core regressions and dedicated
terminal/literal contracts below passed. Typed Unicode terminal input, quoted
Git paths and complete app/browser behavior remain pending. DB commercial
readiness remains HOLD because checkpoint fault recovery is unresolved.

## Terminal API

The Linux/POSIX runtime exposes tty-raw-enter!, tty-raw-leave!, tty-read-byte
and tty-run through the existing Native builtin registry. Existing opcode
positions are preserved. The bridge uses the existing C/POSIX dependencies.

Raw access requires FREELANG_NATIVE_TTY=1, the main thread, a terminal on
standard input and foreground process-group ownership. A private CLOEXEC
duplicate retains the original terminal even if standard input is rebound.
Entering raw mode is idempotent for its owner; a different thread receives
an explicit error. Leaving restores the saved termios, including after the
capability variable is revoked. Normal exit, uncaught language errors and
process-exit register the same restoration through atexit.

tty-read-byte() blocks until one byte is available. Its optional nonnegative
integer timeout is in milliseconds; zero polls once and nil means timeout.
Successful reads return integers 0..255. EOF, a lost foreground group or an
IO failure raises an explicit terminal error. Polling releases the runtime
mutex and retries interrupted waits against a monotonic deadline.

tty-run(executable, arguments[, cleanup-controls]) supervises an interactive
Native child with inherited terminal descriptors. It requires a dedicated
single main thread without existing children. A child process group and
startup handshake establish foreground ownership before execution. The
FREELANG_NATIVE_TTY capability is enabled only in the child's environment.
The executable and each argument are NUL-free strings; no shell is invoked.

The host waits for normal or signal termination and forwards SIGINT, SIGQUIT,
SIGHUP and SIGTERM. It escalates an unresponsive child after 500 milliseconds and reaps
owned descendants. It restores the saved termios and foreground group before
writing optional cleanup controls, then restores signal dispositions, the
signal mask and the previous subreaper setting. The return value preserves
the process exit code or 128 + signal number. A cleanup failure raises an
explicit error. A broken output pipe is handled after terminal restoration.

This uses the foreground-group rules documented by
[tcsetpgrp](https://man7.org/linux/man-pages/man3/tcsetpgrp.3.html), exact
saved settings from [termios](https://man7.org/linux/man-pages/man3/termios.3.html),
non-reaping observation from [waitid](https://man7.org/linux/man-pages/man2/waitpid.2.html)
and Linux [child subreapers](https://man7.org/linux/man-pages/man2/PR_SET_CHILD_SUBREAPER.2const.html).
Fatal loss of the supervising host itself (including SIGKILL or a host crash),
losing both processes or losing the terminal device remains outside the restoration guarantee. Direct
raw consumers should use the supervisor for signal-termination recovery.

## Literals and interpolation

The lexer consumes exactly two valid hexadecimal digits after a lowercase
backslash-x escape, matching the inspected reference lexer. A malformed
hexadecimal escape preserves the reference fallback: the x and following
characters remain text. char-from-code follows the reference low-eight-bit
conversion, including finite truncation and wrapping.

The compiler emits length-aware C string literals, escapes control bytes as
fixed-width octal sequences and preserves embedded NUL bytes. The shared
constructor, concatenation, join, replacement, substring and search paths
needed by the compiler use stored byte lengths. Legacy byte-index operations
retain byte indexing; this change does not establish binary safety for every
remaining legacy CString helper.

Template references are parsed into child AST nodes before lexical binding
resolution. Local shadowing, global bindings and closure captures use the
normal resolver. Parts evaluate in source order exactly once, nil becomes
empty text and other values use the existing JavaScript-compatible value
conversion. Both dollar-brace and hash-brace forms are supported, together
with dotted property lookup and parenthesized expressions. Unclosed or empty
interpolation remains literal text. Nested braces inside interpolation are
not added as a new grammar feature.

String tokens accumulate immutable chunks with binary carries before a
single join. This avoids repeatedly copying the entire growing token.

## Evidence

- Baseline: Native TUI output contained the literal characters x1b rather
  than an ESC byte. The refined host cleanup assertion failed, exit 134.
- Baseline: a diff screen returned before Enter, exit 1. Ordinary key reads
  now block; the frontend applies a 100 ms timeout only to escape sequences.
- Baseline: the independent literal byte-oracle fixture could not link its
  template because the generated C used an unresolved source variable name.
  This was a compilation failure, not a completed byte comparison.
- First batch: template generation exited 1 because its newly added prefix
  selector called an unavailable min function. Selecting the earlier prefix
  now uses a direct comparison. This fixes that compiler code dependency;
  the advertised min builtin's Native runtime gap remains separate work.
- Second batch: the independent hexadecimal-literal oracle matched all 436
  output bytes, exit 0, empty stderr. It includes all 256 escape values,
  malformed escapes, a literal backslash, Unicode, NUL before digits and
  a template containing NUL. Output SHA-256:
  77065c3474708dea4df34c99af0dd89466c50464f6aca763f5856379504cbab0.
- Second batch: template binding/formatting output matched all 152 expected
  bytes, exit 0, empty stderr, including globals, local shadowing, closure
  capture, mixed prefixes, property lookup, nil/false/array conversion and
  ordered side effects. Output SHA-256:
  7425b4f2cf6172c8732400be6b5c12a53eae50af0148e078fa2f0fa49b467e89.
- Second batch: the unchanged C ABI NUL assertion failed, exit 134. The
  separator-first str_join adapter used strlen and quoted display conversion.
  Its correction delegates to the canonical language join and returns UTF-8
  for C callers. The C adapter's empty default for a non-string separator is
  retained. Verification after this correction: PASS, including stored-length NUL joins and both C callers. The baseline legacy join produced six quoted bytes where the canonical result contained seven unquoted/NUL-preserved bytes.
- The third broad batch passed C ABI, raw PTY, character conversion and all
  18 host restoration/termination markers, then failed at the Enter prompt.
  The broad batch was stopped after three failures. Separate focused probes
  found print and println failing to flush before raw input, and the legacy
  print path truncating embedded NUL. Both APIs now write stored string
  lengths and flush. Before-input visibility, seven-byte output and the
  unchanged Enter/menu restoration assertions then passed, exit 0.
- An intermediate fresh bootstrap passed Stage 2 = Stage 3 = Stage 4 after
  explicitly ordering nested push generation/evaluation. Its frontend 71,
  CLI 22, FX 11, checker 32 and four DB core cases passed. Later component
  contracts exposed and corrected value-position tail-call leakage and
  missing flat cond emission inside loops. The final fresh validation passed
  Stage 2 = Stage 3 = Stage 4, frontend 71/71, CLI 22/22, FX 11/11, checker
  32/32 plus default pages and DB cases 7/13/36/40, all exit 0.
- Native str-split now allocates its private result vector once rather than
  copying all previous fields for each append. The original syntax component
  including a 100k-character literal passed 5/5; a six-case independent
  split/JSON-surrogate/NUL/large-input contract passed with a freshly built
  21-object runtime. Public immutable push semantics are unchanged.
- Full Native static-check CLI integration, typed Unicode TUI input,
  Git-quoted status paths, complete app/browser coverage and general binary
  safety of all legacy string helpers remain pending.
- Existing tests, expectations, timeouts, runners and bootstrap input remain
  unchanged. No new JavaScript source or JS line changes are introduced.

## Final Native receipt (2026-11-18 KST)

- Generated Stage 2/3/4 C SHA-256:
  69ec33d69ea93079296fa5723cc84e4b5b50e4466a6bd43d379385056bc72999.
- Stage 3 compiler ELF SHA-256:
  54d85380a55275e5ecce727e2e491e8acfb5fe015d701f2cc21ae0a217bf1d88.
- Stage 2 to 3: 27.77 s, peak 1,141,916 KiB; Stage 3 to 4: 28.76 s,
  peak 1,139,816 KiB; exact comparisons and processes exit 0.
- The final compiler regenerates literal/template contracts before comparing
  all 436/152 expected bytes. Value-context 3/3, ordered-push 3/3, syntax
  guard 5/5 and split/JSON-surrogate/NUL/large-input 6/6 all pass.
- Fresh terminal artifacts pass raw capability/owner/byte checks, character
  conversion 4/4, all 18 host restoration/reaping markers and Enter/menu
  restoration. Actual SGR mouse selection, keyboard unstage and ESC sequence
  handling are exercised in a private repository with no remotes.
- Collected core and terminal execution traces contain zero
  Node/npm/npx/bun/deno/tsx/ts-node executions. This covers these verified paths,
  not every frontend build or app command still awaiting migration.
- Source manifests match before/after each batch. Bootstrap input SHA-256:
  0101cb63a86f20b93451f70b88a0b577f9548ded0a8b328ebdeedb7e8e4fd767.
  The protected pinned bootstrap verifier remains NOT_RUN because it expects
  an older seed; the seed and verifier were not altered.
- Evidence identifiers: native-core-value-split.gzQgDG and
  native-terminal-final.bv22g0. Paths and owners: <가림>.
- The private curl prefix remains an environment-scoped build dependency;
  production dependency packaging/portability has not been verified.
