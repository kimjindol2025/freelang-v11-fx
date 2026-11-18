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

The user explicitly confirmed the complete scope as FX + DB95 + frontend +
applications, with Node.js removal as the first priority and commercial DB
readiness as a required result.

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
| FX compiler/runtime | Native bootstrap, fixed point, language/collection/error/IO/resource regressions, zero Node processes | Native fixed point/core milestones verified; Native language check integration in progress |
| DB95 | WAL and checkpoint safety, acknowledged data recovery under faults, isolation, native wire/TLS/client tests, sustained load | Full Native gate has 20 failures; checkpoint safety blocker recorded |
| Frontend | Native compiler/checker, pages and islands with preserved functionality, runtime and browser flows | Native CLI/TUI, unit and page-check paths verified; default/test/web migration remains |
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

## Milestone 2: frontend Native checker entry

The clean frontend FE18 baseline b371d5abb747c41a1bccc60bfa96e30954399aca
was preserved. A separate codex/native-frontend-start worktree contains the
new scripts/fl-check-native product entry and its usage/evidence document.
The existing checker, fixtures, Node launcher and protected test runners
were not modified.

- Frontend commit: 3c728a8d5fa5b51294cb6353383a36292ee34d04, GitHub ref verified.
- Native checker build and default page check: PASS.
- Existing checker fixture matrix: 32/32, including expected diagnostic
  classes and the documented JSON-props timing bound.
- execve traces: zero Node/npm/npx/bun/deno invocations.
- Missing binary/protocol: explicit exit 2. Native failure: exit preserved.
- New JS files and JS line changes: zero.

The complete website is not yet Node-free: the web builder, dev server,
default launchers and test execution path remain on the dependency list.
Initial inventory found 20 FX server entry roots. Static dependency mentions
are candidates, not proof that a runtime path executes Node.

## Milestone 3: Native assertion and shell contracts

The original frontend tests compiled to C but stopped at Function not found:
is=. Native compilation now handles deftest/describe/it, is/is=, run-tests
and test-summary using the contracts in the existing bootstrap.js source.
The reference was read without executing Node. Test bodies run in order under
a case-level exception boundary. Failures stop that case, increment the real
failure count and allow later cases to run. Assertions retain argument order,
JSON equality, return values and truthiness. Invalid is= arity fails before
argument evaluation. Surplus arguments retain the source evaluation rules.

Summary counts are snapshots. Their error array remains shared with the
current context, and earlier arrays stay valid after run-tests resets that
context. A summary before any case has a detached empty array. The generated
case boundary initializes its context before evaluating the case name.
Reports are isolated per thread; owned error records are released at arena
cleanup, including when a nested temporary arena has already ended. Throw
line tracking is also thread-local. C provides the exception/resource bridge;
the compiler implementation remains FreeLang.

The boundary refinement exposed a pre-existing repeat code-generation order
defect. A real effect probe observed [value,count] instead of [count,value].
The same unsequenced C function arguments also changed temporary ID order
between Stage 2 and Stage 3, causing one exact-comparison failure. repeat now
uses the existing sequential argument-emission helper before calling the
runtime. This fixes both the language effect order and deterministic code
generation; the seed and byte-comparison criteria remain unchanged.

The existing provider tests then exposed two masking failures and a missing
shell-exec-result primitive. Native shell-exec and shell-exec-result now use
POSIX process spawning, separate stdout/stderr pipes, EOF on stdin, UTF-8
decoding, real exit status and child reaping. The existing reference's one-MiB
limit applies to each stream; overflow or signal termination returns exit -1
and ok=false. This is the inherited spawnSync contract, documented in the
[official Node child-process reference](https://nodejs.org/api/child_process.html#child_processspawnsynccommand-args-options).
The implementation does not invoke Node. The frontend uses the documented
text-first regex_replace module API at its six redaction calls, preserving
the masking assertions and the distinct legacy regex-replace signature.

Verification records are retained under evidence identifier
native-test-forms.idxwvm in a private directory (<가림>). Existing frontend
test files, FX regression fixtures, expected outputs, protected runners and
the checker were not changed. New JS files and JS line changes are zero.

The diagnostic probes deliberately produce real failed assertions and check
their exact counts, early body termination, later-case continuation, summary
aliasing, context reset and invalid-arity evaluation order. They are not
counted as an all-passing product suite. Separate shell probes cover success,
failure, signals, independent large streams, the output limit, invalid UTF-8,
NUL bytes and stdin EOF. C probes check descriptor stability across 100
commands, closed standard descriptors, child reaping, nested-arena allocation
cleanup and independent thread contexts. These checks are scoped resource
diagnostics, not an ASan/Valgrind or complete leak-suite result.

Final source verification (2026-11-18 KST):

| Check | Actual result |
|---|---|
| Native Stage 0 through Stage 4 build/generation | Exit 0 |
| Exact Stage 2 = Stage 3 = Stage 4 C comparison | PASS, SHA-256 65fc56c092629248471828a88859237d4a2231de344e05213df7fe0e52f96cc2 |
| Verified Stage 3 compiler ELF SHA-256 | 44379c85c41071c0c7822a8710973ab0a827740b9041f4114767cdb6fbf06546 |
| Stage 2 generated C size | 1,492,756 bytes |
| Stage 2 to 3 generation | 20.19 seconds, peak RSS 1,201,256 KiB, exit 0 |
| Stage 3 to 4 generation | 20.11 seconds, peak RSS 1,200,564 KiB, exit 0 |
| Original frontend unit suite | 7 files, 71/71, each exit 0 |
| Original FX tail/scope/error/order regression fixtures | 11/11, expected outputs identical, each exit 0 |
| Native checker | 32/32 existing fixtures and default pages, expected diagnostic classes, exit 0 for the validation batch |
| JSON props checker bound | 0.07 seconds, below the original one-second bound |
| Forbidden Node/npm/npx/bun/deno executions | 0 in the frontend unit, checker and compiler generation traces |
| Original DB95 cases 7 / 13 / 36 / 40 | Each exit 0 under the original 120-second case bound; transaction isolation and MVCC PASS |
| Shell contract probes | 9/9, exit 0 |
| Negative assertion / summary / arity / repeat-order diagnostics | Expected failures and exact real counts verified; diagnostic processes exit 0 |
| Allocation/thread and descriptor/child cleanup probes | Exit 0 |
| Existing tests, expected outputs, protected runners, JS | Unchanged |

The bootstrap/stage0.c input SHA-256 is unchanged before/after:
0101cb63a86f20b93451f70b88a0b577f9548ded0a8b328ebdeedb7e8e4fd767.
No seed was replaced or regenerated. The protected official verification
script still pins an older f7425bf0 input; it is NOT_RUN for this source,
not reported as passing. The separate evidence harness uses the same Native
GCC/runtime recipe and performs the actual Stage 0→1→2→3→4 generation and
exact comparisons without changing that script or its criteria. All heavy
work used the shared lock, two-GiB virtual-memory cap and existing private
curl through process environment variables. No installation was performed.

The original complete Native DB95 gate remains the recorded 51/71 baseline.
Cases 7, 13, 36 and 40 have now been rerun after the runtime changes. Do not
advertise a new complete-gate count until the complete gate is executed again.

The default frontend test launcher still uses Node. The 71 original unit
cases are also compiled and executed individually as Native binaries in the
evidence harness. A supported Native test entry, the web builder/dev server
and application flows remain migration work. Current Node parity and the
full fl-tools adapter checks are NOT_RUN because their execution paths invoke
Node. No blanket frontend or commercial-readiness claim follows from unit
results.

## Milestone 4: Native CLI execution

The src/main.fl CLI compiled and ran help/status, but real add failed at
str-replace-all. Both inspected reference runtimes define this as the same
literal split/join operation as str-replace. The Native registry now exposes
that alias through the existing implementation, without changing prior
opcode positions or adding a second replacement algorithm. A stale partial
archive initially retained the old dispatch; rebuilding its actual owner,
core.c, corrected the artifact. Both the failed log and successful checks are
preserved. No failing probe was reported as passing based only on exit 0.

The frontend adds scripts/fl-git-native with explicit verified CLI/TUI binary
selection, status-marker validation and no Node fallback. Its context command
also replaces an unimplemented doall wrapper with a doseq loop, preserving all
seven printed cards. The existing default launcher and tests remain unchanged.

- Literal replacement probes: 7/7, including defaults, literal replacement
  tokens, non-overlap, empty search, Unicode and NUL bytes.
- Native CLI integration: 22/22 in a new private fixture repository with
  synthetic local identity and no remotes. Real add/commit/stage/unstage,
  branch creation/deletion and guards, reads, failed Git status propagation,
  text/JSON context and launcher rejection cases passed. Quoted filenames and
  commit messages preserve apostrophes, spaces, Korean and emoji.
- Text context includes cards 01 through 07; JSON contains seven cards and
  seven available keys and reports the actual fixture branch.
- Latest runtime/compiler original frontend suite: 71/71, exit 0, no FAILED
  summaries and empty stderr in all seven files.
- Fresh Stage 2 = Stage 3 = Stage 4 C comparisons: PASS; SHA remains
  65fc56c092629248471828a88859237d4a2231de344e05213df7fe0e52f96cc2.
- Latest compiler ELF SHA-256:
  28d6b736f3ccb8f808eccfe4afb2087329aceed2c75e7cc61683276638bf3836.
- Native CLI ELF SHA-256:
  e9dbd8355c1e97fe7b520b08bea1f0b52c77a518a01231c4792b859ad3bf457a.
- Stage 2→3 / 3→4 generation: 24.99 / 24.13 seconds; peak RSS
  1,201,292 / 1,201,416 KiB; both exit 0 under the unchanged two-GiB cap.
- CLI integration and unit process traces: zero Node/npm/npx/bun/deno.
- Existing tests/runners/expectations, JS and bootstrap input are unchanged.
  No installation or global configuration change occurred. The original
  bootstrap verifier remains NOT_RUN for its older pinned input as explained
  above. Bash syntax and diff whitespace validation passed; ShellCheck is
  unavailable and was not installed.

This advances the Native CLI path. Native TUI interaction, actual remote
provider operations, default-launcher migration, web build/dev and browser/app
flows remain unverified or pending. The complete DB95 gate was not rerun for
the alias addition; its checkpoint durability blocker remains unresolved.

## Native terminal, literal and checker components (historical checkpoint)

Native raw input and supervised terminal execution are implemented with no
Node fallback. The frontend retains its CLI and TUI functionality through a
separate Native entry. Hexadecimal escapes now emit actual control bytes;
length-aware strings retain NUL; template references resolve through lexical
bindings and closure capture. See NATIVE-TERMINAL-RUNTIME.md.

Three broad terminal/string attempts stopped at distinct actual failures:
an unavailable min dependency in the new prefix selector, the legacy C join
adapter's quoted/NUL-truncating conversion, and an unflushed interactive
prompt. The broad batch was stopped after its third failure. Separate focused
probes established each correction without relaxing assertions or waits.
The 436-byte literal oracle and 152-byte template oracle matched exactly.
Raw PTY/host coverage included 18 restoration/signal/child-reaping markers;
the Enter prompt and menu restoration passed after print/println were fixed
to flush stdout. Typed Unicode input and Git-quoted paths remain pending.

A fresh bootstrap exposed unspecified C argument order in nested push code
generation: Stage 2/3 differed only in generated temporary IDs. Generation
and runtime argument evaluation now use explicit ordered statements, including
ignored extra arguments. The baseline private ordering contract failed 0/3;
the correction passed 3/3. Stage 2 = Stage 3 = Stage 4 then matched exactly
(C d5c538551e081c93b11fa006c704c950c436121cb3cb9cf9278cd033a5f44949).
That intermediate source also passed the original frontend 71/71, CLI 22/22,
FX 11/11, checker 32/32 plus default pages, and DB cases 7/13/36/40. These are
historical results, not a substitute for validation of later source changes.

The preserved frontend scripts/fl-test invokes reference check before run.
Compiling and running 71 assertions does not replace its lexical/parser,
metadata/effect/type and JSON diagnostic behavior. Text and JSON reference
modes differ in pure-effect exit behavior. Existing runners are unchanged.

Three FreeLang modules implement independent checker components but are not
yet connected to a compiler command or the frontend test launcher:

| Module | Component scope | Focused execution |
|---|---|---|
| self/syntax-guard.fl | Delimiters, comments, quoted/escaped strings, positions, Unicode/NUL, 100k literal | 5/5; 0.49 s, 60,552 KiB in the first corrected runtime run |
| self/check-meta.fl | Function metadata and source-ordered effects; normalized control/collection traversal | 7/7 |
| self/check-types.fl | Reference best-effort arity and literal-type diagnostics | 10/10 after value-context correction |

Static reference extraction matched all 68 effect catalogue entries
(SHA-256 c0ee01ee9ac71ae3313e497793ea7d01236711c64f38e3953c988b02c6998c40),
43 arity rules (fa252af23349f3ed7bfb463610e1a52d00f606ff63ffb095b9232f626046199a)
and 35 ordered special forms
(dc972c6c817ad438fd122046df92c05b34647cb95f391bd33b19392d120e258f).
The Native walker also inspects collection-contained calls. The reference
walker lacks a general array branch; current zero-difference parity is not
claimed. These heuristics do not prove typed program code generation or
formal type safety.

The components exposed two further compiler defects: flat cond in loop/recur
was silently emitted as nil, and a non-tail if inside a collection or condition
could store an internal tail-call request instead of a completed result.
Flat recur-aware clauses now emit their actual branches; consumed values,
conditions and recursive-call arguments explicitly use value scopes. A private
map/vector/nested-condition contract moved from 0/3 to 3/3.

The 100k-character syntax contract also exposed quadratic immutable appends
inside str-split. The runtime now counts fields and fills one unpublished
result vector, checking size/allocation and releasing decoded units on error.
Public immutable push semantics are unchanged. The original syntax contract
passed 5/5; a freshly built 21-object runtime passed it again and passed the
six-case split/Unicode/NUL/large-input contract. The first new split diagnostic
incorrectly asked the legacy byte-index char-code-at API for UTF-16 units;
its 5/6 result is retained. A separate oracle checks actual JSON surrogate
encoding and round-trip text. One diagnostic accidentally linked an older
archive; those results are excluded. Fresh archives now retain object hashes
and verify them before linking. No protected test/expectation/timeout changed.

The final full Native bootstrap and core regressions passed for the latest
value/split source. Exact Stage 2 = Stage 3 = Stage 4 C SHA-256 is
69ec33d69ea93079296fa5723cc84e4b5b50e4466a6bd43d379385056bc72999;
Stage 3 ELF is
54d85380a55275e5ecce727e2e491e8acfb5fe015d701f2cc21ae0a217bf1d88.
Stage 2 to 3 generation took 27.77 s / 1,141,916 KiB; Stage 3 to 4 took
28.76 s / 1,139,816 KiB, each exit 0. Fresh frontend 71/71, CLI 22/22,
FX 11/11, checker 32/32 plus default pages and DB cases 7/13/36/40 passed.
The newly generated value, push, syntax and split contracts passed, with
436/152 literal/template bytes matching their independent expected output.
A dedicated fresh terminal pass exercised raw capability/ownership, character
conversion 4/4, all 18 host restoration/reaping markers and Enter/menu
restoration, exit 0. Collected core/terminal traces contain zero Node-family
executions. Final metadata/type component verification passed 7/7 and 10/10, exit 0; the original syntax component also passed 5/5 with that compiler/runtime. These are component results, not an integrated Native check command.

Receipts: native-core-value-split.gzQgDG, native-terminal-final.bv22g0 and native-check-components-current.tMIa17.
The seed, protected tests/runners and JS are unchanged; paths/owners are
<가림>. Temporary curl dependency packaging, complete Native check command
integration, typed Unicode TUI input and browser/app/default-launcher
migration remain pending. Current Node comparison is NOT_RUN; no zero-diff
claim is made. The complete DB95 gate is still the recorded 51 PASS / 20 FAIL
baseline; checkpoint fault recovery and commercial readiness remain HOLD.

## Native language check command (initial integration receipt)

The compiler source now loads the verified FreeLang checker components and
routes --check <file> [--json] through them. A small C IO bridge writes UTF-8
diagnostics to stderr; it does not add a dependency. Sources are inspected
without executing their bodies. Lexical/file errors, delimiter stacks,
source positions, metadata/effects and type diagnostics use the reference
freelang.diagnostic.v1 payload shape.

A read-only reference capture used existing Node v22.19.0, solely for
compatibility diagnostics rather than a compiler build stage. Reference
bootstrap SHA-256:
2827f82534025ea18e06050dd56dfeefbd176c71f5eac887d4edd10d0e900052.
All 54 source-check invocations left the reference/inputs unchanged, made
zero network request syscalls and did not execute a file-write expression
embedded in an inspected source. No JS source was created or edited.

The first Native assembly matched 16/26 JSON payloads. A linked token-grammar
preflight then rejected five malformed inputs previously accepted by the
legacy parser and reproduced four delimiter diagnostic records, including
UTF-16 columns, exactly. Native assembly then matched 25/26 JSON payloads.

The first integrated Native command captured 54 executions: 26/27 JSON
payloads and 53/54 exit codes matched the frozen reference. The one semantic
difference is explicit: a println inside an array in a declared pure
function is missed by the reference effect walker and detected by Native.
The Native JSON result is an error; both text modes exit 0 under the legacy
text policy. This is not zero-difference parity. The existing Node checker,
tests, expectations and runners were not changed to hide that result.

Normal, metadata, pure, arity and type text output matched byte for byte in
that command pass. Parser/lexer error text initially differed; its source
line/caret/hint renderer has now been ported and awaits fresh validation.
The integrated compiler's first complete fresh Native fixed-point build is
running with the unchanged seed, shared lock and 2 GiB cap. No new integrated
compiler fixed-point hash or commercial-readiness PASS is claimed yet.

Receipts: native-check-reference.ncFCVI, native-check-assembly.ZmlToB,
native-check-assembly-v2.25bgSh, native-check-command.tfB0Jm and
native-check-fixedpoint.6o1yXU. Paths and owners: <가림>.

## Remaining frontend Node paths (source inventory)

The frontend tracks ten JS/MJS/CJS files. The separate Native CLI/TUI and
page checker have verified execution paths; the protected default entries
remain unchanged. scripts/fl-test still performs reference check then run,
and scripts/provider-gate still invokes Node. The separate scripts/fl-test-native now preserves that first check phase
using the verified Native --check command; the default runner is unchanged.

The web entry still invokes web/fl-front-build.js (2,937 lines) and the
bundled web/runtime/bootstrap.js. The development entry invokes
web/dev-server.js (665 lines), with watch/rebuild, SSE reload, build/error
state and server lifecycle handling. The builder also imports the style
compiler (312 lines), island compiler (582), diagnostic catalogue (220),
and references an optional fix module whose path requires investigation.
These are source counts, not completed migration or test counts.

The existing FX --browser-js target and embedded FreeLang browser-core
provide an available backend for a Node-free island build. They do not
replace the remaining frontend builder/dev server by themselves. Browser
build/render/hydration/interaction parity, default entry migration and
complete app inventory remain pending. No feature is deleted to claim
Node removal or a passing build.

## Native check and test entry: completed checkpoint

The integrated Native --check command and separate frontend
scripts/fl-test-native entry now have fresh receipts. See
NATIVE-LANGUAGE-CHECK.md for diagnostic behavior, exact comparison counts
and the intentional pure-array difference. Earlier pending statements above
describe historical checkpoints, not the current command.

| Current verification | Result |
|---|---|
| Fresh Stage 2 = Stage 3 = Stage 4 C | Exact bytes, exit 0 |
| Stage 2 to 3 / Stage 3 to 4 | 38.80 / 39.58 seconds |
| Generation peak RSS | 1,514,012 / 1,515,368 KiB; unchanged 2 GiB cap |
| Existing frontend test checks, JSON and text | 14/14 byte-identical stdout/stderr and exit |
| Synthetic/missing-file JSON payloads | 26/27 identical |
| Synthetic/missing-file text stdout/stderr | 26/27 byte-identical |
| Synthetic/missing-file exits | 53/54 identical |
| New Native test entry, original tests | 71/71; all seven files checked before compilation/run |
| Native entry negative/discovery controls | 7/7; actual failures remain failures |
| Fresh core regressions | frontend 71/71, CLI 22/22, FX 11/11, page checker 32/32 plus defaults |
| Original DB95 cases 7, 13, 36, 40 | All exit 0; not the complete DB gate |
| Fresh terminal coverage | raw capability/ownership, character 4/4, 18 host markers, Enter/menu restore; exit 0 |
| Collected Native traces | zero Node-family executions and zero test-entry/check network requests |

Current generated C SHA-256:
4e589203e96b1cbfa8e4983439df33ab9a471daa3c95f5d4d89f99573c139c7e.
Current Stage 3 ELF SHA-256:
24355a87f854de32775c65773663f32050f4cc5c0b2e156b964966ea356f7fa3.
Seed before/after remains
0101cb63a86f20b93451f70b88a0b577f9548ded0a8b328ebdeedb7e8e4fd767.
The unchanged official verifier with its older pinned seed is NOT_RUN.

The seven test-entry controls cover a failed assertion, missing summary,
malformed source, unsupported Native block, empty discovery, missing compiler
and unreadable discovery. Syntax/unsupported generation failures invoke no
GCC. Empty discovery reports NO_TESTS; unavailable inputs report BLOCKED.
The new entry adds no product timeout. The protected old entry, existing
tests, expected values, limits, JS and seeds remain unchanged.

One private receipt logger wrote literal backslash-n record separators.
The core driver had completed successfully; its raw log was preserved.
A read-only completion receipt independently checks the exact raw suffix,
source manifests, DB case exits and empty execution/generation audits.
The first downstream harness therefore stopped at its receipt gate without
executing tests; the corrected harness consumed that verified receipt.
No product output or assertion was normalized to obtain a pass.

Receipts: native-check-fixedpoint.6o1yXU,
native-check-command-final.nSZaNo, native-check-core-final.dtc39N,
native-test-launcher-v2.Gs7UX1 and native-terminal-check-build.ZgoNCk.
Paths and owners: <가림>.

The stricter pure-array diagnostic is retained; universal zero-difference
parity is not claimed. Web builder/dev server, browser/app flows, default
entry migration, Unicode TUI input and production dependency packaging remain
pending. The full DB95 gate is NOT_RERUN, with its recorded 51 PASS / 20 FAIL
baseline and checkpoint fault-recovery blocker. Commercial readiness is HOLD.

Final source hygiene removed only extra blank lines at EOF from the two new
FreeLang modules. A fresh Native generation from those final bytes matched
the recorded Stage 4 C exactly, exit 0 (38.75 s, 1,515,944 KiB). Its full
source manifest was unchanged and its trace contained zero Node executions.
Receipt: native-check-final-source.i3wziqo8.

## Native AST and island boundary checkpoint

The new --parse-json interchange, separate island-h/island-mount browser
boundary and fresh bootstrap are documented in NATIVE-AST-INTERCHANGE.md.
Stage 2 = 3 = 4 exact C SHA-256 is
23ce0a1c2c08b0fb2283c2204ae727a4a61e0c4af190d741401da02534bf4466;
compiler ELF SHA-256 is
352cac840529249a5fd22e2c127e20467250f5cc55d80bedcdcf71c9189ec0f7.
Generation took 38.43 / 37.76 s with 1,527,452 / 1,528,424 KiB peak RSS.
The older fixed seed first rejected the new JSON alias at Stage 1; using the
existing supported spelling fixed the build without changing the seed.

Fresh AST coverage passed 18/18, including eight real island files after
five malformed required-attribute entries in two frontend files were fixed.
Bodies and load targets were not executed/read. Existing browser generation
passed 7/7, reproducibility 7/7 and compiler rejections 10/10.
Fresh core passed frontend 71/71, CLI 22/22, FX 11/11, checker 32/32 plus
defaults and DB cases 7/13/36/40; Native execution traces contain no Node.

Browser execution is NOT_RUN: Browser/Playwright were unavailable, Chrome
bridge connection failed and a capped private Chrome startup exited 133.
No memory limit was changed. The frontend's initial Native generation
inventory is 2/8, with six explicit missing-feature failures. This is not a
completed frontend migration, a passing physical UI or DB commercial approval.
The full DB gate remains NOT_RERUN and checkpoint fault recovery remains HOLD.

## Native island connector checkpoint

All eight real client modules now generate through the Native AST/frontend/
browser pipeline. Fixed grammar host calls support the FreeLang request,
form, string and immutable-map kernel. A malformed frontend index if was
corrected by parentheses only; all 7,135 AST nodes/operations/literals remain.
No authored JS, seed, protected runner, existing test or expected value changed.

Fresh bootstrap Stage 2 = 3 = 4 retains C SHA
23ce0a1c2c08b0fb2283c2204ae727a4a61e0c4af190d741401da02534bf4466.
ELF SHA is 80b5d9e911eb71fd5d298b8516b73ae09f3838c96f7c0ddee27f0c4e8b35b9e8.
Elapsed generation 40.42 / 40.13 s missed the existing 40-second target;
peak RSS 1,525,712 / 1,529,112 KiB stayed within the unchanged 2 GiB cap.
No timing-only rerun was used.

Current evidence: 26 frontend controls, eight real generation cases,
seven string round trips, three rejected AST inputs and 38 Native pure-kernel
model checks pass. Original browser scenarios generate reproducibly 7/7 and
reject forbidden inputs 10/10. Fresh core passes frontend 71/71, CLI 22/22,
FX 11/11, checker 32/32 plus defaults and DB95 7/13/36/40. No Node-family
execution was observed in collected Native traces. The failed diagnostic
models and their host/fixture corrections are retained in the linked report;
they are not physical browser proof.

See NATIVE-AST-INTERCHANGE.md and the frontend NATIVE-ISLAND-COMPILER.md for
contracts, receipt IDs and limits. Browser startup remains blocked under the
cap; DOM, HTTP, forms, focus and complete application behavior are NOT_RUN.
Full Native web/default migration and official dependency packaging remain.
Full DB95 71 is NOT_RERUN, with the recorded 51 PASS / 20 FAIL baseline and
checkpoint fault recovery blocker. Commercial readiness HOLD; deployment NO.

## Compiler error propagation and explicit Native HTTP binding

Unsupported IR generation now throws instead of producing a nil placeholder;
the validator propagates failure and the command checks it before writing C.
Private CLI controls reject malformed input/imports while preserving existing
output. HTTP/TLS accepts process-local FL_HTTP_HOST selection; invalid values
are rejected before startup effects. A real loopback health response passes,
and a bind observer verifies the compatibility default without an actual
non-loopback listener.

Final strict-preflight bootstrap receipt native-web-fixedpoint-v3.umdne80t
has Stage 2 = 3 = 4 C SHA
c1f48e888960f21679278d4e90ce0d2e05d3e51bbfecefafcd212819905458fa.
ELF SHA is 5fe1a1cf642c5fd244abe18a9d013979e7a6ed522940973afa764c6a8a0022a7.
Generation took 39.68 / 37.80 seconds and 1,524,352 / 1,530,188 KiB peak RSS.
Seed and 2 GiB cap are unchanged. An initial delimiter error was rejected
by strict preflight even though the permissive compiler reached a fixed
point; that earlier source receipt is not accepted.

Fresh core native-web-core.1t1ve_e_ passes frontend 71/71, CLI 22/22, FX
11/11, checker 32/32 plus defaults and DB95 7/13/36/40. No Node execution is
observed in collected core traces. See NATIVE-COMPILER-HTTP-SAFETY.md for the
boundary controls and remaining syntax/TLS/packaging limits. The frontend's
new Native web path has generated nine route handlers, eight browser modules
and a linked Native server without Node; full HTTP/browser acceptance is
tracked separately. DB95 checkpoint recovery and the full DB gate remain
unfinished. Commercial readiness HOLD.
