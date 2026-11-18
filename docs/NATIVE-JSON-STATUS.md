# Native JSON response status compatibility

The expanded Native web probe reached the application's approval denial.
Its body correctly reported TERMINAL_APPROVAL_REQUIRED, but the wire status
was HTTP 200 instead of the requested 400. The helper recognized only the
existing status-first convention and ignored the payload-first status used
by the application. Receipt native-web-runtime-v7.cba9233d retains the failed
probe and socket trace.

server_json / server-json now supports both valid forms:

~~~freelang
(server_json {"ok" false} 400)
(server_json 201 {"created" true})
(server_json {"data" 1})
~~~

The first returns status 400 with the supplied map. The second retains the
existing status-first result. A body-only call defaults to 200. To preserve
the prior Native contract, a numeric first argument recognized as a status
keeps status-first precedence, including its existing conversion behavior.
Numeric-body/status ambiguity is not silently reinterpreted.

For the payload-first form, an explicit second status must be an integer
numeric value from 100 to 599. Invalid values throw a constant TypeError;
they do not become a successful 200 response. Response bodies remain native
structured values until the existing HTTP transport encodes them. Existing
header preparation, framing and status-first behavior are retained.

## Bounded evidence

Private reproductions were written before the implementation change.
The corrected baseline records payload-first status 200 versus required 400,
while the existing status-first control returns 201. The initial private
probe mistakenly JSON-parsed an already-structured body and exited 1; its
constructor inspection was corrected to read that field directly, keeping
the expected values unchanged. The correction record distinguishes captured
tool evidence from an original raw log.

Receipt native-json-status.swfvh1d9 passes 25 rows: 15 constructor results,
7 rejected invalid statuses, 2 real HTTP responses and server cleanup.
The invalid cases cover 0, 600, fractional, string, nil, false and a large
finite number. A real loopback server emits 400 with the exact denial map
and 201 with the exact creation map. Its collected trace has no Node-family
execution or outbound connect calls; owned/traced processes after cleanup
are zero. The FX source/runtime/seed manifests match before and after.

This probe uses the previously verified compiler for the unchanged public
operation, linking against a freshly built current runtime archive. It does
not claim a fresh current-runtime compiler bootstrap. Fresh four-stage,
core and expanded application acceptance are recorded separately.
Existing tests, expected results, timeouts, protected runners and seed are
unchanged. No JS, dependency download/installation, customer DB connection,
order storage or external provider/email/AI operation was used.
Host paths/owners: <가림>. DB commercial readiness remains HOLD.

## Fresh current-runtime bootstrap

Receipt `native-json-fixedpoint.q68kbcsj` rebuilt the actual seed
through Stage 4 with strict source preflight. Stage 2 = 3 = 4 exact C SHA-256:

`c1f48e888960f21679278d4e90ce0d2e05d3e51bbfecefafcd212819905458fa`

Stage-3 ELF SHA-256:
`c8c6a0e691ddec4c4a9e0626d2b7c17d0d7f0450c40fb1847f4c1b13ff854f67`.
Generation 38.88 / 36.88 seconds, peak RSS 1,526,500 / 1,528,988 KiB.
The 2 GiB cap was unchanged. Seed before/after:
`0101cb63a86f20b93451f70b88a0b577f9548ded0a8b328ebdeedb7e8e4fd767`.
The complete source/runtime/seed manifest remained identical before/after.
The earlier fixed-point and failure receipts remain recorded; this fresh
rebuild follows a real runtime change, not a timing-only retry. The protected
older-seed verifier remains NOT_RUN and unchanged.

## Fresh core and application boundary

`native-json-core.wcpd3881` passes frontend 71/71, CLI 22/22, FX 11/11,
page checker 32/32 plus defaults, primitive contracts and original DB95
7/13/36/40 (all exit 0). The complete source/runtime/seed manifest matches
before/after and collected Native core traces contain zero Node execution.

The next application probe, native-web-runtime-v8.per8gev2, confirmed the
approval denial now returns HTTP 400. It then failed the approved Git-read
output check: the application's redactor used the older pattern-first regex
operation with text-first arguments and replaced ordinary output with a
pattern string. The frontend corrects that call site separately and retains
this failed receipt. The HTTP status and current bootstrap/core results are
not presented as completed application acceptance. Full browser/default
migration, packaging and DB95 commercial gates remain unfinished.
