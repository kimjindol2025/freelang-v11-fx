# Optional Native field-predicate compatibility

`native-field-predicate.fl` is the unchanged 24-case source exercised against
the accepted runtime and a separate candidate runtime. Both suites pass. This
commit archives the test, its two expected outputs and its scoped evidence.
The current product runtime uses the callback fallback; the optional primitive
implementation remains in the private candidate until its broader qualification.

The test probes availability instead of assuming a capability flag. For each
case, the reference is the original `any?` callback using `get` and `=`. A
supported Native result must agree with that reference; `nil` requests fallback.
Cases cover empty/absent/missing values, string/number/bool distinctions,
integer/float equality, negative zero, Unicode, embedded NUL, unsupported keys
and targets, mixed row shapes and the original input-order short circuit.

After strict parsing, Native C generation and linking against the selected
runtime, run the same executable source with the matching expectation:

```bash
EXPECT_NATIVE_FIELD=false timeout --foreground 120s ./predicate-bin
# Compare with native-field-predicate-baseline.expected.
EXPECT_NATIVE_FIELD=true timeout --foreground 120s ./predicate-bin
# Compare with native-field-predicate-candidate.expected.
```

The first invocation selects a runtime without the optional operation; the
second selects one that implements it. The availability assertion prevents a
missing implementation from silently satisfying candidate-mode coverage.
Qualification used two freshly built twenty-source runtime archives, the same
generated C, a 2 GiB address-space limit and owned PID/network isolation. It
did not substitute an SDK archive. See
[the evidence](../../docs/evidence/native-field-predicate-contract-20261121.json).

The enclosing private profile later failed a separate large-WAL recovery with
exit 139. That failure is retained and the whole profile is not PASS. These
24-case results establish the recorded collection contract only; they do not
qualify DB recovery, the complete SDK/frontend, production or commercial use.
Existing tests, expected outputs and default test selections are preserved.
