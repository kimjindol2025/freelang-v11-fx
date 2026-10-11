# FX / FreeLang 공통 오류 한 줄 스키마

상태: `Phase 0 DESIGN` (문서만)  
기록일: 2026-10-12  
범위: FreeLang 계열 공통 스키마. **이번 단계 FX 문서만.** AFJ·Script 구현은 따로.  
상위: [`SIX-ITEMS-BY-DIALECT.md`](SIX-ITEMS-BY-DIALECT.md) 5항  
구현: Phase 1 — FX `runtime/error.c` (이번 Phase 0에서 **수정하지 않음**)

## 1. 한 줄 형식

```text
파일:줄 | 오류종류 | 받은 값 | 이유 | 고치는 법
```

| 칸 | 의미 | 예 |
|----|------|-----|
| 파일:줄 | 소스 위치. 파일이 없으면 `-:줄` 또는 `-:—` | `server.fl:42` |
| 오류종류 | 짧은 분류 토큰 | `uncaught` · `compile` · `lint` |
| 받은 값 | 문제의 값·심볼·메시지 요약 (한 토큰/짧은 구) | `demo-uncaught` · `hash_map` · `(def …)` |
| 이유 | 왜 깨졌는지 | `FLValue를 함수처럼 호출` |
| 고치는 법 | 다음에 할 일 한 구 | `(hash-map …)` → `(hash_map …)` 또는 매핑된 빌트인 사용 |

구분자는 ` | ` (공백-파이프-공백). 한 줄이 원칙이다. 스택·노트는 **그 다음 줄**에 부가로 둘 수 있다.

AFJ·Script도 같은 다섯 칸을 쓰도록 맞춘다. 계열마다 다른 포맷으로 분기하지 않는다 (결정 문서 5항).

## 2. 지금 실제 출력 ↔ 새 형식 (나란히)

실측 노드: aarch64, `freelang-v11-fx`, 설치본 `cgc-bin` emit SHA `63411319…` (2026-10-12).

### 2.1 런타임 Uncaught

**재현**

```bash
# /tmp/fx-err-uncaught.fl
(defn boom []
  (throw "demo-uncaught"))
(boom)
bash fl-build.sh /tmp/fx-err-uncaught.fl /tmp/fx-err-uncaught.elf
/tmp/fx-err-uncaught.elf ; echo RC:$?
```

**지금 실제 stderr** (`runtime/error.c` `print_uncaught`, exit 1)

```text
Uncaught error (line 2): demo-uncaught
Stack trace:
  at boom (line 2)
```

**같은 내용을 새 한 줄로**

```text
-:2 | uncaught | demo-uncaught | throw가 catch 없이 전파됨 | try/catch로 감싸거나 Result/*-safe로 실패를 반환
```

부가(허용):

```text
  at boom (line 2)
```

비고: 현재 포맷에는 **파일 경로가 없다** (`(line N)`만). Phase 1에서 `파일:줄`을 채우려면 throw 지점에 파일 정보가 더 필요함 — 확인 안 됨(지금 `__fl_throw_line`만 존재).

### 2.2 cgc → gcc 컴파일 오류

**재현**

```bash
# /tmp/fx-err-compile2.fl
(defn handle []
  (hash-map "a" 1))
(println (handle))
bash fl-build.sh /tmp/fx-err-compile2.fl /tmp/fx-err-compile2.elf
```

**지금 실제 출력** (fl-build가 gcc stderr를 전달)

```text
❌ gcc 컴파일 실패:
<fl>: In function ‘handle’:
<fl>:3:28: error: called object ‘hash_map’ is not a function or function pointer
In file included from /tmp/fl_build_….c:1:
…/runtime/runtime.h:642:16: note: declared here
  642 | extern FLValue hash_map;
```

**같은 내용을 새 한 줄로**

```text
-:3 | compile | hash_map | extern FLValue 심볼을 C 함수처럼 호출함 | (hash-map …) 대신 매핑된 빌트인/생성자를 쓰거나 cgc 매핑을 추가
```

비고: gcc가 찍는 위치는 생성 C의 `<fl>:줄`이다. 원본 `.fl` 줄과 대체로 가깝지만 **항상 동일하다고 단정하지 말 것**. 앱 예: `apps/fx-short/server.fl`도 `invalid initializer` / `fl_or` 타입 불일치로 동일 경로에서 실패 (2026-10-11~12 스모크).

### 2.3 lint 경고

**재현**

```bash
# /tmp/fx-err-lint.fl
(def x 1)
(defn foo [n]
  (+ n 1))
(server-json {"a" 1})
bash ./fl-lint.sh /tmp/fx-err-lint.fl
```

**지금 실제 출력** (`fl-lint.sh`)

```text
📋 fx-err-lint.fl (4 줄)
  줄 1 [W1] (def ...) → (define ...) 를 사용하세요
  줄 2 [W3] defn 파라미터에 $ 없음: (defn foo [n]
  줄 4 [dialect-fx-api] FX API는 underscore: (server-json
────────────────────────
✅ 0 개 통과  |  ❌ 1 개 실패
```

**같은 내용을 새 한 줄로** (경고마다 한 줄)

```text
fx-err-lint.fl:1 | lint | (def …) | FX/AFJ 모두 define 사용 | (define x …)로 바꾸기
fx-err-lint.fl:2 | lint | (defn foo [n] | 파라미터에 $ 접두 없음 | (defn foo [$n] …)
fx-err-lint.fl:4 | lint | (server-json | FX API는 underscore 축 | (server_json …)
```

비고: lint는 지금도 **수정하지 않고 경고만** 한다. 통과/실패 집계는 경고가 있으면 실패로 센다(현재 스크립트 동작).

## 3. 오류종류 토큰 (초안)

| 토큰 | 출처 | Phase 0 |
|------|------|---------|
| `uncaught` | `runtime/error.c` print_uncaught | 문서만 |
| `compile` | fl-build → gcc (또는 향후 cgc 자체 진단) | 문서만 |
| `lint` | `fl-lint.sh` | 문서만 |
| `trap` | FX-TRAPS 재현 (선택) | 예약 |
| `builtin` | 빌트인 nil/실패 메시지 (선택) | 예약 · 확인 안 됨 |

## 4. Phase 1에 넘기는 것 (이번 금지)

- `runtime/error.c`의 `print_uncaught` / `fl_make_error` 포맷 변경
- cgc가 파일 경로를 throw 지점에 심는 일
- fl-lint 출력 포맷을 스키마 한 줄로 바꾸는 일 (원하면 별 지시)
- AFJ stderr·Script 진단 CLI 구현 — **AFJ 쪽은 따로** ([`SIX-ITEMS-BY-DIALECT.md`](SIX-ITEMS-BY-DIALECT.md) §2)

## 5. 검증 (Phase 0 ①)

| 기준 | 결과 |
|------|------|
| 문서가 실제 출력에 근거 | PASS — §2.1~2.3 명령·stderr 실측 |
| lint 변경 | 해당 없음 (①은 문서만) |
| 고정점 SHA | 아래 게이트에서 확인 (코드 미변경 → `63411319…` 유지 기대) |
| traps 회귀 | 아래 게이트 |
| SIX-ITEMS Phase 0 표시 | 본 문서 링크 + 5항 Phase 0 완료 표시 |
