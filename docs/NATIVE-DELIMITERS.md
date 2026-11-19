# Native structural delimiter rejection

The positional compiler previously accepted a missing or mismatched closing
delimiter and generated C from the resulting incorrect scope. An existing DB
soak source exposed this: strict source-only parsing reported an unclosed opening
at line 29, but positional compilation succeeded, printed intermediate PASS
lines and later failed on a misplaced `finally`. Its separate path-only control
timed out; it is not a successful load qualification.

Parenthesis, vector and map consumers now require the matching closing token.
Unexpected closing tokens also fail. These parser checks cover positional C
generation, browser generation and expanded loads, without adding a second
full-source scan. A failure leaves an existing output file intact. Structured
source-only diagnostics remain available through --parse-json; this correction
does not claim identical diagnostics for every lexical error.

The isolated candidate passes 65/65 controls, including the preserved old
acceptance, missing/mismatched/extra delimiters, browser/load boundaries, output
retention, exact generated C parity for valid inputs and rejection of the
unchanged malformed DB fixture. Self-generated compiler C is identical;
generation took 29.29 seconds with 1545992 KiB RSS under the existing 2 GiB cap.
The current coherent seed-to-Stage4, fresh runtime, core and relocated SDK
verification now passes, and the same 65 controls pass on that SDK. See
[NATIVE-SSE.md](NATIVE-SSE.md#current-coherent-checkpoint) and its machine-readable
evidence for the current hashes and complete scoped results.

Receipt: native-delimiter-20261120. No protected test, assertion, expected value,
runner, time bound or seed was edited. Private host paths and owners: <가림>.
