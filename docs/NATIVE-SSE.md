# Native Server-Sent Events

Named SSE handlers now resolve through the same FreeLang function registry as
HTTP handlers before trying a legacy exported C symbol. Captured closures and
names registered before their definitions work. A missing named handler returns
HTTP 500 before a streaming header is sent.

HTTP retains ownership of the socket and closes it after the handler returns.
SSE connections use process-local monotonic lifetime IDs with synchronized
lookup, sending and release. A close makes subsequent alive/send calls false.
Shutdown closes the transport direction and makes polling handlers leave before
HTTP joins its workers. Route entries are prepared before publication under a
mutex. The existing 16-route limit remains.

Each send emits one complete event, including an empty event, arbitrary-length
payloads and multiple data lines. CRLF, CR and LF split lines consistently.
Short successful writes and EINTR continue until the frame has been sent; a
transport failure marks the connection closed. Closed handles are removed after
the handler finishes, before HTTP can reuse the descriptor.

The compiler adapts server_sse, sse_send, sse_close and sse_alive (including their
hyphen aliases) to ordinary function values when they are passed as data.
The same signature adaptation covers fn?, used to inspect these values.
Wrappers check arity and leave the native operator table and closure ordinals
unchanged. Lexical function bindings keep their existing behavior.

## Verification

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
