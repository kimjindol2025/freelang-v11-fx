# B03 native image contract

User approval: 2026-11-17 “의존성·API·GIF 준비 변경 승인”.
Base c99b6f7833e7c8e73cfd18946867d927e4228146. Read-only seed SHA:
f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c.

The twenty contracts in tests/image-native/contracts.fl and contracts.expected
are fixed before implementation. No existing test, timeout, runner or seed changes.
Fixture images are synthetic C-generated JPEG baseline/progressive 1600x800,
RGBA PNG 1600x800, tiny PNG 20x10, and two-frame GIF; corrupt inputs are truncated
versions and a fake GIF signature. No original production data or keys.

## Interface

image-info-result(path, options), image-process-result(path, outputs, options).
Result is {ok:boolean, error:string, data:map-or-nil}. All invalid input and runtime
failures are observable; no silent exception suppression. Options are required:
max_pixels, max_bytes, memory_bytes, timeout_ms, jpeg_quality (positive integers,
quality 1..100). Optional expected_mime rejects a mismatch. Upper safety bounds:
512MiB worker address space, 5000ms wall time; file contract's existing max remains
unchanged. D05 passes 20,000,000 pixels, 5242880 bytes, 536870912 bytes, 5000ms, 85.
outputs is a nonempty vector of at most two {path,long_edge} maps. Only PNG/JPEG
derivatives, GIF first-frame PNG, aspect retained, no crop/enlarge. On success data
has width,height,mime,alpha,frames and process jpeg_quality and outputs metadata.

Root is existing FL_FILE_BASE, required for image APIs. No traversal/symlink,
nonregular input or output overwrite. Parent securely opens fd-relative paths,
worker receives only explicit descriptors and numeric options, never shell text.
Worker is configured by trusted FL_IMAGE_WORKER absolute executable environment.
A root-scoped lock serializes processes. Parent enforces deadline including wait,
kills/reaps a failed child, removes unpublished/partially published outputs, and
returns an error. Output publication uses exclusive atomic links, no overwrite.

The codec worker is separate so the compiler/runtime have no new codec link ABI.
Build scripts fetch pinned upstream archives to the caller's isolated build tree,
retain licenses, and link only the worker to static libraries. No system install.

IMG19/20 also require supplementary native supervisor evidence: deadline kill,
worker exit, output cleanup, concurrency one, fd/child stability and repeated-job
leak checks. contracts.fl alone does not establish those resource properties.

Contract commit: ef354a55aa12accea3447031821b9d178c647aec. Before implementation
the native execution exited 1: Function not found: image-info-result (line 6).
Initial integration retained two failures (GIF error precedence, output-path errno)
and corrected their causes without modifying the fixed contracts. Then 20/20 passed
(0.58 seconds, 10496KiB); resource supervisor 29/29, 200 repeat jobs, 4 concurrent
requests. This intermediate run precedes bounded input snapshot hardening; final
retest, language regressions, leak sanitizers and bootstrap are still required.

Pinned archive SHA-256 (source archives, not generated binaries):

| Library | SHA-256 |
|---|---|
| zlib 1.4 | 02dd159783677a749c04adbdd02b55b8a2bcc7e054f51b76b549199a916ad584 |
| libpng 1.6.59 | 86a3e4b501f7f50e392c4e456ea158893e9a595b1c63eb88c7bb9f6cf8772dad |
| libjpeg-turbo 3.2.1 | c27ad72a68da151177929aa969ae773a2da5ac2af9ce946079314fdac8cece7f |
| GIFLIB 6.1.3 | b65b66b99f0424b93525f987386f22fc5efb9da2bfc92ad4a532249aaffbab0e |

The isolated prefix contains license notices for all four. The worker uses explicit
static .a paths, and ldd must confirm no dynamic codec dependency. Build with
`bash scripts/build-image-deps.sh ISOLATED_BUILD_ROOT` and then
`bash scripts/build-image-worker.sh ISOLATED_BUILD_ROOT/prefix WORKER_OUTPUT`.
Caller supplies TMPDIR in the task area, shared heavy lock, ps and memory cap.
Set trusted FL_IMAGE_WORKER to the worker executable and FL_FILE_BASE to the
application sandbox. Missing worker configuration produces WORKER_UNAVAILABLE.

Status: language validation in progress; no blog B03 PASS declaration.
