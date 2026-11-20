# Raw JavaScript host grammar for FreeLang adapters

Compiler-only browser runtime modules can now use fixed host grammar for
`this`, `arguments`, `undefined`, strict inequality, greater/less-equal comparison,
remainder, `instanceof`, and iterable spread. Model or application logic remains
in FreeLang; these operations do not embed JavaScript runtime source.

`(j-host-raw-fn (fn [a b] body))` takes exactly one function literal and emits its
parameters without FreeLang's ordinary browser `=null` defaults. Omitted or
explicitly undefined parameters stay undefined. `j-host-arguments` obtains the
actual function argument array and `j-host-this` the actual method receiver.
The raw function keeps the normal resolver's lexical binding behavior.
`j-host-mod` uses the engine's `%` operator, including signed zero and JavaScript
number coercion; it is not approximated by floating arithmetic. `j-host-spread`
uses the actual iterable protocol.

These primitives are available only in explicitly supplied trusted runtime
input to `--browser-runtime-js`. Ordinary user programs still reject `j-host-*`
and private core names. Ordinary `fn` retains its existing defaults. The shared
printer's ordinary output is compared against the accepted compiler corpus.
The new grammar does not change the Native value ABI or bootstrap seed, but it
changes the C runtime's browser backend: compiler ELF, archive and headers must
be rebuilt together. Reusing the preceding archive would omit these operations.

The consumer is the frontend's separate FreeLang headless adapter. This is not
a claim of complete Island compatibility, API/test gate/default development
migration, current whole DB qualification or commercial readiness. The full
objective remains active, commercial HOLD, production deployment NOT_RUN.
