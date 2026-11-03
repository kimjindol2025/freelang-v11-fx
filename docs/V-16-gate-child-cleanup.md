# V-16 gate child cleanup

## Inputs and compiler integrity

- Run: `RUN-20261104-07`
- FX base: `b8fdb29b94eec109db245172237c2d5a55e4c001`
- AFJ DB95 gate snapshot: `b60320b86de044179f284d2e7aa537a111e6da0f`
- Report branch at start: `fix/raw-tcp-review`, HEAD `ad0acf04122acfdd5555c5f8c9c0956bf763f42d`
- Compiler: `<가림>/stage2-compiler`
- Compiler SHA-256, before and after: `9e08f102a579aed6ad77edac06997637e9f7b05dbe7197de1002107a6e905864`
- Stage2 source SHA-256, before and after: `6a0d4e384a18834d4f0cb03c10460aade5bf05f999f5db981dd5dc275d313e7d`
- The compiler was not rebuilt or modified. Both hashes match the supplied values.

## Execution note

No new JS/TS/Python file was created and no Node executable ran. One inline `python3` command was used to insert the discarded-setup paragraph in this report.

## Residual compiler probe

Case 1 (`syntax: tcp-server`) ran once with the original timeout behavior. It ended with `CASE1_RC=124` after 120 seconds. Immediately afterward, `ps` showed one remaining compiler in that case's process group:

```text
566645  566640 R  02:00  99.3 stage2-compiler <가림>
```

`RESIDUAL_PROCESS_COUNT=1`, `RESIDUAL_COMPILER_COUNT=1`; the probe then terminated only its own process group and recorded `PROBE_GROUP_REMAINING_AFTER_CLEANUP=0`.

## Discarded setup attempts

Several setup attempts overlapped external heavy compiles and were not counted as test results. The probe attempts in process groups `533373`, `541108`, and `554271` were terminated only within their own groups; the valid case-1 probe is reported above. A gate wrapper attempt that had not started a case was stopped while it waited for another compile. A separate overlapping case-1 startup was also stopped within its own process group after an external `cc1` appeared. Those attempts are excluded from the 71-case result. No external process was signaled.

## Gate execution conditions

At the successful full-gate start, `uptime` printed:

```text
01:18:56 up 2:37, 3 users, load average: 4.73, 5.46, 5.98
```

The top-15 `ps` snapshot was taken. The concurrent-heavy preflight found no `stage2-compiler`, `native-release-gate.sh`, or `gcc`/`cc1` process above 50% CPU at that instant. The full gate ran under `flock -o` using a lock file under this run's `<가림>` directory. `TMPDIR` also pointed under that directory. Temporary gate-copy paths that explicitly named `/tmp` were redirected into the run directory; test assertions and the 120-second timeout were unchanged. No Node executable ran; case 9 requested `node`, which the runner's safe `PATH` does not provide.

Other runs started heavy compiles during the long gate. The runner waited between cases while an external compiler was active. No external process was stopped or signaled.

## Results

The gate completed all 71 cases without an interruption:

```text
NATIVE_GATE_PASS=4 FAIL=67 TIMEOUT=39 SKIP=0 TOTAL=71
```

The four passes were cases 2, 40, 49, and 51. All 39 timeout cases returned `exit=124`. Every case ran in its own session/process group. The cleanup log has 71 group rows, `before_one=39`, and `after_zero=71`; no case left a process in its group after cleanup.

| Case | Result | Exit | Elapsed (ms) | Test |
|---:|---|---:|---:|---|
| 1 | FAIL | 124 | 120005 | syntax: tcp-server |
| 2 | PASS | 0 | 8013 | syntax: afj-db95 |
| 3 | FAIL | 1 | 12266 | raw binary boundary |
| 4 | FAIL | 1 | 27628 | MariaDB packet codec |
| 5 | FAIL | 1 | 27773 | MariaDB charset contract |
| 6 | FAIL | 124 | 120005 | COLLATE contract |
| 7 | FAIL | 1 | 22849 | page/WAL smoke |
| 8 | FAIL | 124 | 120005 | metrics contract |
| 9 | FAIL | 127 | 122 | local canary rollback |
| 10 | FAIL | 124 | 120005 | large corpus |
| 11 | FAIL | 124 | 120005 | TCP catalog pages |
| 12 | FAIL | 124 | 120005 | TCP lazy catalog eviction |
| 13 | FAIL | 1 | 25263 | persistent recursive B+Tree |
| 14 | FAIL | 124 | 120005 | bounded B+Tree range |
| 15 | FAIL | 124 | 120005 | compound unique index |
| 16 | FAIL | 124 | 120005 | compound predicate range |
| 17 | FAIL | 124 | 120005 | schema MVCC merge |
| 18 | FAIL | 124 | 120005 | composite type order |
| 19 | FAIL | 124 | 120005 | composite NULL order |
| 20 | FAIL | 124 | 120005 | schema-aware composite order |
| 21 | FAIL | 124 | 120005 | mixed declared composite types |
| 22 | FAIL | 124 | 120005 | composite reopen rebuild |
| 23 | FAIL | 124 | 120005 | schema-aware composite range |
| 24 | FAIL | 1 | 769 | TCP concurrency |
| 25 | FAIL | 1 | 856 | TCP 5m performance envelope |
| 26 | FAIL | 1 | 21797 | backup smoke |
| 27 | FAIL | 1 | 21119 | backup restore soak |
| 28 | FAIL | 1 | 12555 | compatibility scorecard |
| 29 | FAIL | 124 | 120005 | AFJ error token mapping |
| 30 | FAIL | 124 | 120005 | MariaDB remediation regression |
| 31 | FAIL | 124 | 120005 | MariaDB unsupported contract |
| 32 | FAIL | 124 | 120006 | TCP database session scope |
| 33 | FAIL | 124 | 120005 | WAL format |
| 34 | FAIL | 1 | 40 | forced-kill WAL recovery |
| 35 | FAIL | 1 | 868 | TCP transaction forced-kill recovery |
| 36 | FAIL | 124 | 120005 | transaction isolation |
| 37 | FAIL | 124 | 120006 | isolation levels |
| 38 | FAIL | 124 | 120005 | MVCC conflicts |
| 39 | FAIL | 124 | 120004 | range predicate conflict |
| 40 | PASS | 0 | 13900 | row-version MVCC core |
| 41 | FAIL | 124 | 120005 | row-version WAL recovery |
| 42 | FAIL | 124 | 120006 | row-version catalog checkpoint |
| 43 | FAIL | 124 | 120006 | savepoint |
| 44 | FAIL | 124 | 120005 | deadlock |
| 45 | FAIL | 124 | 120005 | lock timeout |
| 46 | FAIL | 124 | 120005 | prepared JSON adapter |
| 47 | FAIL | 124 | 120005 | request limits |
| 48 | FAIL | 124 | 120005 | slow request timeout |
| 49 | PASS | 0 | 18065 | auth lockout |
| 50 | FAIL | 1 | 18523 | auth lockout reset |
| 51 | PASS | 0 | 17416 | auth state recovery |
| 52 | FAIL | 124 | 120005 | session cleanup |
| 53 | FAIL | 124 | 120005 | session TTL |
| 54 | FAIL | 124 | 120005 | connection rate limit |
| 55 | FAIL | 124 | 120006 | peer IP rate limit |
| 56 | FAIL | 1 | 631 | runtime peer IP rate limit |
| 57 | FAIL | 124 | 120006 | fragmented TCP requests |
| 58 | FAIL | 124 | 120005 | SQL error mapping |
| 59 | FAIL | 1 | 34 | production TLS boundary |
| 60 | FAIL | 1 | 515 | native TLS listener |
| 61 | FAIL | 1 | 691 | native TLS mTLS/reload |
| 62 | FAIL | 1 | 577 | production native TLS integration |
| 63 | FAIL | 1 | 1455 | TLS adapter handshake/reload |
| 64 | FAIL | 1 | 292 | MariaDB wire packet unit |
| 65 | FAIL | 1 | 27136 | MariaDB mysql_native_password SHA-1 |
| 66 | FAIL | 1 | 1070 | MariaDB wire prepared |
| 67 | FAIL | 1 | 856 | MariaDB standard-client smoke |
| 68 | FAIL | 1 | 449 | native MariaDB wire |
| 69 | FAIL | 1 | 742 | native MariaDB auth rejection |
| 70 | FAIL | 1 | 27798 | native MariaDB decoder |
| 71 | FAIL | 1 | 2324 | native MariaDB prepared |

## First error lines from failing cases

The first five failures by case number were cases 1, 3, 4, 5, and 6. Cases 1 and 6 timed out before their worker logs emitted an error line. The first error lines available from the next failing cases are copied below; local paths are redacted.

- Case 1, exit `124`: no worker error line was emitted before timeout.
- Case 3:

  ```text
  Uncaught error: [buf-to-binary] Function not found: buf-to-binary (at line 2, col 0)
  ```

- Case 4:

  ```text
  Uncaught error: [buf-str] Function not found: buf-str (at line 116, col 0)
  ```

- Case 5:

  ```text
  Uncaught error: [buf-str] Function not found: buf-str (at line 116, col 0)
  ```

- Case 6, exit `124`: no worker error line was emitted before timeout.

Additional first error lines from later failures:

- Case 7: `Uncaught error (line 455): [oversized-wal-smoke-test] oversized WAL was accepted (at line 486, col 0)`
- Case 9: `<가림>/.freelang/smoke.sh: line 11: node: command not found`

## Finding

The gate result is `4` passes, below the comparison value `34`. Child-process residue did not explain the low pass count: timeout children remained after the probe, but the per-case runner removed every case's process group, and all 71 post-case counts were zero. This run therefore supports the V-16 branch `(나) 34 미만이면 다른 원인`.

The compiler's four passing cases establish **4/71** passes for this exact gate run and compiler hash. The observed failures include compiler timeouts and ordinary case errors; this report does not attribute them to a language defect without separate evidence.

## Push record

Only this report is included in the commit. The `origin` remote is Forgejo (`fg.dclub.kr`); `github` is GitHub (`github.com`). Both branch heads were `ad0acf04122acfdd5555c5f8c9c0956bf763f42d` before the report commit.

V16_SUMMARY=4/4
