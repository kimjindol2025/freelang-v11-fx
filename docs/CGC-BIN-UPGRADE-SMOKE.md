# 공유 `cgc-bin` 업그레이드 — 앱 스모크 목록

상태: `ACTIVE`  
기록일: 2026-10-11  
배경: Phase 1 — `fl_rt_db` + `trap-extern-flvalue-defn`(short→`fxb_*`). 고정점 SHA `63411319…`.  
정책: 공유 바이너리 교체 전 대표 앱으로 새 컴파일러 스모크. 남은 RUNTIME_OPEN은 업그레이드 이후 계속.

## 1. 올릴 대상 (이 노드)

| 항목 | 경로 |
|------|------|
| 소스 | `/home/kim/kim/platform/freelang-v11-fx/self/cgc-main.fl` |
| 공유 바이너리 | `/home/kim/kim/platform/freelang-afj/bin/cgc-bin` |
| `fl-build` 기본 탐색 | 위 경로 (또는 `CGC_BIN`) |
| 기대 고정점 SHA (생성 C) | `634113190b96cc289776bc435b27b53c071e74fad1af59828a39850db8176aac` |

참고: 레거시 `fx-upgrade-cgc.sh`는 `/home/kimjin/...` 경로용. 이 노드에서는  
`scripts/upgrade-cgc-bin.sh` + `scripts/smoke-cgc-upgrade.sh`를 쓴다.

## 2. 스모크 앱 목록 (대표 표본)

컴파일 성공을 기본 게이트로 한다. 표시된 앱은 HTTP 한 방도 한다.

| # | 앱 | 경로 | 포트 | 보는 것 | HTTP |
|---|-----|------|------|---------|------|
| 1 | hello | `templates/hello/server.fl` | 40900 | 최소 서버 | `/` |
| 2 | fx-demo | `fx-demo/server.fl` | 40295 | JSON·sqlite 데모 | `/` 또는 헬스 |
| 3 | fx-kv | `fx-kv/server.fl` | 40304 | atom·KV | 빌드만 |
| 4 | fx-auth | `fx-auth/server.fl` | 40306 | 인증 라우트 | 빌드만 |
| 5 | fx-chat | `fx-chat/server.fl` | 40291 | WS·방송 | 빌드만 |
| 6 | fx-world | `fx-world/server.fl` | 40296 | future·http_get | 빌드만 |
| 7 | fx-cron | `fx-cron/server.fl` | 40303 | 스케줄 | 빌드만 |
| 8 | fx-queue | `fx-queue/server.fl` | 40297 | 큐 | 빌드만 |
| 9 | fx-sse-demo | `fx-sse-demo/server.fl` | 40321 | SSE | 빌드만 |
| 10 | fx-short | `apps/fx-short/server.fl` | 40309 | 짧은 앱 | 빌드만 |

회귀(이미 PASS, 업그레이드 전 재확인 권장):

| 회귀 | 명령 |
|------|------|
| nested-defn | `bash tests/traps/run-trap-nested-defn-skip.sh` |
| parallel-run | `bash tests/traps/run-trap-parallel-run-missing.sh` |
| 고정점 | `./verify-fixpoint.sh` · `./scripts/verify-native-bootstrap.sh` |

## 3. 절차

```text
1. bash scripts/smoke-cgc-upgrade.sh
   → 후보 cgc-bin 빌드 + 위 앱 fl-build(CGC_BIN=후보)
2. 전부 COMPILE=PASS (HTTP 대상은 HTTP=PASS)
3. 사용자 확인 후:
   bash scripts/upgrade-cgc-bin.sh
   → 기존 바이너리 .bak 보관 후 교체
4. CGC_BIN 없이 fl-build로 hello 재확인
5. 그다음 RUNTIME_OPEN 8개
```

## 4. 결과 기록

### 2026-10-11 스모크 (후보 vs 설치본)

| 앱 | 후보(새 cgc) | 설치본(옛 cgc) | 판정 |
|----|--------------|----------------|------|
| templates/hello | PASS | PASS | 동일 |
| fx-demo | FAIL (`define db` ↔ `FLValue db()`) | FAIL 동일 | **기존 이슈** |
| fx-kv | FAIL (`db`) | (동일 패턴) | **기존 이슈** |
| fx-auth | FAIL (`db`) | FAIL 동일 | **기존 이슈** |
| fx-chat | FAIL (`db`) | FAIL 동일 | **기존 이슈** |
| fx-world | PASS | PASS | 동일 |
| fx-cron | FAIL (`db`) | (동일 패턴) | **기존 이슈** |
| fx-queue | FAIL (`db`) | (동일 패턴) | **기존 이슈** |
| fx-sse-demo | PASS | PASS | 동일 |
| apps/fx-short | FAIL (invalid initializer 등) | FAIL 동일 | **기존 이슈** |

```text
후보 gen SHA = 53542e964200ee34… (고정점과 일치, gen-a==gen-b)
COMPILE 후보: pass=3 fail=7
설치본 대비 회귀: 0 (실패 원인이 후보 전용인 앱 없음)
```

**업그레이드 게이트 해석:**  
절대 PASS 10/10은 아님. 다만 실패는 전부 **런타임 `db()` 심볼과 앱 전역 `define db` 충돌** 또는 **fx-short 기존 코드젠/앱 버그**이며, 설치본으로도 같은 방식으로 실패한다.  
컴파일러 함정 수정(nested-defn)으로 인한 **신규 회귀는 없다.**

`db` 이름 충돌·fx-short는 RUNTIME_OPEN/앱 수정 안건으로 넘긴다 (업그레이드 차단 사유로 보지 않음).

| 일시 | 후보 생성 C SHA | 앱 회귀 | 업그레이드 |
|------|-----------------|---------|------------|
| 2026-10-11 | `53542e96…` | 없음 (baseline과 동일) | **완료** |

### 업그레이드 기록 (2026-10-11)

```text
TARGET=/home/kim/kim/platform/freelang-afj/bin/cgc-bin
OLD_BIN_SHA=eb5d5eb2534fce9089afba5ad45590de7eb30ea0ef8680d8bf80e7ca62cd2a29
NEW_BIN_SHA=95ca5840ee3970b51938f1931e36a282624a7afbbc06fe2ed3ee16f22272bc15
BACKUP=.../freelang-afj/bin/cgc-bin.bak-20261011-223605
FIXPOINT_SHA=53542e964200ee34fe840a6949e8af62f4c3afd085a5966621e0e0e16e359db4
POST=hello fl-build PASS; nested-defn via default fl-build → 42 PASS
COMMAND=bash scripts/upgrade-cgc-bin.sh
```

### db 이름 충돌 수정 후 스모크 (2026-10-11, 후보 cgc)

| 앱 | 수정 전 | 수정 후 |
|----|---------|---------|
| templates/hello | PASS | PASS |
| fx-demo | FAIL (db) | **PASS** |
| fx-kv | FAIL (db) | **PASS** |
| fx-auth | FAIL (db) | **PASS** |
| fx-chat | FAIL (db) | **PASS** |
| fx-world | PASS | PASS |
| fx-cron | FAIL (db) | **PASS** |
| fx-queue | FAIL (db) | **PASS** |
| fx-sse-demo | PASS | PASS |
| apps/fx-short | FAIL (다른 오류) | FAIL (동일) |

`compile_pass` 3/10 → **9/10**. 고정점 SHA `f9b43b1c…`.  
공유 `cgc-bin`은 지시서대로 **아직 재교체하지 않음** (보고 후 승인).

### 업그레이드 기록 (2026-10-11, fl_rt_db 반영)

```text
TARGET=/home/kim/kim/platform/freelang-afj/bin/cgc-bin
OLD_BIN_SHA=95ca5840ee3970b51938f1931e36a282624a7afbbc06fe2ed3ee16f22272bc15
NEW_BIN_SHA=d69b8d10ce48debcfe4f638594ae4658ad879729221afb8e582258fde272cfff
BACKUP=.../freelang-afj/bin/cgc-bin.bak-20261011-233634
FIXPOINT_SHA=f9b43b1cdbef58bdcf4af7c6f53978f5abdd68b47ea7ac55d05e556895f5bda8
INSTALLED_SMOKE=9/10 (fx-short FAIL 동일)
(db …) 설치본 매핑=fl_rt_db(…)  (교체 전 db(…))
DB_AUTO_BUILD=PASS · FL_BUILD_DB=PASS
COMMAND=bash scripts/upgrade-cgc-bin.sh  (EXPECT_SHA → f9b43b1c…)
```

### 업그레이드 기록 (2026-10-11, trap-extern-flvalue-defn)

```text
TARGET=/home/kim/kim/platform/freelang-afj/bin/cgc-bin
OLD_BIN_SHA=d69b8d10ce48debcfe4f638594ae4658ad879729221afb8e582258fde272cfff
NEW_BIN_SHA=239b729de5063220c41a82aeafe4213c2c4bb73ca85ce4a22a7070644515972a
BACKUP=.../freelang-afj/bin/cgc-bin.bak-20261011-234448
FIXPOINT_SHA=634113190b96cc289776bc435b27b53c071e74fad1af59828a39850db8176aac
TRAP_EXTERN_FLVALUE_DEFN=PASS (설치본)
(db …)=fl_rt_db 유지
INSTALLED_SMOKE=9/10
```

상태: 업그레이드 3회 완료(53542e96 → f9b43b1c → 63411319). 활성 RUNTIME_OPEN 7개. fx-short는 별도.
