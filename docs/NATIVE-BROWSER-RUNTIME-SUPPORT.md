# FreeLang browser runtime modules

`--browser-runtime-js` compiles a separate FreeLang host/runtime module together
with a user program:

```text
flc --browser-runtime-js runtime.fl input.fl output.js
```

The ordinary `--browser-js input.fl output.js` interface remains available.
The new mode uses the same browser backend and syntax checks. Its explicit
runtime module is implementation input: it is parsed and expanded relative to
its source directory, then appended to the embedded browser core in the parser
callback. The user program still passes the existing private-name and host
primitive validation. This does not introduce a handwritten JavaScript emitter
or a separate language interpreter.

The supplied runtime should use its own bindings. Compiler-only `j-host-*`
operations are available there, while ordinary program input continues to reject
them and private core names. JavaScript output must have the existing `.js` or
`.mjs` suffix. Canonical output path equality with either direct input is rejected.
Read/parse/generation failure does not create a successful output artifact.

The initial consumer is the Native headless execution foundation. The separate
SpiderMonkey/GJS process host executes generated code with the original Native
memory cap; the actual Island runner and service integration remain unfinished.

Qualification records unchanged seed bytes, exact Stage 2 = 3 = 4 C, existing
40-second/1.6GiB generation gates under the 2GiB VM cap, relocated self-generation,
default browser byte comparisons, source guards, generated runtime execution and
existing CLI/frontend/FX regressions. The evidence is
`docs/evidence/native-headless-engine-20261120.json` in this repository. No
protected fixture, assertion, runner, expected value or bound was changed.

The full FX/DB/frontend goal remains active. This compiler feature does not
establish complete headless/default development migration or DB commercial
qualification, and no production deployment has run.
