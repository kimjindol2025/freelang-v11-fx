# V-17 failure classification

- RUN-ID: `RUN-20261104-17`
- FX base used in V-16: `b8fdb29b94eec109db245172237c2d5a55e4c001`
- AFJ DB95 gate snapshot used in V-16: `b60320b86de044179f284d2e7aa537a111e6da0f`
- V-16 report commit: `979f3ba945f56e18d09ba7788f1df734856c7012`
- Compiler SHA-256 from V-16: `9e08f102a579aed6ad77edac06997637e9f7b05dbe7197de1002107a6e905864`

No gate was rerun. The 71 status/time results below come from the V-16 report. Input paths and line counts come from `tests/release-gate.sh` and its source files at AFJ snapshot `b60320b86de044179f284d2e7aa537a111e6da0f`. Counts use `wc -l`. File names are repository-relative.

## Evidence limit

V-16's per-case output and process-cleanup logs were removed after its report was pushed, following the cleanup instruction. The V-16 report preserved the aggregate results, all case statuses and elapsed times, a limited set of error lines, and the timeout cleanup totals. As a result, the original first error line and exact failing phase cannot be recovered for every failed case. The table marks those cells as unavailable instead of inferring them from exit code alone. The runner's source identifies the fixed behavior for `.js` inputs, but that does not recover each case's original first compiler diagnostic.

## 71-case table

`Phase` is the observed failing stage when output or runner behavior establishes it. `Unknown` means V-16's case log needed to distinguish FreeLang→C, GCC, execution, and comparison is unavailable.

| # | Result | Time (ms) | Input (lines) | Failed phase | First error line |
|---:|---|---:|---|---|---|
| 1 | FAIL (exit 124) | 120005 | tcp-server.fls (2044) | FreeLang→C (residual Stage2 compiler observed) | `V-16 case log not retained; original first line unavailable` |
| 2 | PASS (exit 0) | 8013 | afj-db95.fls (486) | — | `—` |
| 3 | FAIL (exit 1) | 12266 | binary-raw.fls (6) | Execution | `Uncaught error: [buf-to-binary] Function not found: buf-to-binary (at line 2, col 0)` |
| 4 | FAIL (exit 1) | 27628 | mariadb-wire-codec.fls (10) | Execution | `Uncaught error: [buf-str] Function not found: buf-str (at line 116, col 0)` |
| 5 | FAIL (exit 1) | 27773 | mariadb-charset-contract.fls (8) | Execution | `Uncaught error: [buf-str] Function not found: buf-str (at line 116, col 0)` |
| 6 | FAIL (exit 124) | 120005 | collation-contract.fls (43) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 7 | FAIL (exit 1) | 22849 | afj-db95.fls (486) | Execution/comparison | `Uncaught error (line 455): [oversized-wal-smoke-test] oversized WAL was accepted (at line 486, col 0)` |
| 8 | FAIL (exit 124) | 120005 | metrics-contract.fls (32) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 9 | FAIL (exit 127) | 122 | canary.sh (29) | Execution | `<가림>/.freelang/smoke.sh: line 11: node: command not found` |
| 10 | FAIL (exit 124) | 120005 | large-corpus-soak.fls (45) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 11 | FAIL (exit 124) | 120005 | tcp-catalog-pages.fls (133) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 12 | FAIL (exit 124) | 120005 | tcp-lazy-catalog.fls (110) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 13 | FAIL (exit 1) | 25263 | persistent-btree.fls (22) | Execution/comparison | `Uncaught error (line 455): [oversized-wal-smoke-test] oversized WAL was accepted (at line 486, col 0)` |
| 14 | FAIL (exit 124) | 120005 | btree-range.fls (31) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 15 | FAIL (exit 124) | 120005 | compound-index.fls (87) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 16 | FAIL (exit 124) | 120005 | compound-range.fls (126) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 17 | FAIL (exit 124) | 120005 | tcp-schema-mvcc.fls (61) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 18 | FAIL (exit 124) | 120005 | composite-type-order.fls (17) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 19 | FAIL (exit 124) | 120005 | composite-null-order.fls (15) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 20 | FAIL (exit 124) | 120005 | composite-schema-order.fls (38) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 21 | FAIL (exit 124) | 120005 | composite-mixed-type-order.fls (28) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 22 | FAIL (exit 124) | 120005 | composite-reopen-rebuild.fls (31) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 23 | FAIL (exit 124) | 120005 | composite-schema-range.fls (22) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 24 | FAIL (exit 1) | 769 | tcp-concurrency-smoke.js (143) | FreeLang→C | `[IR-ERR] literal bad type=unknown \| kind=literal line=138` |
| 25 | FAIL (exit 1) | 856 | tcp-long-soak.js (43) | FreeLang→C | `[CGC-ERR] unsupported literal type=unknown line=11` |
| 26 | FAIL (exit 1) | 21797 | backup.fls (406) | Unknown (V-16 case log not retained) | `V-16 case log not retained; original first line unavailable` |
| 27 | FAIL (exit 1) | 21119 | backup-soak.fls (26) | Unknown (V-16 case log not retained) | `V-16 case log not retained; original first line unavailable` |
| 28 | FAIL (exit 1) | 12555 | compatibility.fls (41) | Unknown (V-16 case log not retained) | `V-16 case log not retained; original first line unavailable` |
| 29 | FAIL (exit 124) | 120005 | error-mapping.fls (23) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 30 | FAIL (exit 124) | 120005 | mariadb-remediation-regression.fls (137) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 31 | FAIL (exit 124) | 120005 | mariadb-unsupported-contract.fls (34) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 32 | FAIL (exit 124) | 120006 | tcp-database-scope.fls (51) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 33 | FAIL (exit 124) | 120005 | wal-format.fls (88) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 34 | FAIL (exit 1) | 40 | fault-injection-restart.sh (55) | Unknown (V-16 case log not retained) | `V-16 case log not retained; original first line unavailable` |
| 35 | FAIL (exit 1) | 868 | tcp-transaction-kill-recovery.js (141) | FreeLang→C/gate: JS input not ported | `Raw first line unavailable; runner terminal message: NATIVE_JS_SOURCE_NOT_PORTED` |
| 36 | FAIL (exit 124) | 120005 | transaction-isolation.fls (50) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 37 | FAIL (exit 124) | 120006 | tcp-isolation-levels.fls (76) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 38 | FAIL (exit 124) | 120005 | tcp-mvcc-conflicts.fls (312) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 39 | FAIL (exit 124) | 120004 | tcp-range-predicate.fls (136) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 40 | PASS (exit 0) | 13900 | mvcc-row-version.fls (32) | — | `—` |
| 41 | FAIL (exit 124) | 120005 | tcp-row-version-recovery.fls (44) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 42 | FAIL (exit 124) | 120006 | tcp-row-version-checkpoint.fls (65) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 43 | FAIL (exit 124) | 120006 | tcp-savepoint.fls (50) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 44 | FAIL (exit 124) | 120005 | tcp-deadlock.fls (37) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 45 | FAIL (exit 124) | 120005 | tcp-lock-timeout.fls (39) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 46 | FAIL (exit 124) | 120005 | tcp-prepared.fls (47) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 47 | FAIL (exit 124) | 120005 | request-limits.fls (20) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 48 | FAIL (exit 124) | 120005 | tcp-slow-timeout.fls (21) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 49 | PASS (exit 0) | 18065 | auth-lockout.fls (18) | — | `—` |
| 50 | FAIL (exit 1) | 18523 | auth-lockout-reset.fls (22) | Unknown (V-16 case log not retained) | `V-16 case log not retained; original first line unavailable` |
| 51 | PASS (exit 0) | 17416 | auth-state-recovery.fls (33) | — | `—` |
| 52 | FAIL (exit 124) | 120005 | session-cleanup.fls (19) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 53 | FAIL (exit 124) | 120005 | session-ttl.fls (17) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 54 | FAIL (exit 124) | 120005 | connection-rate-limit.fls (9) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 55 | FAIL (exit 124) | 120006 | ip-rate-limit.fls (24) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 56 | FAIL (exit 1) | 631 | ip-rate-limit-runtime.js (114) | FreeLang→C/gate: JS input not ported | `Raw first line unavailable; runner terminal message: NATIVE_JS_SOURCE_NOT_PORTED` |
| 57 | FAIL (exit 124) | 120006 | tcp-fragmented-requests.fls (12) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 58 | FAIL (exit 124) | 120005 | error-mapping.fls (23) | Unknown (timeout stage not preserved) | `V-16 case log not retained; original first line unavailable` |
| 59 | FAIL (exit 1) | 34 | production-tls-boundary.sh (29) | Unknown (V-16 case log not retained) | `V-16 case log not retained; original first line unavailable` |
| 60 | FAIL (exit 1) | 515 | native-tls-smoke.js (95) | FreeLang→C/gate: JS input not ported | `Raw first line unavailable; runner terminal message: NATIVE_JS_SOURCE_NOT_PORTED` |
| 61 | FAIL (exit 1) | 691 | native-tls-policy-smoke.js (102) | FreeLang→C/gate: JS input not ported | `Raw first line unavailable; runner terminal message: NATIVE_JS_SOURCE_NOT_PORTED` |
| 62 | FAIL (exit 1) | 577 | production-native-tls.js (104) | FreeLang→C/gate: JS input not ported | `Raw first line unavailable; runner terminal message: NATIVE_JS_SOURCE_NOT_PORTED` |
| 63 | FAIL (exit 1) | 1455 | tls-proxy-smoke.js (213) | FreeLang→C/gate: JS input not ported | `Raw first line unavailable; runner terminal message: NATIVE_JS_SOURCE_NOT_PORTED` |
| 64 | FAIL (exit 1) | 292 | mariadb-wire-unit.js (80) | FreeLang→C/gate: JS input not ported | `Raw first line unavailable; runner terminal message: NATIVE_JS_SOURCE_NOT_PORTED` |
| 65 | FAIL (exit 1) | 27136 | mariadb-sha1.fls (17) | Unknown (V-16 case log not retained) | `V-16 case log not retained; original first line unavailable` |
| 66 | FAIL (exit 1) | 1070 | mariadb-wire-prepared.js (157) | FreeLang→C/gate: JS input not ported | `Raw first line unavailable; runner terminal message: NATIVE_JS_SOURCE_NOT_PORTED` |
| 67 | FAIL (exit 1) | 856 | mariadb-wire-smoke.js (149) | FreeLang→C/gate: JS input not ported | `Raw first line unavailable; runner terminal message: NATIVE_JS_SOURCE_NOT_PORTED` |
| 68 | FAIL (exit 1) | 449 | native-mariadb-smoke.js (98) | FreeLang→C/gate: JS input not ported | `Raw first line unavailable; runner terminal message: NATIVE_JS_SOURCE_NOT_PORTED` |
| 69 | FAIL (exit 1) | 742 | native-mariadb-auth.js (126) | FreeLang→C/gate: JS input not ported | `Raw first line unavailable; runner terminal message: NATIVE_JS_SOURCE_NOT_PORTED` |
| 70 | FAIL (exit 1) | 27798 | native-mariadb-decoder.fls (42) | Unknown (V-16 case log not retained) | `V-16 case log not retained; original first line unavailable` |
| 71 | FAIL (exit 1) | 2324 | native-mariadb-prepared.js (157) | FreeLang→C/gate: JS input not ported | `Raw first line unavailable; runner terminal message: NATIVE_JS_SOURCE_NOT_PORTED` |

## Timeout input sizes and relation

The timeout cases are the rows with `exit=124` in the table. Their original input sizes are:

| Measure | Value |
|---|---:|
| Timeout cases | 39 |
| Total input lines | 4197 |
| Mean input size | 107.62 lines |
| Median input size | 39 lines |
| Minimum / maximum | 9 / 2044 lines |
| Under 50 input lines | 24/39 (61.5%) |

Excluding the 2044-line `tcp-server.fls` case 1, the other 38 timeout inputs average 56.66 lines; their median is 38.5 lines and their range is 9–312 lines. For comparison, the 28 non-timeout failures average 104.32 lines, with a median of 87.5 and a range of 6–486. Twelve of those 28 are under 50 lines (42.9%). These figures show no simple positive relationship between the root test file's line count and timeout: small test files also timed out.

The root file does not measure its loaded code graph. For example, the 9-line `connection-rate-limit.fls` and 32-line `metrics-contract.fls` both load `src/tcp-server.fls`; the 43-line COLLATE test loads `engine.fls` and `sql-core.fls`. Case 1 itself is 2044 lines and timed out at 120 seconds. DB95-16 measured `src/tcp-server.fls` at 2040 lines, 115 functions, and `ELAPSED_SECONDS=173.10`, and concluded that the cost accumulates with total code size and function count. V-16's case 1 and the small inputs that load large modules are consistent with that conclusion. The available data cannot show that all 39 timeouts occurred during FreeLang→C; V-16 did not retain each active-stage log.

## Non-timeout failures grouped by available error evidence

There were 28 failures other than the 39 timeouts. Groups below use the common observed terminal error or common runner outcome. Where V-16 logs were removed, the group is explicitly marked unknown.

| Error group | Count | Representative case and evidence |
|---|---:|---|
| `.js` input routed to the native compiler, then runner reports `NATIVE_JS_SOURCE_NOT_PORTED` | 14 | Case 24. Its preserved first compiler line is `[IR-ERR] literal bad type=unknown | kind=literal line=138`; case 25's is `[CGC-ERR] unsupported literal type=unknown line=11`. The other 12 raw first lines were not retained. The tested runner's `tests/native-release-gate.sh` `.js` branch (lines 32–35) always emits `NATIVE_JS_SOURCE_NOT_PORTED` and exits 1. |
| `Function not found: buf-str` | 2 | Case 4: `Uncaught error: [buf-str] Function not found: buf-str (at line 116, col 0)`; case 5 has the same line. |
| `oversized WAL was accepted` | 2 | Case 7: `Uncaught error (line 455): [oversized-wal-smoke-test] oversized WAL was accepted (at line 486, col 0)`; case 13 has the same line. |
| `Function not found: buf-to-binary` | 1 | Case 3: `Uncaught error: [buf-to-binary] Function not found: buf-to-binary (at line 2, col 0)`. |
| `node: command not found` | 1 | Case 9: `<가림>/.freelang/smoke.sh: line 11: node: command not found`. |
| Original error line unavailable; cannot group by wording | 8 | Cases 26, 27, 28, 34, 50, 59, 65, and 70. Six are `.fls` inputs and two are `.sh`; their V-16 per-case logs were removed. |
| **Total** | **28** |  |

The known groups account for 20 cases. Eight cannot be assigned to an error-message group without their original logs, so the grouping is incomplete by design and does not assume those errors match one another.

## Judgment

**(다) 섞여 있음.** The timeout count is large, and the measured `tcp-server.fls` case plus DB95-16 support cumulative FreeLang→C cost for large loaded code graphs. The other failures include 14 native-gate rejections of `.js` sources, missing runtime functions, an oversized-WAL assertion, and a missing `node` dependency. Eight non-timeout failures and 38 timeout phases lack preserved first-line/stage evidence. The V-16 evidence therefore does not support treating the 67 failures as one compiler-speed problem or naming one dominant fix.

The DB95-17 report available in the DB95 working tree analyzes code-size scaling but records no completed native-function profile or 71-case gate result; it does not restore the missing V-16 case logs. DB95-16's cumulative-cost finding is consistent with the evidence above, with the limits noted.

## Language outcome

The preserved evidence supports **5 broad observed failure families**: cumulative compile-time timeouts; `.js` inputs rejected by the native gate; missing runtime functions; the oversized-WAL assertion; and a missing `node` test dependency. Only the first two directly involve the FreeLang→C path, and the `.js` result is a gate limitation rather than a confirmed language defect. Eight non-timeout failures remain unclassified because their original logs were removed.

## Score

The report commit `b41301c6a7d0f94831d32b47c1d83a8b95c42d69` was pushed to Forgejo (`origin`) and then GitHub (`github`). Immediately after both pushes, `git ls-remote` returned `b41301c6a7d0f94831d32b47c1d83a8b95c42d69` for `refs/heads/fix/raw-tcp-review` on both remotes.

| Criterion | Result |
|---|---|
| 1/4: 71 rows with raw original first-error evidence | PARTIAL — all 71 result/time/source-size rows are present, but first error lines and exact phases are unavailable for many cases because V-16 logs were cleaned. |
| 2/4: numeric relation between input size and timeouts | PASS — timeout and non-timeout distributions are compared above. |
| 3/4: group all 28 non-timeout failures by same error wording | PARTIAL — 20 have a supported group; 8 original messages are unavailable. |
| 4/4: judgment, report commit, Forgejo then GitHub, matching heads | PASS — commit `b41301c6a7d0f94831d32b47c1d83a8b95c42d69` was pushed in order and both refs matched afterward. |

V17_SUMMARY=2/4
