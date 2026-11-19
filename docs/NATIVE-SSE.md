# Native Server-Sent Events

Named SSE handlers now resolve through the same FreeLang function registry as
HTTP handlers before trying a legacy exported C symbol. Captured closures and
names registered before their definitions work. A missing named handler returns
HTTP 500 before a streaming header is sent.

For an HTTP stream, HTTP transfers the socket explicitly to a tracked SSE task.
The task closes it after the handler returns, completes its Native thread/arena
cleanup and then reports that it has drained. Ordinary HTTP workers remain
available for health requests and broadcasts while streams stay open. The
synchronous C entry retains its caller-owned descriptor contract.
SSE connections use process-local monotonic lifetime IDs with synchronized
lookup, sending and release. A close makes subsequent alive/send calls false.
Shutdown closes the transport direction and makes polling handlers leave before
HTTP joins its workers. Stream tasks are bounded at 256; stopping, exhausted
capacity and thread-creation failures return 503 without transferring ownership.
Route entries are prepared before publication under a mutex. The existing
16-route limit remains.

Each send emits one complete event, including an empty event, arbitrary-length
payloads and multiple data lines. CRLF, CR and LF split lines consistently.
Short successful writes and EINTR continue until the frame has been sent; a
transport failure marks the connection closed. Closed handles are removed after
the handler finishes, before its owner closes and can reuse the descriptor.

The compiler adapts server_sse, sse_send, sse_close and sse_alive (including their
hyphen aliases) to ordinary function values when they are passed as data.
The same signature adaptation covers fn?, used to inspect these values.
Wrappers check arity and leave the native operator table and closure ordinals
unchanged. Lexical function bindings keep their existing behavior.

## Earlier accepted SDK verification

Paths/owners: <가림>. Receipt: native-sse-firstclass-20261120.
The fresh compiler and full runtime were built from the unchanged seed. All
listed checks use that matching pair; the earlier diagnostic candidate is not
the acceptance SDK.

| Check | Result |
|---|---|
| Normal HTTP/SSE, query strings, closures, forward names, empty/Unicode/multiline/131072-byte frames, 24 clients and open-stream shutdown | 74/74 across normal and short-write/EINTR modes |
| Function values in variables, vectors, map/filter, aliases and closed state | 7/7 plus 5/5 language assertions |
| Legacy exported C handlers and 16 concurrent route registrations | 19/19 |
| Frontend Native HMR/inspection service API | 83/83; supervisor/browser/headless acceptance remains open |
| Native recursive source watcher and snapshot/diff | 23/23 |
| Operator lookup / concurrent calls | 2673/2673 / 32768; byte difference 0 |
| Line join | 24/24 |
| Existing frontend assertions / CLI | 71/71 / 22/22 |
| FX regressions / checker fixtures | 11/11 / 32/32 plus default check exit 0 |
| Existing DB language/runtime cases | 7, 13, 36, 40; each exit 0; not the latest whole DB release gate |
| Compiler Stage2 = Stage3 = Stage4 | Exact generated C bytes |
| Stage2 to Stage3 / Stage3 to Stage4 | 29.51 s / 29.40 s; RSS 1539296 / 1539540 KiB |
| Existing generation bounds | Both below 40 s and 1.6 GiB; process cap 2 GiB |

- Stage2/3/4 C SHA-256: d1041a745a1bffbd2d72a2787da14063b4fdb976f61e09a1ddca7176fad866b9
- Compiler ELF SHA-256: e0035ad76e322e41fdd5406e9dc3a494dc44449fdbca7fba511cccf599e18dd7
- Runtime archive SHA-256: 2f0e0810da55436401fc5bd732b06a135c6bbd033695134ad082de15da35897e
- Seed SHA-256, unchanged: 0101cb63a86f20b93451f70b88a0b577f9548ded0a8b328ebdeedb7e8e4fd767

The original SSE baseline returned health 200 and stream 404. Its generated
FreeLang callback was invisible to the former C-symbol-only lookup. The first
Native development-service compilation also exposed bare C function pointers in
higher-order calls. Both failures are preserved. First-class predicate coverage
then exposed the same problem for fn?; no fixture assertion was removed to make
it compile. The candidate and canonical builds both pass the unchanged tests.

An earlier core run included an unrelated eighth frontend suite and passed all
80 assertions, then failed its fixed seven-suite/71-assertion scope guard. The
canonical core run uses a byte-preserved tracked checkout and the unchanged
seven-suite guard. The current working-tree common check separately includes
all eight suites. No protected test, expectation, time bound, runner or seed was
edited. The approved existing curl inputs were supplied through environment
variables. There was no dependency installation, Node compiler/build stage or
new handwritten JavaScript.

This implementation streams ordinary HTTP only; the HTTP layer currently does
not upgrade TLS connections to SSE. A handler that never returns or polls
connection state can still prevent graceful shutdown. A complete long-lived
application memory/throughput bound is not established by these scoped checks.
Native development supervision, browser HMR/state restoration, headless island
execution, all-app acceptance and DB commercial qualification remain open.
Commercial status stays HOLD. No production deployment was performed.

## Subsequent lifecycle checks

The stream handoff above corrects a reproduced starvation defect: two retained
open streams with FL_WORKERS=2 prevented ordinary health and broadcast requests
from completing. The isolated current-runtime candidate passes 34/34 controls
with eight open streams and two HTTP workers, including delivery, connection
cleanup, and thread/file-descriptor drain. The unchanged 74 SSE, 83 development
API, 7 function-value plus 5 language and 19 C/concurrent-route controls pass too.

A separate ordinary HTTP keep-alive connection also prevented shutdown within
the existing three-second observation bound. Empty receive waits now observe
the stop flag at most 100 ms apart; the existing monotonic header/body deadlines
remain unchanged. Buffered requests and running handlers can complete. The
combined candidate passes 56/56 lifecycle controls covering one/eight idle
connections, a running handler, an accepted queued request and partial headers,
then passes the same stream/API regression controls above. TLS handshakes and
noncooperative user handlers are outside these lifecycle receipts.

Receipts are native-sse-capacity-20261120 and native-http-idle-20261120.
The new coherent seed-to-Stage4/runtime/core/SDK verification passed as
native-dev-coherent-20261120. Its additional checks repeat the 34 capacity and
56 HTTP lifecycle controls on the newly packaged SDK. The older SDK hashes
above do not identify this current checkpoint. No protected assertion, source
fixture, deadline, runner or seed was edited.

## Current coherent checkpoint

[Machine-readable evidence](evidence/native-dev-coherent-20261120.json) records
the exact compiled-source changes and distinct acceptance scopes.

- Stage2 = Stage3 = Stage4 C SHA-256:
  26f07fec1123c4b7bd396b68fd6272a7d9882e513e70f747d03fe206fe13d2c0.
- Stage-3 compiler ELF: 16f1772378b93765dea9bd1c6149e48130324667db730bf98d5090627d131586.
- Fresh runtime archive: 4d3140faaa9623c9d11a7d263ae956774c4d88828c53cfda68771f3e46d4473b.
- Stage2/3 generation: 29.88/29.31 seconds, RSS 1545824/1545804 KiB;
  unchanged 40-second/1.6-GiB bounds and 2-GiB process cap pass.
- Fresh original core, CLI, checker, SSE/API/function/C-route and watcher
  regressions pass. The canonical tracked frontend cohort is 71/71; the actual
  working-tree cohort is separately 80/80, preserving its existing extra suite.
- One stream remained open for 180 seconds with periodic ordinary HTTP:
  41/41 checks, warm RSS growth 216 KiB, open FD growth 0 and normal shutdown.
  This is a measured scoped observation, not a general lifetime/load bound.
- The relocated SDK passes real default common/Provider/page entries,
  library resolution, exact self-generated C and manifest tamper/restore checks.

Actual Chrome HMR/store/CSS/error behavior passes 23 functional controls;
screenshot and pointer hit-testing remain unverified. Development supervision,
complete diagnostics/metadata, headless execution, deployment migration, actual
application flows and whole DB commercial qualification are still required.
Commercial HOLD, deployment NO, overall goal active.
