# Native AST interchange and island boundary

The compiler provides a non-executing source interchange for Native frontend
tools:

~~~sh
"$FREELANG_NATIVE_COMPILER" --parse-json client.flx
~~~

Success prints one JSON object with schema freelang.ast.v1, status ok, the
absolute source file and a nodes array. It performs lexical/structural
preflight before parsing, retains source lines and does not execute bodies or
follow load directives. Empty source returns an empty array. Invalid argument
count exits 2; read/syntax failures return the existing diagnostic shape and
exit 1. No Node runtime is invoked or searched for.

Consumers must distinguish this interchange from the legacy --ast debug
printer. Number literals retain token text, for example a node with kind
literal, type number and value "12.5". Arrays/maps are block nodes with type
Array/Map and fields.items. Function/island forms are sexpr nodes; normalized
and/or and other parser kinds retain their own kind. The schema does not
promise support for every raw kind in every backend. Unknown/invalid fields
must fail rather than become nil or placeholder code.

JSON string syntax is separate from FreeLang source string syntax. Frontend
emitters must preserve values through supported source escapes or a JSON
data expression; directly copying arbitrary JSON literals into source loses
unsupported escapes. The new frontend normalizer uses a JSON data expression
for isolated UTF-16 surrogate units because UTF-8 file writes intentionally
replace those units. That does not change the file API's encoding policy.

## Island browser primitives

The browser target adds island-h(tag, attributes, children-array) and
island-mount(id, render-function). Their algorithms are FreeLang in
self/browser-core.fl. The C printer adds only fixed host calls for text-node
creation and console error output. Its embedded source is refreshed by the
existing refresh-browser-core.sh procedure; the seed is unchanged.

The island HTML boundary is separate from the existing restricted browser-*
APIs. It supports the frontend's HTML tags/attributes, typed event callbacks,
nested children and text nodes. It retains URL checks. Mounting builds the
result before replacing children and coalesces updates requested during a
render. Errors become text and a diagnostic marker. This is a developing
island boundary: focus/selection preservation, stores/HMR, additional host
APIs and complete application behavior are not accepted yet.

## Verification

| Evidence | Result |
|---|---|
| Fresh Native Stage 2 = Stage 3 = Stage 4 C | exact bytes, exit 0 |
| Stage 2 to 3 / Stage 3 to 4 generation | 38.43 / 37.76 s |
| Peak RSS | 1,527,452 / 1,528,424 KiB, unchanged 2 GiB cap |
| AST command controls plus original island files | 18/18; all 8 real files parse |
| Source body / load directive probes | no body execution, no load-target read |
| Original browser scenarios | 7/7 generated twice with exact byte matches |
| Original browser compiler rejection cases | 10/10; no new output or source mutation |
| Current Native core regression | frontend 71/71, CLI 22/22, FX 11/11 |
| Page checker | 32/32 plus defaults, exit 0 |
| Original DB95 cases 7, 13, 36, 40 | all exit 0; not the complete DB gate |
| Collected Native parser/generation/core traces | 0 Node-family executions |
| Parser/browser-generation network audit | 0 network request syscalls |
| Physical browser execution | NOT_RUN |

The first fresh seed build stopped in Stage 1: the older fixed seed did not
recognize json-str and emitted an undeclared C call. The new module now uses
the already-supported json-stringify spelling. The seed, tests, expectations,
timeouts and protected runners were not modified.

Generated C size: 1,985,216 bytes.
Stage 2/3/4 C SHA-256:
23ce0a1c2c08b0fb2283c2204ae727a4a61e0c4af190d741401da02534bf4466.
Stage 3 compiler ELF SHA-256:
352cac840529249a5fd22e2c127e20467250f5cc55d80bedcdcf71c9189ec0f7.
Seed before/after:
0101cb63a86f20b93451f70b88a0b577f9548ded0a8b328ebdeedb7e8e4fd767.

Receipts: native-island-fixedpoint.m3wumw8j,
native-island-fixedpoint-v2.1z1tjn2x, native-island-parser-final.cq39336f and native-island-core.k7fl8rx_.
The Browser plugin and Python Playwright were unavailable. Chrome bridge
sockets refused connection. A fresh private Chrome startup under the
unchanged 2 GiB virtual-memory cap exited 133; stderr reported only the
monitored process crash, so its precise cause is unproven. It left zero owned
browser processes. Browser identity, DOM, screenshots, console and interaction
checks are NOT_RUN. No dependency was installed or limit raised.

The protected official bootstrap verifier pins an older seed and remains
NOT_RUN; the actual fresh Native stage recipe above supplies the fixed-point
receipt. Production dependency packaging, the remaining Node frontend paths,
complete application coverage and DB95 checkpoint fault recovery remain
pending. The full DB95 gate is NOT_RERUN, with the recorded 51 PASS / 20 FAIL
baseline. Commercial readiness remains HOLD. Host paths/owners: <가림>.
