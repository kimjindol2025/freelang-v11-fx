# FreeLang Script stdin line input

The v11 `stdin-on-line` host function accepts:

```text
stdin-on-line(callback, intervalMs = 1000, requireTerminatedLine = false)
```

`requireTerminatedLine` is a boolean option. Its default is `false`, preserving
the general line-reader behavior that delivers a non-empty final buffer at
EOF. When enabled, a non-empty buffer without a newline is rejected: the
process exits unsuccessfully and writes `unterminated line at EOF` to stderr;
the callback is not invoked for that incomplete frame. The option is
implemented in both the interpreter host module and generated-JavaScript
bridge.

`readline(prompt = "", requireTerminatedLine = false)` exposes the same strict
EOF rule for synchronous compatibility loops. MCP stdio uses strict mode
because JSON-RPC messages on that transport are newline-delimited. Callers
that treat EOF as a valid line terminator should keep the default.
