# FreeLang v11 통합 기준선 (Integration Baseline)

> 2026-06-07 확정. 이번 세션 조사의 산출물 = **코드가 아니라 결론**. 이 문서는
> 향후 통합을 "재조사"가 아니라 "계획된 이식 작업"으로 만들기 위한 고정 기록이다.
> 원칙: 기록이 증명이다.

## 0. 2026-06-08 갱신 — 원격 a08ae05 회귀 수정 완료 (이 문서 결론 정정·동결)

⚠️ **아래 1~8절은 b76ddf23 기준 기록(역사 보존). a08ae05 에서 결론이 바뀜.**

- 원격 master 진전: `b76ddf23` → **`a08ae05`** (다른 작업자). 우리가 발견한 회귀를
  **원격이 자체·더 완전하게 수정**: `KNOWN_ALIASES 40개` + `case "mod"/"inc"/"strlen"` 등 추가,
  `core 하위호환 복원`(mariadb/mongodb/ws/http-server), `server-start TLS`, `머지 전 앱 호환성 검수 스크립트`.
- **실측(clone a08ae05 + npm install esbuild + build)**: scenario **15/15 정상**(mod·includes?·strlen·
  first-class·file 전부), **gateway demo 22/22 PASS** = 회귀 해소 확인.
- ⇒ 2·6절의 "11.7.11=회귀 보유, 통합 명분=모듈화뿐" 결론은 **a08ae05 에서 무효**.
- ⇒ **`patches/711-regression-port.patch`(base b76ddf23) 폐기** — 원격이 더 완전히 수정함. (`patches/DEPRECATED.md` 참조)
- **통합 명분 = 모듈화 + 회귀수정 + 호환성복원 + 신기능 = 전방위 강화. 통합 준비 완료(명분 확실).**
- **선결 과제**: ① bootstrap 빌드 검증 — **완료**(a08ae05 clone→build OK, 산출물 정합). ② 앱 호환성
  검수 스크립트 구동 — **진행**(원격 동봉 스크립트 rule셋 확인 + fl-watchdog 등 주요 모듈 시뮬).
- ⚠️ 통합 잔존 주의(불변): bootstrap.js = **빌드 산출물**(git 0바이트) → 운영 단순 `git pull` 금지,
  `pull → npm install → build` 필수.

## 1. Source of Truth

- **원격 `gogs.dclub.kr/kim/freelang-v11` master (`b76ddf23`, v11.7.11)** = 최신 소스 구조.
  - `src/` **537개 모듈**(.ts)로 분리. `bin/` 네이티브 바이너리.
  - `bootstrap.js` 는 **git 산출물이 아니라 빌드 생성물**: `node scripts/build.js`
    = esbuild 가 `src/cli.ts` → `bootstrap.js` 번들. git 트리의 bootstrap.js·stage1.js
    는 **0바이트**, `dist/` 는 .gitignore. 옛 1.7MB 는 `bootstrap.js.v11-archived` 보존.
- **로컬 운영 `/root/freelang-v11`(=`/root/kim/freelang-v11` 심볼릭, base `26e7eb05`)**
  = 11.5.1 빌드 산출물이 채워진 **옛 실행 스냅샷**. 원격보다 2커밋 뒤(FF 가능, divergence 아님).

⚠️ **치명 함정**: 운영에서 단순 `git pull` 하면 bootstrap.js 가 0바이트가 되어 **FL 앱 즉시 다운**.
정식 경로는 `git pull → npm install(필요시) → npm run build → 실행`. 빌드가 따라와야 한다.

## 2. 안정성 결론 — "최신 = 더 안전"은 거짓

`11.7.11 > 11.5.1` 가정 **불성립**. 실측(빌드된 11.7.11 메인 인터프리터 `node bootstrap.js run`):

| 검증 | 11.7.11 결과 |
|------|------|
| `(mod 7 3)` | **FAIL** (Function not found) |
| `(swap! m update :a inc)` | **FAIL** (first-class) |
| `(strlen "abc")` | **FAIL** |
| `(conj [1 2] 3)` | OK (conj 는 evalBuiltin 존재) |
| gateway demo | **FAIL** — `[swap!] Function not found: inc` |

→ 11.7.11 도 **동일 회귀 보유**. 게다가 11.5.1 대비 **별칭 3개(read-file/write-file/file-exists?) 추가 소실**.
모듈화 과정의 회귀. `cons` 누락인데 `conj` 존재하는 비대칭도 확인.

## 3. 현재 기준선 = 운영 11.5.1 + fx 패치

`freelang-v11-fx/bootstrap.js` 의 패치가 사실상 **참조 구현(reference implementation)**:
- (A) builtin first-class 일반화 — `flIsKnownBuiltin`(evalBuiltin.toString() case 스캔)으로
  713개 builtin 을 swap!/map 함수인자로 허용. 기존 하드코딩 화이트리스트 대체. 미정의 심볼 보존.
- (B) evalBuiltin 누락 case 추가 (아래 4절 목록).
- 검증: gateway demo **22/22 PASS**, check-let-regressions 5/5, examples 정상.

## 4. 이식 목록 (11.7.11 evalBuiltin 누락 — 실측 확정)

flExecOpNative(101 ops) 를 빌드된 11.7.11 런타임으로 전수 호출 → "Function not found" 11개:

**핵심 8 (필수 이식)** — 게이트웨이·언어 동작 좌우:
```
mod  true?  false?  strlen  includes?  includes-item  cons  closure?
```
구현은 `flExecOpNative`(src/eval-builtins.ts:191~) 의 해당 case 를 `args[0]/args[1]` 기반으로 옮김.
(예: `case "mod": return args[0] % args[1];`)

**별칭 3 (선택)** — 정식 `file-read`/`file-write`/`file-exists` 는 존재하고 이 별칭들은 미사용:
```
read-file  write-file  file-exists?
```

**first-class 일반화 (필수)**:
`src/interpreter.ts:1550` 의 `if (this.context.functions.has(varName) ...)` 분기를
evalBuiltin case 인식 기반으로 확장 (fx 의 flIsKnownBuiltin 접근 이식).

## 5. 이식 위치 (2파일)

- `src/eval-builtins.ts` → `evalBuiltin`(899~) switch 에 4절 핵심 8(+선택 3) case 추가
- `src/interpreter.ts:1550` → first-class 일반화

## 6. 통합 명분 재평가

| 통합으로 얻는 것 | 통합으로 못 얻는 것 |
|---|---|
| 모듈화(537), 빌드체계 현대화, 유지보수성 | 버그 감소·안정성 향상 (회귀 동일) |

→ **서두를 이유 없음.** 통합은 "fx 패치를 src 이식 → build → gateway+앱 검증 → 운영 전환" 순.

## 7. 재현 명령 (이식/검증 시)

```bash
# clone + build (실험 격리)
git clone --depth 1 https://gogs.dclub.kr/kim/freelang-v11.git /tmp/fl-v11-711
cd /tmp/fl-v11-711 && npm install esbuild --no-save && node scripts/build.js

# 회귀 확인
node ./bootstrap.js run /tmp/verify711.fl     # mod/swap-update/strlen FAIL 재현
node /root/kim/freelang-v11-fx/bootstrap.js run /root/kim/freelang-gateway/demo.fl  # fx: 22/22 PASS
```

## 7.5. 이식 실행 기록 (2026-06-07 — 검증 완료, 미푸시)

`/tmp/fl-v11-711`(base `b76ddf23`)에 4·5절 이식을 **소스 레벨로 적용 + 재빌드 + 검증**했다.

- 변경 2파일: `src/eval-builtins.ts`(+52 — `flIsKnownBuiltin`:905 정의 + evalBuiltin switch 에 핵심8 + `inc`/`dec` + 별칭3 case), `src/interpreter.ts`(+8/−1 — :1549 first-class 일반화).
- 재빌드 `bootstrap.js` 실측: 언어 체크(mod/strlen/true?/false?/includes?/cons/closure?/`swap! update`) **전부 PASS**, **gateway demo 22/22 PASS** = fx 참조구현과 동일 동작. → **이식 = 회귀 해소가 실증됨.**
- ⚠️ `/tmp` 휘발 위험 → diff 를 **`patches/711-regression-port.patch`** (+ `.base` = base 커밋)로 영구 보존. 재현: `git clone … b76ddf23` 후 `git apply patches/711-regression-port.patch`.
- **결정**: 리서치 기록까지만. gogs 커밋·푸시·운영 전환은 **보류**(서두를 이유 없음 — 2·6절). 통합 재개 시 이 patch 적용이 출발점.

## 8. 결정 요약

1. 11.7.11 은 최신 구조이지만 **회귀 수정판이 아니다**.
2. **운영 11.5.1 + fx 패치**가 현재 가장 신뢰 가능한 기준선.
3. 향후 통합은 **회귀 수정 이식 후 검증**을 거쳐야 한다.

관련: `freelang-v11-fx/bootstrap.js`(참조 구현), gateway phase2-lc(이 회귀를 드러낸 검증).
