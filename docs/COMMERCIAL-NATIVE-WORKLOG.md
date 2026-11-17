# Node-free FreeLang and DB95 commercial readiness

## Accepted objective

Complete FX, DB95, the frontend and applications against their declared
requirements. First remove Node.js from build and execution paths. Preserve
features, existing assertions, expected values and time limits. Commercial DB
readiness requires independent evidence for data safety, recovery, concurrency,
security, supported client protocols, sustained load and operations.

Work stays on server 191 in isolated worktrees. Original worktrees and seeds
are preserved. Do not install dependencies, use sudo, regenerate seeds, access
servers 23/25, or modify erp, grok-router, test11-hello or review/hermes.
Use the existing verified private curl prefix through environment variables.
Heavy commands require the shared lock and the existing 2 GiB limit.
Actual production deployment remains a separate final approval step.

## Recorded starting evidence

- FX baseline: cbf0e9db7a090a8e653337dfb79056eb29a62208.
- Stage 2 = Stage 3 = Stage 4 generated C:
  6325a0b92053e2d859458057a0e833f93ce42ffece74493a8e1d841f9a601f37.
- DB95 baseline: 1b208c8e8bf400ac7ee24e80d3c217f715ea707f,
  including two pre-existing modified tests, preserved as test input.
- Native gate: 51 PASS, 20 FAIL, 1 timeout included, 71 total, exit 1.
- Cases 36 and 40: PASS in the full gate and in the focused verification.
- Failures: JS not ported 13; Node-dependent shell paths 3; oversized WAL
  2; soak timeout/cleanup 1; compatibility baseline 88% 1.
- The compatibility failure is not merely a missing PASS marker. Existing
  documents require further MVCC, index and durable recovery evidence.
- Existing DB95-193 storage report records a checkpoint recovery failure.
  Older readiness documents describe another execution path and do not
  establish current Native commercial readiness.

## Work areas and completion evidence

| Area | Required evidence | Current state |
|---|---|---|
| FX compiler/runtime | Native bootstrap, fixed point, language/collection/error/IO/resource regressions, zero Node processes | Fixed point verified at baseline; runtime gaps under investigation |
| DB95 | WAL and checkpoint safety, acknowledged data recovery under faults, isolation, native wire/TLS/client tests, sustained load | Full Native gate has 20 failures; checkpoint safety blocker recorded |
| Frontend | Native compiler/checker, pages and islands with preserved functionality, runtime and browser flows | Inventory pending; original frontend worktree is dirty and must be preserved |
| Applications | Every declared app builds and executes its supported flows without Node; local integration and failure cleanup | Inventory pending; existing deployed services are not modified |
| Release/operations | Clean build/artifact hashes, configuration checks, backup/restore and rollback rehearsal, monitoring and security review | Not yet verified |

## Ordered implementation

1. Reproduce the numeric-tag boundary causing the oversized WAL check failure.
   Correct the shared runtime, then rerun the unchanged affected cases.
2. Separate compiler/runtime defects from malformed soak fixture structure.
   Preserve the original soak assertions and 360-second runner limit.
3. Inventory Native equivalents for the 13 JS entries and the 3 shell paths.
   The existing runner unconditionally rejects most JS paths. Prepare concrete
   adapter proposals before any change to protected runners or fixtures.
4. Complete the frontend/app dependency inventory and migrate supported
   execution paths without deleting features or introducing new JS.
5. Implement durable checkpoint generations, explicit IO failure propagation
   and recovery validation against the documented DB95 storage contract.
6. Recheck the fixed point and applicable regressions after changes, then
   validate the complete Native gate, sustained load and release procedures.

Do not infer completion from source counts or old PASS tables. Attach commands,
actual exit codes, hashes and bounded logs to each completed item. New
diagnostics live outside the source repositories until their contract and
placement are reviewed.

## Milestone 1: numeric repeat boundary

The current V11 arithmetic bridge returns integral results as FL_FLOAT.
fl_repeat previously accepted only FL_INT, so (+ limit 1) produced an empty
array instead of the intended oversized WAL input. This reproduced in a
temporary FreeLang probe before the fix (exit 1, computed count lost).

The runtime now accepts finite, nonnegative, integral numeric counts in the
existing uint32 vector length domain. Invalid lengths throw an explicit error.
The immutable result uses one checked data allocation, preventing the silent
empty result and unchecked builder growth in this path.

Recorded verification, outside the repositories:

- Number/count/content/zero/negative/fraction probe: 6/6 PASS, exit 0.
- Native Stage 0 through Stage 4 generation: PASS.
- Stage 2 = Stage 3 = Stage 4: exact byte comparison PASS.
- Original DB95 case 7 page/WAL smoke: exit 0; oversized WAL PASS.
- Original DB95 case 13 persistent B+Tree: exit 0; oversized WAL and
  persistent recursive B+Tree PASS.
- Existing tests, expected values, official runners and the seed were not
  changed. The complete 71-case gate has not been rerun for this milestone.

This closes the two observed WAL test failures. It does not establish crash
durability or resolve the recorded checkpoint corruption recovery blocker.
