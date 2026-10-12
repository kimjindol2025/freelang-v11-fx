# Phase1 6항 작업보고서 · 2026-10-13

**저장소:** `/home/kim/kim/platform/freelang-v11-fx`  
**브랜치:** `fx-server-update-2026-10-05`  
**마감문서:** [`PHASE1-SIX-ITEMS-CLOSEOUT-2026-10-13.md`](PHASE1-SIX-ITEMS-CLOSEOUT-2026-10-13.md)  
**지시 원칙:** 한 항씩 · PASS→로컬 커밋 · push/cgc-bin은 명시 승인만 · AFJ/Script/FX2/3/v11 미수정 · additive/warn · 보고 한국어

---

## 1. 목적

AI–언어 간격 6항을 FX(네이티브)에 **고정점을 지키며** 착지한다.

- 강제 실험은 AFJ 먼저, FX는 병행 API·lint·문서·함정 1개씩.
- Phase0 문서·lint 이후 Phase1 실제 수정 ①–⑥.

---

## 2. 커밋 타임라인

| # | 커밋 | 요약 | 고정점 SHA |
|---|------|------|------------|
| ① | `09e12d6` | ERROR-SCHEMA uncaught 한 줄 (`runtime/error.c`) | `01cf5f5b…` 유지 |
| ② | `60623f1` | `*_result` 5개 + `fl_result_ok/fail` + lint `[result-nil-http]` | `01cf5f5b…` 유지 |
| ③ | `20cf270` | `fl-build --bundle=core,net,db,mail` (기본 풀셋 유지) | `01cf5f5b…` 유지 |
| ④ | `e3b30fe` | cgc kebab naming warn + `--strict-names` | `01cf5f5b…` → `a3f41b5e…` |
| ④b | `f6a5e9b` | kebab 닫힌 목록 34 (`FX-KEBAB-API-LIST.airc`) | `a3f41b5e…` → `f4685b6f…` |
| ⑤ | `4185c29` | fl-lint `[effect-purity]` pure vs net/db | `f4685b6f…` 유지 |
| ⑥ | `29004ae` | 남은 3 trap relocate/archive · active 0 | `f4685b6f…` 유지 |

**원격:** ①–④는 origin(Forgejo) push 완료. **④b+⑤+⑥는 local ahead 3** (푸시 대기).

**공유 cgc-bin:** Phase1 구간 교체 없음. 설치본은 이전 승인분(`a19ccd99…` / EXPECT `01cf5f5b…`) 유지. ④b 이후 소스 SHA는 `f4685b6f…`.

---

## 3. 항목별 요지

### ① ERROR-SCHEMA
- Uncaught 시 `파일:줄 | 종류 | 값 | 이유 | 고치는 법` 한 줄 선출력.
- 회귀: `tests/six/run-error-oneline.sh`

### ② Result `*_result`
- `http_get_result` / `file_read_result` / `env_get_result` / `nth_result` / `shell_run_result`
- 성공 `[v nil]` · 실패 `[nil msg]`. 기존 빌트인 시맨틱 유지.
- 회귀: `tests/six/run-result-api.sh`

### ③ bundle
- opt-in `--bundle`. 기본 링크 풀셋. db/mail stub.
- 회귀: `tests/six/run-bundle.sh`

### ④ / ④b kebab
- 기본 warn · `--strict-names`에서 reject.
- ④b: 접두 매칭 폐기 → 닫힌 목록 34. 사용자 helper(`json-ok` 등) 오탐 제거.
- 회귀: `tests/six/run-kebab-names.sh`

### ⑤ effect-purity
- `@effect pure`인데 본문이 FX-BUILTIN-EFFECTS net/db 호출 → 경고.
- 문법 추가 없음. `scripts/fl-lint-effect-purity.py`
- 회귀: `tests/six/run-effect-anno.sh`

### ⑥ traps (증거 기반 재분류)
정책: 「증거대로 재분류·아카이브」 (네이티브 set_interval 구현 거부).

| 트랩 | 실측 | 처분 |
|------|------|------|
| set-interval-swallow | link `undefined reference to set_interval` | RELOCATED `ops-set-interval-host` |
| defn-throw-rerun | `caught=first` / `calls=1` | ARCHIVED |
| mariadb-count-float | `fetch_rows` → `fl_str_val`; `(= "0" 0)=false` | ARCHIVED (string 계약) |

---

## 4. 게이트 증거 (⑥ 직후)

```text
verify-native-bootstrap: STAGE2_EQ_STAGE3=PASS
  STAGE1/2/3_SHA=f4685b6fd03b59496acdf28b8069160ed9fc7ebb9b0953d2d1e1f354591d0c33

verify-fixpoint: SELF_HOST_FIXED_POINT=PASS
  GEN_A_EQ_GEN_B_EQ_GEN_C=PASS
  GEN_*_SHA=f4685b6fd03b59496acdf28b8069160ed9fc7ebb9b0953d2d1e1f354591d0c33

tests/traps/run-*.sh: 14/14 PASS (TRAPS_FAIL=0)
  포함: SET_INTERVAL_UNLINKED / DEFN_THROW_RERUN / MARIADB_COUNT_STRING

tests/six/run-*.sh: 5/5 PASS
  BUNDLE / EFFECT_ANNO / ERROR_ONELINE / KEBAB_NAMES / RESULT_API
```

스모크 기준선: **9/10** (`apps/fx-short`만 FAIL · 본 트랙 제외).

---

## 5. 주요 변경 파일 (⑥ 중심)

- `FX-TRAPS.airc` — trap_count 0 · archive 2 · relocated 1
- `docs/FX-OPS-CHECKLIST.md` — `ops-set-interval-host`
- `docs/decisions/FX-TRAPS-RECLASS-PHASE0.md` · `SIX-ITEMS-BY-DIALECT.md`
- `fx-std/safe.fl` — set-interval 범위 주석 (호스트 전용)
- `runtime/mariadb.c` — fetch_rows string 계약 주석
- `tests/traps/trap-{set-interval-unlinked,defn-throw-rerun,mariadb-count-string}.fl` + run-*.sh
- `FIXPOINT_LOG.md` · `SPEC.airc`

---

## 6. 미결 (보스 승인 대기)

1. **push** `f6a5e9b` + `4185c29` + `29004ae` (+ 본 마감 커밋) → Forgejo `origin` ± GitHub `github`
2. **cgc-bin 교체** — EXPECT_SHA를 `f4685b6f…`로 맞출지 승인 후 `scripts/upgrade-cgc-bin.sh`
3. typed MariaDB coerce (libmariadb-dev / MYSQL_FIELD) — 의도적 연기
4. FX 네이티브 timer API — 새 id로 실측 후 등록 (relocated id 재사용 금지)

---

## 7. 다음 에이전트

```text
너는 Phase1 ①–⑥ 로컬까지 끝난 상태다.
task-phase1-six-items-complete-push-cgc-pending 만 본다.
push·cgc-bin은 보스 지시 전에는 하지 않는다.
```

진입: `print-hot.py` on product `SPEC.airc` only.
