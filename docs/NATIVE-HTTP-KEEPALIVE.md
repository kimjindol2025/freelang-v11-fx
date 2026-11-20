# Native HTTP keep-alive under worker pressure

Two completed HTTP keep-alive connections could occupy both configured workers
while a third accepted connection waited in the queue. The original three-second
health control timed out. In the actual browser this also prevented
`native-dev.js` from completing, after the independent CSP problem was fixed.

The request reader now releases a completed, idle plain-HTTP connection when
all workers are occupied and another connection is queued. It checks the pool
under its existing mutex, only in the existing 100 ms idle poll, before reading
any byte of the next request. A nonblocking `MSG_PEEK` preserves data that arrives
between the poll and the pressure check. A fresh connection, partial header or
partial body follows its previous path. TLS is excluded from this new branch.

HTTP permits a server to close an idle persistent connection; clients must
handle asynchronous closure. See [RFC 9112, connection management](https://www.rfc-editor.org/rfc/rfc9112.html#section-9.3.1)
and [teardown](https://www.rfc-editor.org/rfc/rfc9112.html#section-9.6).
This change adds no timer and changes no worker, queue, connection, header,
body or memory limit. The existing header budget is 10 seconds, body budget
60 seconds and keep-alive maximum 100 requests.

Receipt `native-http-pressure-20261120` retains the failing original control.
The matching-runtime control responds in about 0.10 seconds with the same two
workers and three-second deadline. Additional raw-socket controls preserve
100 requests on one uncontended socket, a fresh first header, partial reused
headers/bodies, twelve queued concurrent requests, and normal shutdown.
The [evidence](evidence/native-http-pressure-20261120.json) records qualification
and exact source/runtime/compiler hashes.

The runtime was rebuilt from all 21 source objects. Nineteen objects match the
previous archive; `http.o` contains this change and `core.o` differs in Ryu assert
source-location strings after moving the private source snapshot. The only
changed C/header inputs are `http.c` and `http-form.h`. Fresh seed-to-Stage4
bootstrap retains byte-identical Stage 2/3/4 generated C and the unchanged seed.
The SDK relinks the compiler with a relative library path; its separate
relocated generation gate must also pass before acceptance.

No protected test, expected output, runner, timeout, seed or handwritten product
JavaScript was changed. This is plain-HTTP pressure qualification. TLS pressure,
full application/DSL parity, default development/deployment migration and latest
whole DB95 safety/operations remain separate requirements. Commercial HOLD;
production deployment NOT_RUN. Private paths and owners: <가림>.
