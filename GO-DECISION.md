# 운영 전환 최종 심사 — 판정 기록 (2026-06-08)

대상: 운영 `freelang-v11`(11.5.1) → 원격 `a08ae05`(11.7.11) 통합 전환.
방식: 현재 확보된 증거만으로 판정 (추가 분석·구현·테스트 없이).

## [판정] CONDITIONAL GO

코드 준비 완료, **배포 전 체크리스트 수행 후 진행**.

## [판정 이유] — 검증된 사실만

- **코드 준비 = 완료, 중대 결함 없음**: build OK · scenario 15/15 PASS · gateway 22/22 PASS ·
  check-apps-compat 검수 완료 · 영향 앱(fl-watchdog/fl-truth/discord/square) 운영 11.5.1 대비 **퇴행 0**.
  → NO-GO 사유(코드/검증 결함) 부재.
- discord(`fl-require "wsc"`)·square(`load "src/state.fl"`) 2건은 **신규 회귀 아님**
  (양쪽 런타임 동일 증상) → 코드 결함으로 계상 안 됨.
- **배포·롤백 준비 = 미완**: 롤백 절차 미문서화, 운영 백업 확보 여부 미검증.
  코드 결함이 아니라 **절차 미비** → GO 불가, NO-GO도 불가.
- ⇒ "코드 준비 완료 + 배포 전 체크리스트 수행 후 진행" = **CONDITIONAL GO** 정의에 부합.

## [배포 전 필수 체크리스트] (최대 5)

1. 운영 서버 현 `bootstrap.js` **백업 확보** (bootstrap 소실 사고 전례 대응)
2. **롤백 절차 문서화 + 1회 리허설** (백업 → 복원 → 정상 확인 경로)
3. `git pull` 직접 금지 — `pull → npm install → build → 산출물 검증` 후 교체 (0바이트 함정 차단)
4. **단계적 배포** (카나리 1앱 → 소수 → 전체), 67앱 일괄 교체 금지
5. 운영 서버 **빌드 환경 가용성 확인** (node/esbuild/node_modules)

## [배포 후 즉시 확인 항목] (최대 5)

1. 카나리 앱 정상 기동 여부
2. 회귀 함수 실경로 동작 (`mod`/`includes?`/`includes-item` — fl-watchdog·discord 등)
3. 대표 앱 런타임 응답 (gateway 등) 정상
4. 67앱 기동 상태 (online 수 전/후 비교)
5. 에러 로그 모니터링 (`실행 오류`/`Function not found`)

## 증거 분리

- [검증됨] baseline a08ae05 · build OK · npm install+build 재현 · scenario 15/15 · gateway 22/22 ·
  check-apps-compat · 영향 앱 비교 · 11.5.1 대비 퇴행 0 · TLS 추가 · gogs 기록 동결 · 선결과제 ①② 완료
- [검증됨-제한] discord(wsc)·square(load) = 양쪽 동일 증상(신규 회귀 아님)
- [미검증] 운영 서버(166/73) 배포 · 운영 실부하 · 장기 안정성 · 운영 bootstrap 상태 · 운영 빌드 환경
- ⚠️ 운영 서버 상태는 추정하지 않음. CONDITIONAL GO 의 "조건" = 위 미검증 항목을 배포 전
  체크리스트로 확보하는 것.

관련: `INTEGRATION-BASELINE.md`(0절 a08ae05 갱신), `patches/DEPRECATED.md`(patch 폐기).
