# Compact FX / FX3 문법 초안 v0

기록일: 2026-09-29

```text
STATUS=GRAMMAR_V0
IMPLEMENT=NO
ABBREVIATION=UNFIXED
EXPR_END=__
```

이 파일은 발견 당시의 문법 기록이다. 정본은 `/home/kim/kim/platform/freelang-fx3/GRAMMAR-V0.md`다. 사용자 피드백 이후 식의 끝은 `__`가 아니라 `;`다. 방향 기록은 [FX3-DIRECTION.md](FX3-DIRECTION.md)에 있다.

v0가 고정하는 것은 구조와 식의 끝이다. `J`, `U`, `L`, `R` 같은 축약어는 확정하지 않는다. 구조를 먼저 고정하고, 실제 FX 코퍼스에서 후보를 경쟁시킨다.

```text
newline ≠ syntax
whitespace ≠ syntax
__ = explicit expression boundary
```

## 1. 목표

Compact FX는 새로운 실행 의미를 만들지 않는다.

모든 코드는 반드시 기존 FX `.fl`로 내려간다.

```text
Compact FX
   ↓
Canonical FX .fl
   ↓
FX Native Runtime
```

언어 철학:

> **사람 가독성은 버리고, AI 가독성만 남긴다.**

목표:

**사람이 보기 좋은 코드가 아니라, AI가 가장 안정적으로 읽고 쓰는 고밀도 코드.**

줄 수, 들여쓰기, 예쁜 키워드, 긴 함수명은 부차적이다. 남겨야 하는 것은 다섯 가지다.

- 경계가 명확함
- 해석이 한 갈래
- 생성 오류가 적음
- 수정 위치가 국소적임
- `.fl`로 정확히 복원됨

핵심 골격:

```text
F name[$args]{              함수
  $x=expr __                binding + 표현식 종료
  $y=expr __
  ?cond{true}{false}        조건
}

@$x.a.b                     중첩 get
~$x                         null?
foo(...)                    일반 호출
__                          표현식 종료
{} [] ()                    구조 경계
$                           변수 경계 유지
```

## 2. 함수 선언

FX:

```lisp
(defn handle-rate-single [$req]
  ...)
```

Compact:

```text
F handle-rate-single[$req]{...}
```

규칙:

```text
F 함수명[인자...]{본문}
```

함수명은 압축하지 않는다.

## 3. 지역 변수

FX:

```lisp
(let [$c expr1
      $b expr2]
  body)
```

Compact:

```text
{$c=expr1 __ $b=expr2 __ body}
```

`let` 키워드는 제거한다. 블록 안의 바인딩이 `let`이다.

## 4. 조건문

FX:

```lisp
(if condition
  true-expr
  false-expr)
```

Compact:

```text
?condition{true-expr}{false-expr}
```

예:

```text
?~$body{json-err("환율 API 오류")}{json-ok($body)}
```

## 5. null 검사

FX:

```lisp
(null? $x)
```

Compact:

```text
~$x
```

의미:

```text
~$x
↓
(null? $x)
```

## 6. Map / 객체 접근

FX:

```lisp
(get (get $req "params") "currency")
```

Compact:

```text
@$req.params.currency
```

변환:

```text
@$req.params.currency
↓
(get (get $req "params") "currency")
```

v0는 `@`를 유지한다. `$req.params.currency`처럼 `@`를 생략하는 형태는 나중 후보다. 한 겹 `(get X K)` 축약은 별도 후보로 남긴다. self-host의 `get`은 중첩보다 한 겹이 많다.

## 7. 함수 호출

일반 함수 이름은 그대로 유지한다.

FX:

```lisp
(http-get-body RATE_URL)
```

Compact:

```text
http-get-body(RATE_URL)
```

여러 인자:

```text
foo($a,$b,$c)
```

## 8. 변수

v0는 `$`를 유지한다.

```text
$req
$body
$count
```

이유:

- 변수와 함수명 경계가 분명하다
- lexer가 단순하다
- AI 생성 오류가 줄어들 수 있다

실험에서 이점이 없을 때만 제거한다.

## 9. 공백, 줄바꿈, 식의 끝

문장 끝은 줄바꿈이 아니라 `__`다. 줄바꿈과 공백은 전부 보기용이다. parser는 구조 문자와 `__`만 본다.

```text
newline ≠ syntax
whitespace ≠ syntax
__ = EXPR_END
```

다음 둘은 동일하다.

```text
F add[$a,$b]{
  $c=$a+$b __
  $c
}
```

```text
F add[$a,$b]{$c=$a+$b __ $c}
```

`}` 바로 앞의 마지막 식은 `__`를 생략할 수 있다. 중간 식은 `__`로 끝난다. 그래서 식 하나를 고쳐도 해석이 `__` 밖으로 번지지 않는다.

```text
$c=... __ $b=... __ ?...{...}{...}
```

구조 경계는 다음 문자다.

```text
()
[]
{}
,
```

`,`는 인자나 맵 항목을 나눈다. 식을 끝내지 않는다. `;`는 식의 끝이 아니다.

### lexer

`__`를 만나면 무조건 끊지 않는다.

```text
1. string, raw literal, comment 안
   "hello__world"
   → 내용

2. 바로 앞과 바로 뒤가 모두 identifier-char
   currency__b
   → 이름의 일부

3. __ 로 시작해 __ 로 끝나는 이름 전체
   __pct__
   __apply__
   __default
   → 이름 한 토큰. 감싼 __ 는 EXPR_END가 아니다

4. 그 외의 __
   currency __
   currency__$b
   expr__}
   → EXPR_END
```

`$`는 identifier-char가 아니다. 그래서 `currency__$b`는 `currency`, `EXPR_END`, `$b`다.

dunder 이름 바로 뒤에 다음 식을 붙이면 경계가 없다. `__pct__$x`는 이름 `__pct__` 다음에 `$x`가 붙은 잘못된 식이다. 경계를 넣으려면 `__pct__ __ $x`처럼 `__`가 이름 밖에 있어야 한다.

## 10. do

`do` 키워드는 없다. 블록이 순차 실행이다.

FX:

```lisp
(do
  expr1
  expr2
  expr3)
```

Compact:

```text
{expr1 __ expr2 __ expr3}
```

## 11. Map

FX:

```lisp
{"ok" true "count" $count}
```

Compact:

```text
{ok:true,count:$count}
```

문자열 키가 기본이면 따옴표를 생략할 수 있다. 특수문자가 들어간 키는 따옴표를 유지한다.

```text
{"total-count":$n}
```

블록과 맵은 이렇게 가른다.

- 안에 `key:expr`가 있으면 맵이다. `{ok:true}`는 맵이다.
- 식이 `__`로 이어지면 블록이다.
- 빈 `{}`는 빈 맵이다.

## 12. Vector / List

후보:

```text
[$a,$b,$c]
```

Canonical FX로는 필요에 따라 `(list $a $b $c)` 또는 FX vector 표현으로 변환한다. v0 구조 문자는 이 표기를 후보로 열어 두고, 확정은 라운드트립 실험 뒤다.

## 13. 압축 원칙

압축 우선순위:

```text
defn
let
if
get 중첩
do
future + fn
server_json + json_stringify
반복되는 고빈도 builtin
```

압축하지 않는 것:

```text
handle-rate-single
parse-weather
send-notify
db-insert
사용자 정의 함수명
드물게 사용하는 builtin
```

## 14. 아직 확정하지 않는 축약

아래는 후보만 둔다. 코퍼스 경쟁 전에 문법으로 채택하지 않는다.

### JSON 응답

```text
J{ok:T,data:$rows}
↓
(server_json (json_stringify {"ok" true "data" $rows}))
```

`J`는 고빈도일 때만 남긴다. `U` 같은 builtin 한 글자도 같은 후보다.

### Boolean

```text
T = true
F = false
N = nil
```

함수 선언의 `F`와 `F = false`는 한 문법에 같이 둘 수 없다. v0는 `true`, `false`, `nil`을 그대로 쓴다.

### 논리 연산

```text
a&b
a|b
!a
$a>10
$a>=10
$a==$b
```

연산 우선순위는 최소화하고, 필요하면 괄호를 쓴다. 채택 전에 해석이 한 갈래인지 확인한다.

### Lambda

초기 안전 후보:

```text
L[$x]{+$x,1}
```

`[$x]{...}`는 vector 뒤에 블록이 오는 형태와 겹칠 수 있다. `L`을 쓰기 전에 loop의 `L`과 역할을 나눈다.

### try / catch

명시 후보:

```text
T{expr}C[$e]{fallback}
```

짧은 후보 `!{expr}{$e:fallback}`는 나중 실험이다. `T`를 true의 약자로도 쓰면 해석이 갈라진다.

### future

```text
^{send-notify($x)}
↓
(future (fn [] (send-notify $x)))
```

### loop / recur

self-host의 핵심이라 압축보다 명확성을 먼저 본다.

```text
L[$i=0]{?$i==10{$i}{R[$i+1]}}
```

`L`은 loop 후보, `R`은 recur 후보다. lambda의 `L`과 동시에 확정하지 않는다.

## 15. Hard Gate

문법 후보는 다음 중 하나라도 발생하면 폐기한다.

```text
AMBIGUOUS_PARSE=FAIL
FL_ROUNDTRIP=FAIL
SEMANTIC_MATCH=FAIL
```

- 두 가지로 해석되면 버린다.
- `.fl`로 안정적으로 복원되지 않으면 버린다. 같은 Compact를 다시 내리면 같은 `.fl`이 나오고, 그 실행이 원본과 같아야 한다. 원문과 공백까지 글자가 같은지는 게이트가 아니다.
- 원본 FX와 실행 결과가 다르면 버린다.
- 명세상 유효한 입력인데 파서가 실패해도 버린다.

반복 생성에서 같은 구문이 계속 깨지는 표기는 폐기 후보다. 실패 위치가 매번 흩어지면 `AI_GENERATION_ERROR_RATE` 점수로 남긴다.

## 16. 효율 경쟁

Hard Gate를 통과한 후보만 비교한다.

```text
CHAR_REDUCTION
LLM_TOKEN_REDUCTION
AI_GENERATION_ERROR_RATE
EDIT_LOCALITY
```

목표는 가장 짧은 언어가 아니다.

**AI가 가장 적게 틀리고, 충분히 짧고, 수정하기 쉬운 밀도**를 고른다.

경쟁 코퍼스는 앱, stdlib, self-host 세 부류다. 앱에서만 짧아지는 표기는 Compact FX로 채택하지 않는다.

## 예시

FX:

```lisp
(defn handle-rate-single [$req]
  (let [$cur  (str_upper (get (get $req "params") "currency"))
        $body (http-get-body RATE_URL)]
    (if (null? $body)
      (json-err "환율 API 오류")
      (json-ok $body))))
```

Compact FX. `U`는 아직 확정된 축약이 아니다. v0 구조만 쓰면 `str_upper`가 남는다.

```text
F handle-rate-single[$req]{
  $cur=str_upper(@$req.params.currency) __
  $body=http-get-body(RATE_URL) __
  ?~$body{json-err("환율 API 오류")}{json-ok($body)}
}
```

한 줄:

```text
F handle-rate-single[$req]{$cur=str_upper(@$req.params.currency) __ $body=http-get-body(RATE_URL) __ ?~$body{json-err("환율 API 오류")}{json-ok($body)}}
```

두 표기는 같은 `.fl`로 내려간다. `U(...)`로 `str_upper`를 줄이는 후보는 이 구조가 고정된 뒤 코퍼스에서 경쟁시킨다.
