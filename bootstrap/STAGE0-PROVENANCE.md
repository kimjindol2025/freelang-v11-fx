# Native Stage 0 provenance

This seed is a generated artifact. It was not hand-edited, reformatted, or
post-processed.

- AFJ generator commit: `051f5347c07653b833c190b45233d1abb9475a3d`
- AFJ branch: `feat/bootstrap-native-parser-adapter`
- FX input commit: `587d8ed352e336ed99c20bc2faf9b618a8e4f5cf`
- FX input: `self/cgc-main.fl`
- FX input SHA-256: `a7af00f8ff736d152b30f3be71f00db035aa975e2ad17471e3dc7ec92cb9e10d`
- Generation command:

  ```text
  FL_BOOTSTRAP_NATIVE_LEX=1 FL_BOOTSTRAP_NATIVE_PARSE=1 \
    node bootstrap.js run self/cgc-main.fl self/cgc-main.fl /tmp/stage0.c
  ```

- Generated `stage0.c` SHA-256: `f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c`
- Canonical fixed-point SHA-256: `aa8bed315d2bae91630fcff72d7edcafb9afafd84dc357133bb271c3d1bda4fd`
- Stage 0 is a JS-assisted seed. It is not a claim of zero-seed bootstrap.
- From stage 0 execution onward, verification uses only the generated native
  ELF, the FX `.fl` source, the listed C runtime, and the system C compiler.
- The verifier runs native commands with `PATH=/usr/bin:/bin` and records
  `execve` calls. It rejects Node, npm, TypeScript/JavaScript runtimes, AFJ
  files, and `cgc-bin` after seed creation.

The runtime source list is intentionally shared with the existing fixed-point
verification contract, including `runtime/user-fns.c`.
