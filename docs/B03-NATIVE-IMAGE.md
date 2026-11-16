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

Status: contracts fixed, implementation and all twenty results NOT_RUN.
