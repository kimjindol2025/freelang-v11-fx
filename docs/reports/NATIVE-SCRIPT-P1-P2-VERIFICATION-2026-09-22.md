# Native Script P1 / P2 Verification Record — 2026-09-22

## Scope

This record covers **FreeLang FX native** only. It does not establish a PASS
for FreeLang AFJ, FreeLang Script's Node runtime, or any self-hosted path.

```text
REPOSITORY=/home/kim/kim/platform/freelang-v11-fx
BRANCH=master
COMMIT=022fdf11396cb7abc1c06a2f4f9700afedda2213
WORKTREE_STATE=DIRTY
PRODUCT_SOURCE_CHANGED=NO
PATCH=NO
```

This document records an inspection and execution result. The existing native
implementation files were not modified by this verification.

## Finding A — Script ABI P1-A

```text
IMPLEMENTATION=LOCAL_ONLY
REMOTE_IMPLEMENTATION=NO
COMMIT=NOT_CREATED
```

The local worktree contains uncommitted changes in:

- `runtime/aliases.c`
- `runtime/runtime.h`
- `tests/test-script-abi.c`
- `tests/run-script-abi.sh`

The implementation provides Script-compatible `get_arg`, `sys("args")`, and
an `exit(FLValue)` bridge that accepts only integer status values from 0 to
255.

### Execution evidence

```bash
cd /home/kim/kim/platform/freelang-v11-fx
nice -n 10 bash tests/run-script-abi.sh
```

```text
PASS Script ABI P1-A
```

PASS gate: argv retrieval, `sys("args")`, integer exit status `42`, and
invalid-exit rejection all completed in a compiled native test binary.

## Finding B — Native import and lockfile P1-B

```text
IMPLEMENTATION=LOCAL_ONLY
REMOTE_IMPLEMENTATION=NO
COMMIT=NOT_CREATED
```

The local worktree contains:

- `tools/fx-script-pack.c`
- `tests/run-script-import.sh`

The packer constrains imports to top-level relative paths, rejects project-root
escape and cycles, and creates or verifies module-hash lockfiles.

### Execution evidence

```bash
cd /home/kim/kim/platform/freelang-v11-fx
nice -n 10 bash tests/run-script-import.sh
```

```text
fx-script-pack: modules=2 ... mode=update
fx-script-pack: modules=2 ... mode=verify
PASS Script import P1-B
```

PASS gate: a native module packer built and verified a two-module import graph
and its lockfile.

## Finding C — Native capability gateway P2

```text
IMPLEMENTATION=NOT_IMPLEMENTED
PLAN_EXISTS=YES
EXECUTION=BLOCKED
```

No native runtime symbol or implementation was found for a capability gateway,
read/write/net/env allowlists, canonical-path checks, host allowlists, or
permission-denied enforcement.

The direct native effect entry points remain outside such a gate:

- `runtime/io.c`: `_fl_env_get` calls `getenv`; `_fl_env_set` calls `setenv`.
- `runtime/http.c` and `runtime/http_client.c`: HTTP entry points are directly
  callable.

The separate `freelang-script` native conformance runner explicitly records
this boundary as:

```text
native capability gateway is not implemented; P2 preflight blocks effect execution
```

Node Script runtime permission checks are not evidence for this FX-native P2
finding.

## Forgejo status

```text
REMOTE=forgejo-dclub:kim/freelang-v11-fx.git
REMOTE_BRANCH=master
REMOTE_HEAD=022fdf11396cb7abc1c06a2f4f9700afedda2213
LOCAL_BASE_HEAD=022fdf11396cb7abc1c06a2f4f9700afedda2213
REMOTE_IMPLEMENTATION=NO
```

The local P1-A and P1-B files are uncommitted, so the matching Forgejo branch
does not contain this implementation.

## Verdict

```text
FX_NATIVE_SCRIPT_ABI_P1_A=PASS
FX_NATIVE_IMPORT_LOCKFILE_P1_B=PASS
FX_NATIVE_CAPABILITY_GATEWAY_P2=NOT_IMPLEMENTED
PRODUCT_SOURCE_CHANGED=NO
PATCH=NO
```

The P1 PASS results do not imply P2 capability enforcement, a full native
toolchain, or overall FreeLang completion.
