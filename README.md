# freelang-v11-fx

FL v11 **fx C 네이티브 런타임** + 앱 생태계. (원칙: 기록이 증명이다)
- **언어/빌드 입문**: `FX-FOR-CLAUDE.md` · **API 레퍼런스**: `CLAUDE.md` · **함정 명세**: `FX-TRAPS.airc`
- **빌드**: `bash fl-build.sh <app>/server.fl <out>` (이 노드 aarch64: `.fl-build-root.sh`)
- **문서**: `docs/{status,decisions,reports,architecture}/`

## ✅ C 셀프호스팅 고정점 — 공식 검증 완료

FreeLang C native compiler는 자기 자신의 소스를 연속 3세대 생성하고 동일한
SHA-256 산출물로 수렴한다. 검증기와 배열형 loop 회귀 테스트는 다음 파일에
있다.

```text
verify-fixpoint.sh
tests/fixpoint-loop-array.fl
```

검증 명령:

```bash
./verify-fixpoint.sh
```

검증 결과:

```text
gen-a → gen-b → gen-c       PASS
gen-a == gen-b == gen-c     PASS
array loop C 실행           PASS
clean checkout 재현         PASS
user-fns.c 누락 음성 테스트  FAIL_AS_EXPECTED
```

검증기는 저장소 기준 경로를 사용하고 `runtime/user-fns.c`를 포함한다.
`cgc-bin`은 교체하지 않으며, 라벨형 loop는 현재 FX AST 구조 범위 밖이다.

## 🗂 fx 앱 카탈로그

| 앱 | 포트 | 설명 |
|----|------|------|
| `fx-sqlite-browser` | 40286 | SQLite DB 브라우저 (테이블·CRUD·SQL 콘솔) ✓검증 |
| `fx-server-monitor` | 40288 | HTTP 서비스 health (Outside-In·가용성%) ✓검증 |
| `fx-live-monitor` | 40290 | 실시간 서비스 health (websocket) |
| `fx-chat` | 40291 | 실시간 채팅 |
| `fx-demo` | 40295 | fx 언어 데모 |
| `fx-world` | 40296 | 환율/날씨 API |
| `fx-queue` | 40297 | 작업 큐 |
| `fx-mail` | 40299 | 메일 발송 (sendmail) |
| `fx-notify` | 40300 | 이벤트 버스 알림 |
| `fx-monitor` | 40301 | 시스템 모니터 (Inside-Out·CPU/Mem/PM2) |
| `fx-deploy` | 40302 | 배포 엔진 |
| `fx-cron` | 40303 | 인터벌 스케줄러 |
| `fx-kv` | 40304 | Key-Value 스토어 |
| `fx-log` | 40305 | 중앙 로그 수집 |
| `fx-auth` | 40306 | 인증 서버 |
| `fx-proxy` | 40307 | 리버스 프록시 + 로드밸런서 |
| `fx-search` | 40308 | TF 역인덱스 검색 엔진 |
| `fx-short` | 40309 | URL 단축기 (`apps/`) |
| `fx-dashboard` | 40310 | 읽기전용 운영 콘솔 (7패널) ✓검증 |
| `fx-files` | — | 파일 탐색기 (CLI/FS) |

> 모니터 3종 = `fx-monitor`(Inside-Out) + `fx-server-monitor`·`fx-live-monitor`(Outside-In). 상세 `docs/decisions/MONITOR-AXES.md`.

## 구성

| 파일 | 내용 |
|------|------|
| `bootstrap.js` | 11.5.1 **patched 참조구현** — builtin first-class 일반화 + evalBuiltin 누락 case 복구. gateway demo **22/22 PASS** |
| `INTEGRATION-BASELINE.md` | 11.7.11 통합 조사 결론 — Source of Truth, 회귀 분석, 이식 목록, 결정 |
| `patches/711-regression-port.patch` | 최신 소스(11.7.11, base `b76ddf23`)에 적용할 회귀 수정 패치 |
| `patches/711-regression-port.base` | patch 기준 커밋 SHA |

## 핵심 결론 (INTEGRATION-BASELINE.md 요약)

- 원격 `freelang-v11` master(11.7.11)는 최신 소스 구조(src/ 537모듈)이나 **회귀 수정판이 아님** — 빌드된 11.7.11도 mod/swap!update/strlen/gateway 동일 회귀 보유.
- **현재 기준선 = 운영 11.5.1 + 이 리포의 fx 패치**(검증완료).
- 통합 명분 = 모듈화/빌드체계뿐(안정성 이득 없음) → 서두를 이유 없음.

## 711 회귀패치 재적용

```bash
git clone https://gogs.dclub.kr/kim/freelang-v11.git /tmp/v11
cd /tmp/v11 && git checkout $(cat <fx>/patches/711-regression-port.base)
git apply <fx>/patches/711-regression-port.patch
npm install esbuild --no-save && node scripts/build.js   # bootstrap.js 재생성
# 검증: node bootstrap.js run <gateway>/demo.fl → 22/22 PASS
```

## stdlib / 실행

`stdlib/` 는 이 리포에 미포함(운영 `freelang-v11/stdlib` 에서 링크·복사).
실행: `node bootstrap.js run <file.fl>` (stdlib 필요).
