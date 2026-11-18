# Native line joining

str_join_lines / str-join-lines joins an array with LF separators. It reuses
the existing Native str-join value coercion: nil elements are empty, nested
arrays use their normal string conversion, and Unicode/UTF-16 text is
preserved. It adds no trailing LF. Missing/nonarray input raises TypeError;
extra arguments are ignored as in the inspected reference function.

The reference is the existing bootstrap.js implementation at line 27223,
which applies array join with an LF separator. That file was read only and
was not executed or edited. No authored JS file or JS line increase is added.

## Evidence

Host paths/owners: <가림>. Private cohort native-db-wal.nrizo_40.

Before the change, the verified Native runtime reported the operation absent
(capability probe exit 77). The original tests/silent-tcp-wal-recovery.fls
compiled and linked but exited 1 with Function not found: str_join_lines
before its 3,200-record recovery assertion. Its source/expectations were not
changed. With the new compiler/runtime, that same original fixture exits 0,
prints all five original recovery/corruption PASS markers and preserves its
one committed row and row hash across the 3,200 pending prepare records.

| Verification | Result |
|---|---|
| Public Native line-join contracts | 24/24 |
| Contract data | Empty/single/multiple lines, LF/quotes/backslash, Korean/emoji, nil, booleans, objects/nested arrays, lone UTF-16 unit, wrong receivers and argument counts |
| Actual seed through Stage4 | Stage2=3=4 exact bytes |
| Current compiler ELF SHA-256 | eb23e2039163794c641d2f3f3f076ddf5cd3e3d6bff5800216ab638e39d2361b |
| Stage2/3/4 C SHA-256 | c1f48e888960f21679278d4e90ce0d2e05d3e51bbfecefafcd212819905458fa |
| Fresh core | Frontend 71/71, CLI 22/22, FX regressions 11/11, checker 32/32 plus default checking |
| Original DB language/runtime subset | 7/13/36/40, each exit 0 |
| Updated DB storage / unchanged original workers | WAL contracts 25, Unicode storage 4, checkpoint contracts 28, original cases 11/12/41/42 and independent writer/reader pass |
| Seed before/after | 0101cb63a86f20b93451f70b88a0b577f9548ded0a8b328ebdeedb7e8e4fd767 |
| Source/tests/limits | Existing tests, expectations, protected runners and 2 GiB limit unchanged |

Bootstrap receipt: native-join-fixedpoint.zym7ud8t. Core receipt:
native-join-core.1uxuiwsx. The stale pin in the original protected bootstrap
verifier remains unchanged; the documented private recipe runs the actual
unchanged seed and compares the generated stages. Existing approved private
curl 8.9.1 is connected only by process environment, with its header/library
hashes checked. Packaging remains pending; no package was installed.

## Performance at this milestone

The two final standard generations took 39.14 and 42.36 seconds, with peak
RSS 1,528,792 and 1,526,048 KiB. Memory remained below the limit; the latter
run missed the 40-second target. Exact fixed-point equality is PASS, and the
current time criterion is not accepted. No timing-only rerun replaced it.

Diagnostic receipt native-codegen-profile.c383mbrw builds a separate -pg
compiler, preserves generated C bytes and leaves the real seed/compiler
untouched. Its instrumented timing is not an acceptance measurement. gprof
records 43,876,386 fl_v11_has calls; that function currently scans the static
operator list linearly. This is evidence for a lookup-index optimization,
not a claim that its cost alone explains the elapsed-time miss. Profiling
uses the already-installed tool; no download/install or privileged change
was performed. Full Node removal and commercial DB readiness remain open.

A subsequent measured lookup-index change passes the same time criterion with
34.08 and 35.95 seconds while preserving generated C bytes. See
NATIVE-OP-LOOKUP.md for the new compiler, contracts and regression receipts.
The earlier failure and instrumented diagnostic above remain historical evidence.
