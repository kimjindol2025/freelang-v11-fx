# J-1 fixed fixtures

Before implementation, commit these outcomes unchanged. Nine cases per API,
100 repetitions per case, native ASan/UBSan/LSan with empty stderr required.
Cases: trailing error after array; trailing error after nested arrays/object;
invalid character midway; invalid character inside outer array; truncated array;
truncated outer array; invalid escape; valid nested arrays/object; empty array.

The legacy API intentionally accepts several malformed inputs and returns its
existing partial/nil values. The v11 API rejects them. Preserve both contracts.
Errors must release all in-progress builders, including nested arrays. Successful
results must be ordinary immutable vectors. Sanitizers detect abandoned heap
builders after arena reset and process cleanup; do not suppress reports.

`measure.sh` runs each API/size in a new process. Limits stay identical before and
after. Parse time uses CLOCK_MONOTONIC; process peak RSS and process time use GNU
time. A nonzero MEASUREMENT_EXIT is a failed measurement, never a success time.
