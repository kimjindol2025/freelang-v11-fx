# Native line TCP requests and linear range construction

SCOPED_RUNTIME=PASS; COMMERCIAL_READINESS=HOLD; DEPLOYMENT=NO.

`tcp-send` now runs through a Native helper, without a Node child. Activate a
matching SDK that contains `bin/fl-tcp-request-native`; activation exports
`FREELANG_NATIVE_TCP_REQUEST`. A direct host can configure that absolute helper
path explicitly. Otherwise the runtime selects that filename below
`FREELANG_NATIVE_SDK_ROOT/bin`. Requests, including passwords, travel through
anonymous stdin framing, never executable argv or shell evaluation.

The retained wrapper defaults are localhost, port 30390, an empty request and
3000 ms. It adds LF only when absent, accumulates complete received chunks
through the chunk containing LF, normalizes UTF-8 and trims the result. Socket
inactivity can return a nonempty partial response; EOF without LF does not.
Zero disables the idle timer while the enclosing 500 ms budget remains. The
parent owns the total `timeout + 500 ms` budget, including DNS, helper startup,
stdin and output. Invalid arguments, process failures, empty output and budget
or output failures return nil. This is plaintext line TCP, with no SQL, TLS or
authentication policy in C.

The internal `process-input.h` boundary uses literal absolute argv, anonymous
binary stdin and a combined stdout/stderr byte budget. It polls all streams
together, closes inherited descriptors, preserves embedded NUL and leaves
SIGPIPE disposition unchanged. A private process group and an unreaped owned
leader pin cleanup until the direct child is reaped. Existing shell entry
points retain their previous contracts.

The source contract follows the project's retained bootstrap wrapper and
[Node's timeout validation](https://github.com/nodejs/node/blob/v22.17.0/lib/child_process.js),
[socket timers](https://github.com/nodejs/node/blob/v22.17.0/lib/internal/timers.js),
[non-final UTF-8 decoding](https://github.com/nodejs/node/blob/v22.17.0/src/string_decoder.cc)
and [combined synchronous output accounting](https://github.com/nodejs/node/blob/v22.17.0/src/spawn_sync.cc).
The new TCP parent retains a 1 MiB combined output budget after helper decoding
and trim. No legacy Node reference process was executed in this qualification;
the recorded controls establish their tested domain rather than every Node
side effect, extreme numeric input or output-flush edge.

Sixteen process controls pass, including 3 MiB binary stdin, bidirectional
backpressure, literal argv, inherited descriptors, exact output/overflow,
nonzero exits, signals, timeout/reap and owned descendant cleanup. Forty TCP
controls pass for request coercion/defaults, Unicode/NUL, fragmented and
coalesced replies, idle refresh, total timeout, EOF, incomplete UTF-8, large raw
whitespace, output bounds, localhost DNS and numeric IPv4/IPv6. New runtime,
helper and SDK product traces contain zero Node-family/Python executions.

`range` now fills one unpublished vector with geometric arena growth. It
preserves the original repeated double addition, coercion, order, tags,
rounding and empty cases; it adds no mathematical pre-count or smaller data
limit. Twenty-two small cases match the accepted runtime exactly. Three
retained 16,224-value ranges verify every value and published immutability.
Six fresh full-data cold recoveries of actual 16,224/14,664/13,424-row WALs,
twice each, retain the old schema2 reader's generated C and range iteration.
Only the linked runtime changes. Every raw string ID/name, revision and both
independent digests match under the original 120-second/2 GiB limits; WALs stay
byte-identical. The earlier exit139 remains recorded.

The unchanged existing shell17, HTTP-cancellation30 and HTTP-pressure129
controls also pass. Their two private Python controllers and two original
Python shell-fixture helpers are explicitly accounted for; they introduce no
runtime dependency. The unchanged seed reaches identical stage2/3/4 generated
C under the existing 40-second, 1.6 GiB RSS and 2 GiB address-space gates.
The matching SDK contains 18 ELFs, including the new helper. Its corrected
frontend tooling passes 45 original release checks and five additional helper
relocation/integrity/legacy-argv checks, with no service activation.

The three new C probes in `tests/native-bootstrap` are the exact qualified
sources. Link them with the matching runtime/archive and existing platform
libraries. Invoke `process-input-probe` through an absolute filename under its
20-second/2 GiB test envelope and the host heavy lock. `range-probe` runs the
small cases; `--large` additionally checks all 16,224 values three times.
`tcp-send-probe` takes a JSON argument vector on stdin for isolated fixtures.
Do not substitute a different runtime archive or regenerate the bootstrap seed.

Hashes, per-cohort results, trace audits and retained failures are in
[`evidence/native-tcp-request-range-20261121.json`](evidence/native-tcp-request-range-20261121.json).
These results do not replace the earlier original DB diagnostic of
52 PASS / 19 FAIL. The later private DB UTF-8 wire correction, full Native
client ports, TLS/prepared/auth scenarios, storage ownership and operational
qualification require their own evidence before any commercial decision.
