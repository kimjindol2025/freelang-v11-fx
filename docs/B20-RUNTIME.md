# B-20R runtime validation

## Start point and dependency

- FX checkout: branch `fix/b20-runtime`, HEAD `c99b6f7833e7c8e73cfd18946867d927e4228146`, 11 pre-existing dirty paths. No pre-existing changes were staged, committed, or overwritten.
- Seed C SHA-256 at start and end: `f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c`.
- Temporary curl was used (built directly for this window, signature previously verified, this build only): `PREFIX_ID=b20r-curl-8.9.1.ARZB6s`; `curl-config --version` reported `libcurl 8.9.1`.
- The private `curl/curl.h` SHA-256 matched `09512c1a42879cccf9a1c5f976e6de325f707eefa8ddbb90109896074078fd16`; the private libcurl SHA-256 matched `27c543faf27ad4745a1d26cc06567acdac9f46e2833410a57949b1cc241ecd36`.
- The build used only process-scoped `CPATH`, `LIBRARY_PATH`, `LD_RUN_PATH`, and `LD_LIBRARY_PATH`; prefix path is redacted. Replace this temporary curl with an approved official development package before future builds.

## Results

| Gate | Result |
| --- | --- |
| E10 bootstrap fixed point | PASS, exit 0; Stage2 = Stage3 = Stage4 = `f38f373890d0898552cbe89768b992f934e9c4a567bf27cdf66248a3c1b5cb81`; static audit and array loop PASS; residual process count 0 |
| Fresh Native compiler | SHA-256 `275c280fb143e14a36b1f517e832dc4505b4ef41c62900e9e32a144b95635967` |
| FX seed | Start and end SHA-256 matched the pinned value above |
| Regression | `verify-l06-regressions.sh` exit 0; 10/10 legacy runtime tests and L07 binary/runtime regressions PASS. Full `verify-e08-completion.sh` did not pass: it stopped at sanitizer link because `/usr/lib64/libasan.so.8.0.0` is unavailable. |
| Leak checks | Not run: the E08 sanitizer link failed before leak programs ran. The 12/12 leak gate is not established. |
| API 20/20 | Not rerun in this batch. The last recorded B17R result is 20/20 and is historical evidence only. |
| Map 100,000 keys | PASS; fixed map cases 8/8; `0.162416478` seconds, values verified |
| Native fl-git matrix | 26/26 PASS using the fresh compiler, current FX runtime objects, and launcher-equivalent environment for `tools/fl-check.fl`. `src/tui.fl` and `web/pages/_order-api.flx` both compiled, linked, and ran with exit 0. fl-git remained read-only and clean. |
| Blog restore and Seoul timezone | Not established. Focused tests generated C (exit 0) but linking failed on symbols supplied by the absent DB95 `native-store.fl` dependency. Full `app/main.fl` C generation also stopped with exit 1 on that missing module. No submodule fetch or source substitution was performed. |
| Main C memory/time gate | Not established; the blog main C generation stopped before producing C, so no valid memory/time result is available. |
| PRD | Last recorded 2/24; the two blog items were not raised because their focused tests could not link. |
| Schedule timing only | 100: `0.000213843s`; 1000: `0.001438627s`; 4096: `0.005809421s`. Each count was measured once with the existing `schedule-at` path; no 4097 limit assertion was run here. The measured loop is scheduling plus ordered-list insertion under `timer_mutex`; the same-due list scan is the likely cost center. |
| `-lcrypto` | `runtime/core.o` has 11 undefined crypto symbols, including `CRYPTO_memcmp`, EVP, HMAC, and SHA256 symbols, so linking core requires `-lcrypto`. No dedicated core-only shell link test was found; the existing bootstrap link scripts include `-lcrypto`. |
| Ports | No server was started; ports 40850/40851 had zero listeners at final verification. |

The release gates are incomplete because the sanitizer development runtime and the blog DB95 source dependency are unavailable. No commit or push was made; the document is left uncommitted with the 11 pre-existing FX changes preserved.
