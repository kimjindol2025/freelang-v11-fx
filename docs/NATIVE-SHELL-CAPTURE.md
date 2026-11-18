# Native shell_capture for web Git APIs

The Native web server reached the real Git status API, then failed with
`Function not found: shell_capture`. The Native runtime now implements that
existing module operation. It runs `sh -c` and returns exactly `stdout`,
`stderr` and `code`; it does not substitute the differently shaped existing
`shell-exec-result` result.

The operation requires a string command, rejects embedded NUL bytes, gives
the child EOF on stdin, collects the two streams independently and applies
the existing one-MiB per-stream capture bound. Output is normalized to UTF-8
with replacement for invalid input. Success returns the shell exit status;
spawn, signal, timeout and capture failures use code 1, matching the module's
`status ?? 1` behavior. This is an existing shell execution facility, not a
sandbox: callers must quote untrusted shell arguments and restrict commands.

A monotonic 30,000 ms deadline covers both output collection and the final
child wait, including commands that close both streams before sleeping.
Each invocation creates its own child process group. Capture cancellation
kills that group and waits for the direct child. It never targets unrelated
process groups. Cleanup after termination of the host itself is not proven.
The legacy `shell-exec-result` operation keeps its original four fields,
unbounded time profile and -1 signal/spawn failure status.

## Contract evidence

Receipt `native-shell-capture.sho7dtio` passed 17 private cases using generated
FreeLang/C binaries linked with a freshly compiled runtime archive:

| Cases | Result |
|---|---|
| stdout/stderr, exit 7, Korean/emoji, NUL output, invalid UTF-8, EOF stdin | PASS |
| Signal failure and first-class invocation | PASS |
| Nil, number and NUL-containing commands | Rejected, exit 1 |
| Simultaneous 60 KiB stdout/stderr; output above one MiB | PASS |
| Existing result shape and signal status | PASS |
| Deadline while pipes remain open | 30.045 s; owned descendant no longer running |
| Deadline after both streams reach EOF | 30.017 s |
| Collected exec/network traces | Node 0; network requests 0 |
| FX source/seed manifest before and after | Identical |

The probe used the previously verified compiler C with the current runtime
as a provisional compiler; it is not itself a fresh four-stage bootstrap.
Fresh bootstrap, core regression and complete web API integration results are
recorded separately before this change is accepted as a checkpoint.

Existing tests, expected results, timeouts and protected runners were not
changed. No JS, dependency download, package installation, seed generation,
customer database access or order storage was performed. The approved
private curl dependency is connected only through process environment.
Host paths and owners: <가림>.

## Fresh current-runtime bootstrap

Receipt `native-shell-fixedpoint.3gcxm52h` rebuilt the actual seed through
Stage 4 with strict source preflight. Stage 2 = 3 = 4 C SHA-256:

`c1f48e888960f21679278d4e90ce0d2e05d3e51bbfecefafcd212819905458fa`

Stage-3 ELF SHA-256:
`d4f80c5ca59f67187a205db4ed9a4578f12b832d08fb336cfa78f9203ed6db42`.
Generation took 40.29 / 39.97 s, with 1,528,984 / 1,528,352 KiB peak RSS.
The 40-second target was missed by one measurement; no timing-only retry
was performed. The 2 GiB cap remained unchanged. Seed before/after:
`0101cb63a86f20b93451f70b88a0b577f9548ded0a8b328ebdeedb7e8e4fd767`.
The complete source/runtime/seed manifests were identical before and after.
The official protected verifier with its older seed pin remains NOT_RUN;
its runner and seed were not altered.

## Fresh core verification

`native-shell-core.jn7p3ky4` passed frontend 71/71, CLI 22/22, FX 11/11,
page checker 32/32 plus defaults, literal/value/split/push contracts and the
original DB95 cases 7/13/36/40 (all exit 0). Its generation and exec audits
found no compiler-generation errors or Node-family execution. The complete
current source/runtime/seed manifest remained unchanged. The DB subset uses
the existing Native gate; it is not a full DB commercial result.

The web builder and Git API integration are tracked by the frontend's
NATIVE-WEB-BUILDER.md. Actual browser execution and full DB95 acceptance remain
unfinished; commercial readiness is HOLD.
