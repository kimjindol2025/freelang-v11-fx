# Legacy/native and v11 API names after DB95-8

Keep each engine and its fixed outputs. Legacy fixture call names explicitly
select the original native contract; mainline public names keep v11 semantics.

| Legacy contract | Explicit legacy spelling | Mainline spelling |
|---|---|---|
| strict Map keys / non-Map error | map-has-key? | has-key? |
| POSIX pattern-first match | regex-match | regex_match |
| POSIX string-only insensitive match | regex-match-i | regex_match_i |
| raw socket with five-argument closure | fl-tcp-server-raw | tcp-server-raw |
| write to that legacy connection | fl-tcp-write | tcp-write |
| poll that legacy socket loop | fl-tcp-yield | fl-yield |

The fl-tcp spellings are existing C entrypoint aliases from runtime.h. The
mainline socket loop uses named callbacks and is kept alongside the legacy loop.
Expected values, port ranges, timeouts and error checks remain unchanged.
