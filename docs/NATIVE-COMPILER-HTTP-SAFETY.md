# Native compiler failures and HTTP bind selection

The positional Native compiler rejects unsupported IR/literal/block kinds
with an exception instead of substituting fl_nil(). IR validation now
propagates failure, and the positional command checks its result before
generating or writing C. This prevents the observed extra-delimiter source
from printing a successful compile message or overwriting an existing output.

Three private CLI controls cover an extra parenthesis, an extra bracket and
an invalid loaded module. Each exited 1, retained the existing C sentinel and
printed no successful compile message. These controls are not a claim that
every malformed source form is rejected by the existing permissive parser.
Strict --parse-json/--check preflight remains necessary for source syntax;
full positional syntax preflight is still pending.

## Per-process HTTP/TLS address

FL_HTTP_HOST optionally selects an IPv4 bind address for server_start and
server_start_tls. The address is parsed before upload recovery, worker-pool
creation or certificate reads. Invalid input throws a constant diagnostic
without echoing the supplied value. There is no DNS lookup or persistent
configuration. The unset default remains compatible with the old API.

The separate Native web launcher forces FL_HTTP_HOST=127.0.0.1 and refuses
other values. Direct SDK use must set the intended address explicitly.
The web launcher's behavior is separate from the SDK's compatibility default.

| Evidence | Result |
|---|---|
| Invalid hostname / empty / IPv6 / newline values | 8/8 rejected, HTTP and TLS entry |
| Invalid-host socket bind / certificate access | none observed |
| Unset / explicit loopback selection | both observed through bind fault injection |
| Fault-injection observer | no real non-loopback bind performed |
| Actual Native HTTP /health | 200, exact response bytes on ephemeral 127.0.0.1 |
| Real TLS handshake | NOT_RUN; no key/certificate files created |
| Owned health-server processes after cleanup | 0 |

The observer intercepts bind and returns an error so the compatibility default
can be inspected without opening a listener on all interfaces. The real
health-server probe uses an ordinary Native binary with the current runtime.

## Fresh bootstrap and regression

Final accepted receipt: native-web-fixedpoint-v3.umdne80t. Strict Native
source preflight succeeds before the actual seed -> Stage 1 -> Stage 2 ->
Stage 3 -> Stage 4 procedure. Stage 2 = 3 = 4 C is byte-identical:

c1f48e888960f21679278d4e90ce0d2e05d3e51bbfecefafcd212819905458fa

Stage-3 ELF SHA-256:
5fe1a1cf642c5fd244abe18a9d013979e7a6ed522940973afa764c6a8a0022a7

Generation: 39.68 / 37.80 seconds, peak RSS 1,524,352 / 1,530,188 KiB.
The 2 GiB cap was unchanged. Seed before/after:
0101cb63a86f20b93451f70b88a0b577f9548ded0a8b328ebdeedb7e8e4fd767.

An initial edit had mismatched throw delimiters. The existing permissive
compiler reached a fixed point despite that source error; strict source
preflight rejected it, exit 1. That receipt is not accepted as source
validation. The edit was corrected, preflight was added to the private
bootstrap driver, and the final validator-return change was rebuilt and
verified separately. The seed and protected verifier were not edited.

Fresh core receipt native-web-core.1t1ve_e_ passed frontend 71/71, CLI 22/22,
FX 11/11, page checker 32/32 plus defaults, literal/value/split/push contracts
and original DB95 7/13/36/40. Collected Native core traces contain no Node
execution. The protected official verifier with its older seed pin remains
NOT_RUN; the actual fresh four-stage procedure above supplies the evidence.

Boundary receipt native-web-runtime.qzqqqnu6 completed the compiler, host and
real health-server assertions before a private offline PATH hid strace from
the later full-application probe. That driver failure is retained; it is not
a successful full application or browser result.

The approved temporary curl prefix b20r-curl-8.9.1.ARZB6s was verified and
used only through process environment. Official packaging remains pending.
No existing tests, expected values, timeout, runner, JS or seed were changed.
Host paths/owners: <가림>. Full DB95 71 was not rerun (51 PASS / 20 FAIL
recorded baseline); checkpoint fault recovery still blocks commercial readiness.
