# freelang-v11-fx

FL v11 **실험 런타임** + **11.7.11 통합 기준선/회귀패치 보존** 리포.
운영 `freelang-v11` 과 분리된 실험·기록 공간. (원칙: 기록이 증명이다)

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
