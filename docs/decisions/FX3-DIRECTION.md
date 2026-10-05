# FX3 방향

기록일: 2026-09-29

```text
STATUS=DIRECTION
FX3_IMPLEMENT=NO
COMPACT_EXPERIMENT=NOT_STARTED
```

이 문서는 FX3를 열기 전에 남긴 방향 기록이다. 언어 프로젝트 정본은 `/home/kim/kim/platform/freelang-fx3`다. 이 저장소의 FX 문법과 런타임은 바꾸지 않는다.

## 한 줄

**FX3는 기능을 더 넣는 버전이 아니라, FX와 FX2에서 실제로 살아남은 기능만 가지고 AI 기준으로 다시 깎는 세대.**

FX와 FX2가 표면에서 길어지는 것은 실패가 아니다. FX3를 만들기 위한 실전 데이터 축적이다.

## 출발

FX는 AI가 쓰기 좋은 언어를 지향해서 시작했다. 사람 가독성이나 전통적인 문법보다, AI가 빠르게 생성하고 해석하고 실행하는 쪽이 핵심이었다.

밑바닥 runtime, native 기능, builtin, 라이브러리를 계속 추가하면서 `server_json`, `json_stringify`, `fxb_sqlite_query_p` 같은 길고 변형된 함수명과 호출 방식이 누적됐다. 이것은 일부러 사람 친화적으로 만든 문법이 아니다. 기능 추가 과정에서 AI가 익숙한 방식으로 조금씩 덧붙인 흔적에 가깝다.

그래서 지금 FX를 억지로 다시 깎지 않는다.

**FX / FX2는 실전에서 부딪히며 확장된 세대**로 남긴다. 그 기록 자체를 보존한다.

- FX: `/home/kim/kim/platform/freelang-v11-fx` — 이 저장소
- FX2: `freelang-v11-fx2` — 별도 세대. 이 문서가 FX2 소스를 바꾸지 않는다

## 계보

```text
FX
↓
실전 사용 / native / self-host / 기능 확장

FX2
↓
runtime / library / interface 확장

Compact FX 실험
↓
AI에게 가장 좋은 표현 밀도 탐색

FX3
↓
AI-native 원칙으로 다시 정리된 다음 세대
```

## FX3 방향

다음 세대에서 원래 철학을 다시 잡는다.

- AI가 주 사용자
- 의미는 명확해야 함
- 공백/줄바꿈 의존 최소화
- 표현은 짧게
- 긴 함수명과 중복 패턴 최소화
- 기능은 많아도 언어 표면은 작게 유지
- 기존 FX 의미를 가능한 한 그대로 활용
- 필요하면 Compact 계층으로 `.fl`에 내려감
- 실패한 문법 후보는 빨리 버림

실험 철학:

> **사람 가독성은 버리고, AI 가독성만 남긴다.**

사람이 보기 좋은 코드가 아니라, AI가 가장 안정적으로 읽고 쓰는 고밀도 코드가 목표다. 줄 수, 들여쓰기, 예쁜 키워드, 긴 함수명은 부차적이다. 경계, 한 갈래 해석, 적은 생성 오류, 국소 수정, `.fl` 복원이 남는다.

> **의미가 깨지면 버린다. 효율은 경쟁시킨다. 문법 자체에는 집착하지 않는다.**

한 줄로 적으면:

> **명확하게 만들고, 빠르게 검증하고, 쓸모없으면 바로 버린다.**

## Compact FX

문법 초안은 [COMPACT-FX-V0.md](COMPACT-FX-V0.md)에 있다. 정본의 식의 끝은 `/home/kim/kim/platform/freelang-fx3`에서 `;`로 바뀌었다. 줄바꿈과 공백은 문법이 아니다. `J`, `U`, `L`, `R`은 아직 확정하지 않는다.

Compact FX는 새 실행 언어가 아니다. 기존 FX `.fl`로 반드시 내려가는 압축 표기층이다. FX3 설계 전의 검증장이다.

```text
Compact FX
   ↓ transpile
FX .fl          ← 기준 / 역사 / 복구 표현
   ↓
기존 FX 실행 경로
```

같은 프로그램을 네 가지로 만든다.

1. 원본 `.fl`
2. 이름 보존 Compact (`Compact-L`)
3. 중간 밀도 Compact (`Compact-M`)
4. 극단 압축 Compact (`Compact-X`)

밀도 후보는 세 단계로 잡는다.

| 이름 | 하는 일 |
|------|---------|
| C1 Structural Compact | 긴 구조만 줄인다. 이름은 대부분 유지한다 |
| C2 Semantic Compact | 자주 쓰는 core/builtin만 짧은 별칭을 쓴다 |
| C3 Extreme Compact | 한 글자 opcode와 구조 문자 중심 |

앱 코드만 잘 줄어드는 문법은 웹 축약일 수 있다. 언어 전체의 Compact FX가 되려면 아래 셋을 같이 통과해야 한다.

1. 일반 앱 코드 — server / DB / HTTP
2. stdlib — map / reduce / retry / cache
3. compiler / self-host — `self/cgc-main.fl` 포함

### 1단계 — Hard Gate

통과하지 못하면 그 표기는 즉시 폐기한다.

```text
AMBIGUOUS_PARSE=FAIL      → 즉시 폐기
FL_ROUNDTRIP=FAIL         → 즉시 폐기
SEMANTIC_MATCH=FAIL       → 즉시 폐기
```

- 해석이 두 갈래면 폐기한다.
- `.fl`로 안정 복원이 안 되면 폐기한다. 안정 복원이란, 같은 Compact를 다시 내리면 같은 `.fl`이 나오고 그 실행이 원본과 같은 것이다. 원문 `.fl`과 공백·바인딩 표기까지 글자가 같은지는 이 게이트가 아니다.
- 실행 결과가 원본과 다르면 폐기한다.
- 명세상 유효한 입력인데 파서 자체가 실패하는 것도 Hard Gate다.

반복 생성 시험에서 같은 구문이 구조적으로 계속 틀리는 표기는 즉시 폐기가 아니라 폐기 후보다. 같은 과제, 같은 모델, 같은 예시 수로 여러 번 돌렸을 때 같은 구문이 계속 깨지면 버린다. 실패 위치가 매번 흩어지면 `AI_GENERATION_ERROR_RATE` 점수로 남긴다.

성공 조건:

```text
Compact → .fl → 실행 결과
        ==
원본 .fl → 실행 결과
```

이 등식이 유지되는 동안 Compact 문법을 어떻게 줄여도 FX 의미론은 그대로다.

### 2단계 — 효율 경쟁

Gate를 통과한 후보끼리만 비교한다. 절대값 하나로 자르지 않고, 후보 간 trade-off를 본다.

- `CHAR_REDUCTION`
- `LLM_TOKEN_REDUCTION`
- `AI_GENERATION_ERROR_RATE`
- `EDIT_LOCALITY`

`EDIT_LOCALITY`는 작은 수정 하나에 Compact 코드를 얼마나 건드리는지다. 원본 `.fl` 기준의 의미 변화 하나를 주고, 바뀐 토큰 비율과 그 수정으로 생긴 무관 동작 변화를 같이 본다.

`PARSE_SUCCESS_RATE`는 별도 점수가 아니다. 유효 입력의 파서 실패는 Hard Gate다. AI가 잘못된 Compact를 생성하는 비율은 `AI_GENERATION_ERROR_RATE`다.

예시. `Compact-X`가 문자를 크게 줄여도 생성 오류가 높고, `Compact-L`이 오류는 가장 낮지만 압축이 약하고, `Compact-M`이 압축·오류·수정에서 균형이면 M이 AI용 표기 후보가 된다.

## 먼저 볼 표면과 남겨 둘 표면

문자 질량으로 먼저 줄어들 후보:

- `(get …)` 와 `(get (get …))` 중첩. 점 접근은 앱의 중첩 패턴이고, self-host의 `get`은 한 겹이 대부분이라 `(get X K)` 축약도 같이 있어야 한다
- `(defn NAME [ARGS] BODY)`
- `(let […] BODY)`
- `(fn […] BODY)`
- `(if A B C)`
- `(server_json (json_stringify X))` — 앱에 반복
- 긴 native 이름: `fxb_sqlite_*`, `server_*`, `json_stringify`

self-host 쪽에 몰린 구조: `str`, `loop`, `recur`, `cond`, `and`. 웹 별칭만 전역으로 두면 self-host에서는 죽은 기호가 된다.

점수 경쟁으로 남길 것:

- 변수 `$`. 글자는 작지만 토큰 경계를 고정할 수 있다
- 공백·줄바꿈 제거. 문자 비율은 크지만 LLM 토큰 비율과 같지 않다. 무공백을 쓰려면 구조 문자가 식별자 경계를 완전히 보장해야 한다

당장 문법 경계로 건드리지 않을 것:

- 괄호 종류
- 문자열 경계
- map / list 표현

끝까지 유지할 것:

- `.fl`로 안정 복원
- 의미 동일
- 원본 FX 실행 경로

## 측정 순서

코드 수정 없이, 실제 FX 코퍼스에서 어떤 문법이 문자와 LLM 토큰을 가장 많이 먹는지 빈도부터 잰다. 감으로 줄이지 않는다. 가장 비싼 문법부터 Compact 후보로 올린다.

2026-09-29 문자 질량 1차 측정(주석·문자열 내용 제외, `.fl` 47개):

| 구간 | 파일 | 코드 문자 | 큰 항목 |
|------|------|-----------|---------|
| 앱 | 27 | 98,001 | `server_*`, `json_stringify`, `server_json`, 중첩 `get` |
| stdlib | 7 | 20,339 | `defn`, `get`, `null?`, `let`, `str`, `if` |
| self | 1 | 87,267 | `get` 850회, `str`, `defn`, `loop`/`recur`/`cond`. `server_json`·`future`는 0 |
| 테스트 | 12 | 11,465 | 표기 설계의 본체로 쓰지 않는다 |

전체 코드 문자 217,072 가운데 공백은 33.6%다. `$` 변수는 8,288개이고, `$` 글자만은 코드의 3.8%다. 이 숫자는 문자 기준이다. LLM 토큰 순위는 아직 없다.

## 하지 않는 일

- 현재 FX `.fl` 문법과 실행 경로를 바꾸지 않는다
- FX2 소스를 이 방향으로 고치지 않는다
- FX3 구현을 시작하지 않는다
- Compact 표기가 Gate를 통과하기 전에 FX3 문법으로 채택하지 않는다
