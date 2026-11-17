# Native language check command

The verified Native compiler provides language source checking:

~~~sh
"$FREELANG_NATIVE_COMPILER" --check program.fl
"$FREELANG_NATIVE_COMPILER" --check program.fl --json
~~~

Source is inspected without executing it or following load directives. The
command performs lexical/structural preflight, function metadata/effect
analysis and the reference's best-effort arity/literal-type checks. JSON uses
the freelang.diagnostic.v1 fields, diagnostic families, legacy codes and
severity/status shape. This is separate from the frontend's page/island
shape checker in tools/fl-check.fl.

Text mode preserves the inspected reference exit policy: syntax/file/type
errors fail; metadata/type warnings and printed pure-effect violations do
not fail. JSON mode fails on every diagnostic with error severity. Invalid
Native --check arguments fail; missing file arguments show usage with exit 2.
The new compiler command does not invoke Node or search for another runtime.

## Verified receipt

The command is compiled by the existing Native bootstrap, with no JS/Node
compiler stage and no seed replacement or regeneration.

| Evidence | Result |
|---|---|
| Fresh Stage 2 = Stage 3 = Stage 4 C | Exact byte comparison, exit 0 |
| Stage 2 to 3 generation | 38.80 s, peak 1,514,012 KiB, exit 0 |
| Stage 3 to 4 generation | 39.58 s, peak 1,515,368 KiB, exit 0 |
| Existing frontend test-file checks | 7 files in JSON/text: 14/14 identical stdout, stderr and exit |
| Synthetic/missing-file JSON comparison | 26/27 identical payloads |
| Synthetic/missing-file text comparison | 26/27 identical stdout/stderr bytes |
| Synthetic/missing-file exit comparison | 53/54 identical |
| Source body execution | None; an inspected file-write expression creates no file |
| Native command trace | 0 Node-family executions, 0 network request syscalls |

Generated C SHA-256:
4e589203e96b1cbfa8e4983439df33ab9a471daa3c95f5d4d89f99573c139c7e.

Stage 3 compiler ELF SHA-256:
24355a87f854de32775c65773663f32050f4cc5c0b2e156b964966ea356f7fa3.

Bootstrap input SHA-256 before/after:
0101cb63a86f20b93451f70b88a0b577f9548ded0a8b328ebdeedb7e8e4fd767.
The protected official bootstrap verifier pins an older input and remains
NOT_RUN; a separate evidence harness uses the existing Native stage recipe.

Reference comparison used the existing Node v22.19.0 checker only to capture
compatibility diagnostics. Its bootstrap SHA-256 is
2827f82534025ea18e06050dd56dfeefbd176c71f5eac887d4edd10d0e900052.
The reference and inspected inputs were unchanged; those comparison runs
made zero network requests. Reference execution is not a Native build
dependency and is separate from the zero-Node Native command traces.

## Explicit difference and limits

The reference metadata walker misses println nested inside an array in a
declared pure function. Native finds it. The Native JSON result contains
E_PURE_VIOLATION and exits 1 where the reference exits 0. Both text commands
exit 0, with an additional Native violation diagnostic. The stricter
detection is retained; zero-difference parity is not claimed. Existing
reference code, tests, expectations and runners were not changed to hide it.

The type checker remains a heuristic, not a soundness proof. Metadata,
normalized control forms and collection traversal use the Native parser
representation; arbitrary advanced grammar, every IO error/locale and
adversarial/deep input remain unverified. This receipt does not prove typed
program code generation or complete language/runtime API coverage. File IO
retains its existing size and confinement policies.

The ordinary C-compilation source loader and the explicit check command are
separate paths. The explicit checker rejects malformed inputs tolerated by
the legacy parser, including missing delimiters, empty calls, incomplete maps
and invalid top-level literals. It reports lexical errors without attempting
to run the inspected program. No global configuration or system dependency
was installed for this work; the existing private curl build environment is
still temporary and production packaging remains pending.

Receipts: native-check-reference.ncFCVI,
native-check-frontend-reference.Fq7L4u,
native-check-assembly-v2.25bgSh, native-check-fixedpoint.6o1yXU and
native-check-command-final.nSZaNo. Host paths and owners: <가림>.

Final source hygiene removed only extra blank lines at EOF from the two new
FreeLang modules. A fresh Native generation from those final bytes matched
the recorded Stage 4 C exactly, exit 0 (38.75 s, 1,515,944 KiB). Its full
source manifest was unchanged and its trace contained zero Node executions.
Receipt: native-check-final-source.i3wziqo8.
