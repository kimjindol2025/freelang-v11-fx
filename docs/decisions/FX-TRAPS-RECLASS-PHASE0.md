# FX Phase 0 — 함정 재분류표

상태: `IN_PROGRESS`  
기록일: 2026-10-11  
상위 결정: [`SIX-ITEMS-BY-DIALECT.md`](SIX-ITEMS-BY-DIALECT.md)  
정본 함정: `/home/kim/kim/platform/freelang-v11-fx/FX-TRAPS.airc` (`trap_count: 27`)  
참고: `CGC-IMPROVEMENT-ROADMAP.md` (RESOLVED 주장 — 정본 미반영)

## 1. 목적

함정 목록을 **문서에만 쌓지 않고**, 다음 칸으로 나눠 줄여 간다.

| 상태 코드 | 의미 | 다음에 할 일 |
|-----------|------|----------------|
| `RESOLVED_CANDIDATE` | 이번 세션에서 실행으로 완화 확인 | 회귀 테스트 1개 고정 후 `RESOLVED` → 정본에서 제거 또는 archive |
| `RESOLVED_CLAIMED` | 로드맵만 RESOLVED. `FX-TRAPS.airc` 미갱신 | **재실측** 후 Candidate 또는 OPEN |
| `NEEDS_REVERIFY` | 로드맵도 미확인 | 실측 |
| `COMPILER_OPEN` | cgc 수정 필요 (알려진 잔여) | Phase 1 — 함정 1개씩 + 고정점 게이트 |
| `RUNTIME_OPEN` | runtime/stdlib 수정·문서화 필요 | Phase 1 |
| `DOCS_PROCESS` | 언어 시맨틱이 아님 (노드·체크리스트·조사범위) | 체크리스트로 옮기고 함정 목록에서 제외 후보 |
| `OPEN` | 아직 Phase 0 증거 없음 | 분류·실측 |

Phase 0는 **재분류·문서 동기화**다.  
`FX-TRAPS.airc`의 `trap_count`를 지금 바로 숫자만 깎지 않는다.  
`RESOLVED`로 올릴 때마다 회귀 테스트 경로를 남긴 뒤 정본을 갱신한다.

## 2. 재분류표 (27개)

| ID | lint | Phase 0 상태 | locus | 근거·다음 |
|----|------|--------------|-------|-----------|
| `trap-defn-do` | false | **RESOLVED** | compiler | 회귀 PASS `tests/traps/run-trap-defn-do.sh` → `step-a`/`step-b`/`42`. `@archive` |
| `trap-count-type` | false | **RESOLVED** | compiler/runtime | 재실측 PASS `tests/traps/reverify-claimed.fl` — count→4 OK|
| `trap-kebab-symbol` | false | **RESOLVED** | compiler/runtime | 재실측 PASS `tests/traps/reverify-claimed.fl` — hello-world kebab defn OK|
| `trap-str-includes-int` | false | **RESOLVED** | compiler/runtime | 재실측 PASS `tests/traps/reverify-claimed.fl` — now boolean true/false (not int 0/1)|
| `trap-throw-invalid-initializer` | false | **RESOLVED** | compiler/runtime | 재실측 PASS `tests/traps/reverify-claimed.fl` — throw/catch OK|
| `trap-free-locale` | false | **RELOCATED** | ops | → [`FX-OPS-CHECKLIST.md#ops-free-locale`](../FX-OPS-CHECKLIST.md#ops-free-locale) · `FX-TRAPS` `@relocated` |
| `trap-runtime-header` | false | **RELOCATED** | ops | → [`FX-OPS-CHECKLIST.md#ops-runtime-header`](../FX-OPS-CHECKLIST.md#ops-runtime-header) · `FX-TRAPS` `@relocated` |
| `trap-extern-flvalue-defn` | false | **RESOLVED** | compiler | A안 cgc short→`fxb_*`. `run-trap-extern-flvalue-defn.sh` → `ok-42`. SHA `63411319…` |
| `trap-server-json-string` | false | **RESOLVED** | runtime | HTTP 스모크 PASS `/map` `/str` `/vec` 모두 200+JSON. `runtime/http.c` 자동 stringify. `bash tests/traps/run-trap-server-json-string.sh` |
| `trap-server-html-quote` | false | **RESOLVED** | compiler/runtime | 재실측 PASS `tests/traps/reverify-claimed.fl` — HTML " in FL string OK|
| `trap-lambda-capture` | false | **RESOLVED** | compiler | Phase 1 PASS `run-trap-lambda-capture.sh` → 15/30/33. CAP/env-stack. SHA `01cf5f5b…` |
| `trap-mariadb-count-float` | false | **RESOLVED** | runtime | Phase1 ⑥ archive — cells are strings (`fl_str_val`); `run-trap-mariadb-count-string.sh`. Typed coerce deferred |
| `trap-replace-all-rename` | false | **RELOCATED** | ops | → [`FX-OPS-CHECKLIST.md#ops-replace-all-rename`](../FX-OPS-CHECKLIST.md#ops-replace-all-rename) · `FX-TRAPS` `@relocated` |
| `trap-string-literal-1024` | false | **RESOLVED** | compiler/runtime | 재실측 PASS `tests/traps/reverify-claimed.fl` — 1100B literal OK|
| `trap-cgc-stale-node` | false | **RELOCATED** | ops | → [`FX-OPS-CHECKLIST.md#ops-cgc-stale-node`](../FX-OPS-CHECKLIST.md#ops-cgc-stale-node) · `FX-TRAPS` `@relocated` |
| `trap-set-interval-swallow` | true | **RELOCATED** | ops/host | Phase1 ⑥ → [`FX-OPS-CHECKLIST.md#ops-set-interval-host`](../FX-OPS-CHECKLIST.md#ops-set-interval-host) · FX unlinked; swallow=Node/AFJ |
| `trap-nth-out-of-bounds` | false | **RESOLVED** | runtime | Phase 1 PASS `run-trap-nth-out-of-bounds.sh` → oob-hi/neg/empty=nil. FX nil 관례 유지(Result는 AFJ 먼저). SHA 유지 `01cf5f5b…` |
| `trap-try-map-literal` | false | **RESOLVED** | compiler/runtime | 재실측 PASS `tests/traps/reverify-claimed.fl` — try {..} OK|
| `trap-nested-defn-skip` | false | **RESOLVED** | compiler | Phase 1 PASS `run-trap-nested-defn-skip.sh` → `42`. hoist fwd decls. 고정점 SHA `53542e96…` |
| `trap-inc-dec-missing` | false | **RESOLVED** | compiler/runtime | 재실측 PASS `tests/traps/reverify-claimed.fl` — inc/dec OK|
| `trap-cache-cgc-missing` | false | **RESOLVED** | compiler/runtime | 재실측 PASS `tests/traps/reverify-claimed.fl` — fx-std/cache.fl OK|
| `trap-http-get-cgc-mapping` | false | **RESOLVED** | runtime | Phase 1 PASS `run-trap-http-get-cgc-mapping.sh` → map status/body/headers. `http_client.c` http_get; emit≠fl_http_get. SHA 유지 `01cf5f5b…` |
| `trap-parallel-run-missing` | false | **RESOLVED** | runtime | Phase 1 PASS `run-trap-parallel-run-missing.sh`. `fl_run_parallel`+`fx-std/parallel.fl`. SHA 유지 `53542e96…` |
| `trap-atom-deref-gc` | false | **RESOLVED** | runtime | Phase 1 PASS `run-trap-atom-deref-gc.sh` → hello/1. `fl_heap_copy` on deref/reset. SHA 유지 `01cf5f5b…` |
| `trap-defn-throw-rerun` | false | **RESOLVED** | runtime | Phase1 ⑥ archive — FX `calls=1` no re-run; `run-trap-defn-throw-rerun.sh`. v11 claim → AFJ |
| `trap-defn-docstring-cgc` | false | **RESOLVED** | compiler/runtime | 재실측 PASS `tests/traps/reverify-claimed.fl` — docstring+body→42 OK|
| `trap-v11-hof-closure-bug` | false | **RELOCATED** | ops | → [`FX-OPS-CHECKLIST.md#ops-v11-hof-scope`](../FX-OPS-CHECKLIST.md#ops-v11-hof-scope) · `FX-TRAPS` `@relocated` |

## 3. 집계 (Phase 0 시점 → Phase1 ⑥ 후)

| 상태 | 개수 |
|------|------|
| RESOLVED (archive) | 14 → **22** |
| RELOCATED (ops checklist) | 5 → **6** |
| COMPILER_OPEN | 0 |
| RUNTIME_OPEN | 8 → **0** |
| **active 함정 (고칠 것)** | **0** (2026-10-13 Phase1 ⑥) |

목록을 0으로 만드는 경로:

1. ~~`DOCS_PROCESS` 5개~~ → **완료** [`docs/FX-OPS-CHECKLIST.md`](../FX-OPS-CHECKLIST.md) + `FX-TRAPS` `@relocated` (검색용 포인터 유지)
2. ~~`RESOLVED_CLAIMED` / 확인 함정~~ → archive 12
3. ~~남은 COMPILER_OPEN + RUNTIME_OPEN~~ → Phase 1에서 1개씩 **완료** (⑥ set-interval relocate / throw-rerun + mariadb archive)

### DOCS_PROCESS 이동 색인 (2026-10-11 + ⑥)

| 옛 id | 새 위치 |
|-------|---------|
| `trap-free-locale` | `docs/FX-OPS-CHECKLIST.md#ops-free-locale` |
| `trap-runtime-header` | `docs/FX-OPS-CHECKLIST.md#ops-runtime-header` |
| `trap-replace-all-rename` | `docs/FX-OPS-CHECKLIST.md#ops-replace-all-rename` |
| `trap-cgc-stale-node` | `docs/FX-OPS-CHECKLIST.md#ops-cgc-stale-node` |
| `trap-v11-hof-closure-bug` | `docs/FX-OPS-CHECKLIST.md#ops-v11-hof-scope` |
| `trap-set-interval-swallow` | `docs/FX-OPS-CHECKLIST.md#ops-set-interval-host` (2026-10-13) |


### 회귀 증거 — RESOLVED_CLAIMED 10개 (2026-10-11)

```text
COMMAND=bash tests/traps/run-reverify-claimed.sh
RESULT=REVERIFY_CLAIMED=PASS
PASS=10 FAIL=0
NOTE=trap-str-includes-int now returns boolean (old safe_pattern (= … 1) obsolete)
```

### 회귀 증거 — `trap-defn-do`

```text
COMMAND=bash tests/traps/run-trap-defn-do.sh
RESULT=TRAP_DEFN_DO=PASS
EXPECT=step-a / step-b / 42
DATE=2026-10-11
```

## 4. Phase 0 문서 수정

1. ~~defn 다중 본문 거짓 규칙~~ — 완료 (입문 문서·스킬)
2. ~~`FX-TRAPS.airc` Phase 0 메타~~ — 완료 (`trap_count: 16`, archive 11)
3. 이름 방언 카드·오류 스키마·번들 지도 — Phase 0 나머지 (별도)
4. ~~obsolete locks~~ — `lock-no-throw` / `lock-str-includes` retired (함정 RESOLVED에 맞춤)

## 5. 다음 실측 순서 (권장)

1. ~~`trap-defn-do`~~ — 완료  
2. ~~`RESOLVED_CLAIMED` 10개~~ — 완료  
3. ~~`trap-server-json-string`~~ — 완료 (HTTP 스모크 PASS + 고정점 PASS)  
4. ~~`DOCS_PROCESS` 5개~~ — 완료 → `docs/FX-OPS-CHECKLIST.md`  
5. Phase 1: ~~`trap-nested-defn-skip`~~ · ~~`trap-parallel-run-missing`~~ 완료 (COMPILER_OPEN 0)  
6. **공유 `cgc-bin` 업그레이드** — 아직 하지 않음. 보스 승인 후  
7. ~~Phase 1 나머지 RUNTIME_OPEN~~ → **⑥ 완료** (active 0). set-interval RELOCATED; throw-rerun·mariadb ARCHIVED

### Phase 1 ⑥ 증거 — 남은 RUNTIME_OPEN 3개 닫음 (2026-10-13)

```text
POLICY=증거대로 재분류·아카이브 (보스 승인)
trap-set-interval-swallow → RELOCATED ops-set-interval-host
  COMMAND=bash tests/traps/run-trap-set-interval-unlinked.sh → PASS
  NOTE=undefined reference to set_interval; swallow=Node/AFJ bootstrap
trap-defn-throw-rerun → ARCHIVED
  COMMAND=bash tests/traps/run-trap-defn-throw-rerun.sh → PASS
  EXPECT=caught=first / calls=1
trap-mariadb-count-float → ARCHIVED (observed_now=string cells)
  COMMAND=bash tests/traps/run-trap-mariadb-count-string.sh → PASS
  NOTE=fetch_rows fl_str_val; (= "0" 0)=false
ACTIVE_TRAP_COUNT=0
```

### Phase 1 증거 — `trap-parallel-run-missing` (2026-10-11)

```text
COMMAND=bash tests/traps/run-trap-parallel-run-missing.sh
RESULT=TRAP_PARALLEL_RUN_MISSING=PASS
  run-parallel=2
  parallel-run=3
  parallel-map=60

FIXPOINT=./verify-fixpoint.sh → PASS
NATIVE=./scripts/verify-native-bootstrap.sh → STAGE2_EQ_STAGE3=PASS
SHA=53542e964200ee34fe840a6949e8af62f4c3afd085a5966621e0e0e16e359db4
NOTE=cgc-main.fl unchanged this cycle → SHA same as nested-defn fix
CGC_BIN_UPGRADE=deferred (user: after compiler traps + app smoke)
```

### Phase 1 증거 — `trap-nested-defn-skip` (2026-10-11)

```text
COMMAND=bash tests/traps/run-trap-nested-defn-skip.sh
RESULT=TRAP_NESTED_DEFN_SKIP=PASS (stdout 42)

FIXPOINT=./verify-fixpoint.sh → SELF_HOST_FIXED_POINT=PASS
NATIVE=./scripts/verify-native-bootstrap.sh → STAGE2_EQ_STAGE3=PASS
SHA_BEFORE=bd5c08189640c5a6bb6df3c3b386c2d2a6c7dc98ad06dc228641f0090eaa785b
SHA_AFTER =53542e964200ee34fe840a6949e8af62f4c3afd085a5966621e0e0e16e359db4
NOTE=SHA changed (expected); gen-a==gen-b==gen-c and stage2==stage3
```

### 회귀 증거 — `trap-server-json-string` (2026-10-11)

```text
COMMAND=bash tests/traps/run-trap-server-json-string.sh
RESULT=TRAP_SERVER_JSON_STRING=PASS
MAP=200|30|{"ok":true,"via":"map-direct"}
STR=200|29|{"ok":true,"via":"stringify"}
VEC=200|7|[1,2,3]
FIXPOINT=./verify-fixpoint.sh → SELF_HOST_FIXED_POINT=PASS
NATIVE=./scripts/verify-native-bootstrap.sh → STAGE2_EQ_STAGE3=PASS
SHA=bd5c08189640c5a6bb6df3c3b386c2d2a6c7dc98ad06dc228641f0090eaa785b
```

## 6. 변경 규칙

- 상태를 `RESOLVED`로 올릴 때: 재현 명령 + 기대 출력 + 테스트 경로를 본 문서 또는 `tests/`에 남긴다.
- `FX-TRAPS.airc`에서 항목을 지울 때: archive 절 또는 `verification_status: resolved` + 본 표 갱신.
- COMPILER_OPEN 수정은 `./scripts/verify-native-bootstrap.sh` PASS 없이 커밋하지 않는다.
