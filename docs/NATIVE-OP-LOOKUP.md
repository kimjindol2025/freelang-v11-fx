# Native operator lookup

The compiler's ordinary builtin discovery preserves the existing public names,
name normalization and closure opcodes, but no longer scans the whole immutable
operator list on every lookup. Both fl_v11_has and fl_v11_fn use the same static
index. Slots retain the original first matching ordinal from v11_ops; the
operator array and callback ABI are unchanged.

pthread_once publishes the index before any reader uses it. The table has 405
size_t slots for the current 201 names (3,240 bytes on this host), uses no
dynamic allocation and never caches caller strings or request-arena pointers.
Only runtime/v11-compat.h changes executable behavior.

## Why this change

The previous accepted semantic fixed point generated C in 39.14 and 42.36
seconds; the second result failed the existing 40-second criterion. That
failure is retained in NATIVE-JOIN-LINES.md. A separate, already-installed
gprof diagnostic observed 43,876,386 fl_v11_has calls. At that revision each
lookup scanned the static name array. Instrumented timings were excluded from
acceptance measurements.

## Verification

Paths/owners: <가림>. Receipt identifier: native-op-index.ziw7ijun.
The private contracts were written and run against the prior runtime before
the implementation changed; the same source/output expectations were retained.

| Check | Result |
|---|---|
| Existing names, underscore aliases, unknown names, long/embedded-NUL/Unicode names and nonstring receivers | 2,673 cases; old/new output byte difference 0 |
| Captured closure ordinals | Exact old first-match indices; callable identity callback also passes |
| Concurrent first initialization and lookup | 8 threads, 32,768 calls; difference 0 |
| Fresh compiler build from the unchanged seed | Exit 0; Stage2=3=4 exact bytes |
| Generated C versus the previous semantic fixed point | Byte difference 0 |
| Stage2→3 generation | 34.08 seconds; peak RSS 1,525,704 KiB; exit 0 |
| Stage3→4 generation | 35.95 seconds; peak RSS 1,526,584 KiB; exit 0 |
| Existing 40-second / 1.6-GiB generation criteria | PASS for both measured generations; process cap remains 2 GiB |
| Fresh matching runtime / public line-join contracts | 24/24 |
| Original frontend unit assertions | 71/71 |
| Native CLI integration | 22/22 |
| FX language regressions | 11/11 |
| Checker fixtures and default check | 32/32 and exit 0 |
| Existing DB language/runtime cases | 7/13/36/40, each exit 0 |
| DB storage SDK regression | WAL 25/25, Unicode 4/4, checkpoint 28/28; original 11/12/41/42, independent writer/reader and 3,200-record test pass |

No timing-only rerun replaced a failure. These are the two generations from a
single fresh build after a substantive implementation change, using the same
build flags, recipe and measurements as before. They establish this measured
result, not a universal latency guarantee.

- Compiler ELF SHA-256:
  e966e6342053c48fdd3af001a029e0d4a4923832f5122f0f300696ce8d1b2c94
- Stage2/3/4 C SHA-256:
  c1f48e888960f21679278d4e90ce0d2e05d3e51bbfecefafcd212819905458fa
- Fresh runtime archive SHA-256:
  09be8c24c8c2bff543d6d9a7c5942595529473759c60b52be18ff061951624f1
- Seed before/after SHA-256:
  0101cb63a86f20b93451f70b88a0b577f9548ded0a8b328ebdeedb7e8e4fd767

The protected bootstrap verifier retains its old seed pin; it was not edited or
used to claim this result. The existing private recipe compiles the actual
unchanged seed through Stage4. The already-approved private curl 8.9.1 header
and library hashes were verified and attached only through process environment
variables. No installation, download, JS/Node build stage, seed regeneration,
test/expectation/timeout/protected-runner change was made.

Collected execve traces show zero Node-family executions. Collected DB/probe network traces show zero connect calls; exec-only traces do not establish network absence. The last original DB test was interrupted by an Orca restart during C linking, before its executable ran. That incomplete trace/log is retained. Its already-generated C was reused with the original worker source list, flags, environment categories and 360-second bound in a fresh output directory; the resumed test exits 0 with all five original markers and identical before/after row count/hash. Earlier successful cases were not rerun. A new, unrelated frontend dirty state was observed after the core receipt: the 71-assertion result refers to the original suite at frontend HEAD b6d443a95098dc1bdc6f349879070acaf89a6f2f before those separate edits. Those changes and their additional tests are not claimed as verified here.

Full Node removal remains open: default CLI/check/test and dev/deploy paths
still need complete verification and migration. This SDK's whole web/browser
rebuild is NOT_RUN. DB commercial readiness remains HOLD: the full Native 71
and the outstanding commit-I/O health, ownership, generation reclamation,
concurrency, long-running load, security and operating procedures are not
proved by the subset above. No customer database or production deployment was
accessed.
