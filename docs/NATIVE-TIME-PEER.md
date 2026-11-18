# Native UTC time and request peer compatibility

The generated web application still contained unresolved now_iso and
server_req_client_ip lookups after its read-oriented HTTP probe passed.
Private FreeLang reproductions were written first; each compiled and then
failed with Function not found, exit 1. The new runtime supplies the existing
public operations without empty fallback functions or application feature
removal.

## UTC time

now_iso / now-iso obtains CLOCK_REALTIME and formats the value with gmtime_r.
It returns UTC ISO text with three millisecond digits, matching the existing
module's toISOString behavior. Formatting does not use localtime or mutate
TZ, locale or the system clock. Year 0, signed six-digit years and the
Date-compatible range boundaries are handled explicitly. Clock/conversion/
range errors throw; they do not become an invented time.

Existing now-ms, date-parse and format-date behavior is retained. Local/Korean
presentation remains a separate formatting concern; now_iso always ends in Z.

## Request peer

server_req_client_ip / server-req-client-ip reads the request map's remote_ip
field, with unknown for a missing/false/empty value. The Native HTTP request
builder populates both existing peer-address and compatible remote_ip from
the same getpeername/inet_ntop result. Headers, query values and JSON body
fields cannot override that socket result. The accessor requires a request
map. A caller constructing its own map can supply its own field, as with the
existing module helper; this is not an identity/authentication mechanism.

## Private contract evidence

native-time-peer.g_b0tvv0 passes 28 checks:

| Evidence | Result |
|---|---|
| Epoch, sub-ms flooring, one millisecond, negative timestamp, leap date | PASS |
| Year zero, extended positive/negative years, both range endpoints | PASS |
| Outside both range endpoints and clock failure | Rejected, exit 1 |
| Same frozen instant under UTC / Asia-Seoul / America-New-York | Identical UTC text |
| Live clock; both spellings and first-class time calls | PASS |
| IPv4/IPv6 request maps, missing/empty/false fields, first-class peer call | PASS |
| Lexical function shadowing | PASS |
| Eight concurrent real loopback HTTP requests | Correct UTC text and 127.0.0.1 |
| Forged forwarding/custom IP headers, query and body IP values | Ignored for socket peer |
| Owned/traced processes after HTTP cleanup | 0 |

The time probe links a private clock_gettime wrapper for deterministic values;
it never changes the host clock. The server uses an ordinary Native binary
and an ephemeral loopback port. Its collected trace contains no Node execution
or outbound connect call. Source/runtime/seed manifests match before/after.
The 28-row result includes the server cleanup row, not 28 independent test
files or the protected API suite.

This initial probe uses previously verified compiler C with the current
runtime as a provisional compiler. Fresh four-stage bootstrap, regression
and real web audit integration are recorded separately before checkpoint
acceptance. Existing tests, expected values, runners, timeouts and seed were
unchanged. No JS, download, installation, order storage, customer DB access
or external provider operation was used. Host paths/owners: <가림>.

## Fresh current-runtime bootstrap

Receipt `native-time-peer-fixedpoint.wzi4egiu` performs the actual
seed -> Stage 1 -> Stage 2 -> Stage 3 -> Stage 4 rebuild with strict source
preflight. Stage 2 = 3 = 4 C is byte-identical:

`c1f48e888960f21679278d4e90ce0d2e05d3e51bbfecefafcd212819905458fa`

Stage-3 ELF SHA-256:
`509425939227c07cefdbf907f7f56c97b0f47289401c021269de0eb219f9b9ae`.
Generation: 38.93 / 39.40 seconds; peak RSS 1,527,172 / 1,525,868 KiB.
The 2 GiB cap was unchanged. Seed before/after:
`0101cb63a86f20b93451f70b88a0b577f9548ded0a8b328ebdeedb7e8e4fd767`.
The complete source/runtime/seed manifest matched before and after.
This rebuild follows actual runtime changes; earlier measurements, including
the shell_capture checkpoint's 40.29-second miss, remain in their receipts.
The older-seed protected verifier remains NOT_RUN and was not edited.

## Fresh core verification

`native-time-peer-core.8u77cz_p` passed frontend 71/71, CLI 22/22, FX 11/11,
page checker 32/32 plus defaults, literal/value/split/push contracts and the
original DB95 7/13/36/40 cases (all exit 0). It records CORE_VALIDATION=PASS,
CORE_EXIT=0 and identical complete source/runtime/seed manifests. Collected
Native core traces contain zero Node-family executions. The original DB
subset is a regression result; full DB95 71 and commercial readiness are
not established by it.

The frontend's NATIVE-WEB-BUILDER.md records the rebuilt application and its
real Git audit integration separately. Complete browser/default-entry,
production packaging and DB checkpoint recovery gates remain unfinished.

The first expanded web probe, native-web-runtime-v7.cba9233d, reached the
approval-denial control and failed: the existing server-json helper ignored
its payload-first status argument and emitted HTTP 200 instead of 400.
The socket trace confirms the denial body and wrong status. This is not
counted as successful full web audit integration. The time/peer contracts,
fresh bootstrap and core results above remain their own passing evidence;
the HTTP status compatibility defect is addressed separately.
