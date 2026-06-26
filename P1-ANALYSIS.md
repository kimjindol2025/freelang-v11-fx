# P1 함정 분석 (11개, 심각)

> **작업량**: 20-25시간  
> **우선도**: +++++  
> **상태**: 2026-06-26 분석 시작

---

## 🎯 P1 함정 우선순위 (작업량 기준)

### Tier 1: 1-2시간 (빠른 승리)

#### 1️⃣ trap-inc-dec-missing — inc/dec 함수 없음

**현상**: `(inc x)`, `(dec x)` 호출 시 "함수를 찾을 수 없음" 오류

```lisp
;; ❌ 오류
(inc 5)      ;; → "inc not found"
(dec 10)     ;; → "dec not found"

;; v11: 부트스트랩에 내장
;; cgc: 미구현
```

**근본 원인**: cgc 런타임이 `inc`, `dec` 을 정의하지 않음

**해결책**:
```lisp
;; stdlib 또는 runtime에 추가
(defn inc [$x]
  (+ $x 1))

(defn dec [$x]
  (- $x 1))

;; 또는 C 런타임 (runtime/math.c)에 직접 구현
FLValue fl_inc(FLValue x) { ... }
FLValue fl_dec(FLValue x) { ... }
```

**파일 수정**:
- `stdlib/core.fl` (있으면) 추가
- 또는 `runtime/math.c` 추가
- 또는 `cgc-main.fl` codegen에서 자동 생성

**예상 시간**: 1시간 (구현 + 테스트)

---

#### 2️⃣ trap-str-includes-int — 진리값 타입 오류

**현상**: `str-includes` 반환값이 C `int` (0/1) → 진리값이 아님

```lisp
;; ❌ 오류
(if (str-includes? "hello" "ll")
  (println "found")      ;; 1은 진리값이 아니라 정수
  (println "not found"))

;; C에서: str-includes는 1/0 반환 (bool이 아님)
;; 따라서 (if 1 ...) 에서 1은 truthy지만 명시적 boolean이 아님
```

**근본 원인**: C 함수가 `int` 반환 → FL `boolean`으로 자동 변환 안 됨

**해결책**:
```c
// runtime/string.c 수정
FLValue fl_str_includes(FLValue haystack, FLValue needle) {
  ...
  return fl_bool(result);  // 1/0 대신 bool 래핑
}
```

**파일 수정**:
- `runtime/string.c` (또는 유사)
- `cgc-emit-builtin-call`에서 `str-includes` 특수 처리

**예상 시간**: 1시간 (1-2개 함수 수정)

---

### Tier 2: 2-3시간 (중간 난이도)

#### 3️⃣ trap-server-json-string — false-200

**현상**: `(server-json {"ok" true})` 직접 전달 시 Content-Length: 0 (빈 응답)

```lisp
;; ❌ false-200
(server-json {"ok" true :data $rows})
;; → HTTP 200 but body empty

;; ✅ 올바른 패턴
(server-json (json-stringify {"ok" true :data $rows}))
;; → HTTP 200 with JSON body
```

**근본 원인**: `server-json`은 이미 직렬화된 **문자열만** 받음
- 맵을 직접 받으면 타입 오류 → 내용 무시 → 빈 응답

**해결책**:
```c
// runtime/server.c 또는 cgc codegen
// 방법 1: 타입 체크 → 에러 메시지
if (!is_string(arg)) {
  fl_error("server_json expects string, got " + type_name(arg));
}

// 방법 2: 자동 stringify
if (is_map(arg)) {
  arg = json_stringify(arg);
}
```

**파일 수정**:
- `runtime/server.c` (타입 체크 강화)
- 또는 `cgc-emit-*` (codegen에서 자동 stringify)

**예상 시간**: 2시간 (에러 메시지 추가 + 테스트)

---

#### 4️⃣ trap-count-type — cgc가 count를 모름

**현상**: `(count [1 2 3])` 호출 시 "count not found" 빌드 에러

```lisp
;; ❌ cgc 빌드 실패
(defn process [$items]
  (let [[$n (count $items)]]
    (println (str "items: " $n))))

;; v11: 부트스트랩 내장
;; cgc: `count` 함수 없음 (또는 `length`로 다르게 구현)
```

**근본 원인**: cgc 런타임에 `count` 함수 정의 누락

**해결책**:
```lisp
;; stdlib/core.fl에 추가 (또는 runtime에)
(defn count [$coll]
  (if (nil? $coll)
    0
    (length $coll)))
```

**파일 수정**:
- `runtime/builtin.c` 또는 `stdlib/core.fl`
- `cgc-main.fl` builtin list에 추가

**예상 시간**: 1시간 (간단한 래퍼 함수)

---

### Tier 3: 3-4시간 (중간)

#### 5️⃣ trap-kebab-symbol — kebab-case 함수명 미정의

**현상**: 함수명이 kebab-case일 때 파서/codegen에서 미정의 처리

```lisp
;; ❌ cgc 미정의
(defn process-items [$items]
  (map process-item $items))

(defn process-item [$item]
  item)

;; v11: kebab-case 지원
;; cgc: underscore만 지원 → process_items, process_item로 변환됨
;;     하지만 defn/call의 이름이 일치하지 않음
```

**근본 원인**: cgc가 kebab-case를 underscore로 자동 변환하지 않거나 일관성 부재

**해결책**:
```lisp
;; cgc-name 함수에서 kebab→underscore 자동 변환
(defn c-name [$symbol]
  ;; "process-items" → "process_items"
  (str-replace-all $symbol "-" "_"))
```

**파일 수정**:
- `cgc-main.fl` (cgc-name 또는 유사 함수)
- 파서/codegen 모두에서 일관적으로 적용

**예상 시간**: 2시간 (자동 변환 로직 + 회귀 테스트)

---

#### 6️⃣ trap-throw-invalid-initializer — throw가 잘못된 C 생성

**현상**: throw 코드가 유효하지 않은 C initializer 생성 → gcc 에러

```lisp
;; ❌ 빌드 실패
(try
  (if (< $x 0) (throw "negative") $x)
  (catch e (println e)))

;; cgc가 생성한 C 코드:
;; FLValue e = { .error = "negative" };  ;; ← 유효하지 않은 initializer
```

**근본 원인**: cgc의 throw codegen이 FLValue struct 초기화를 잘못함

**해결책**:
```c
// cgc-emit-throw 수정
// 현재 (잘못됨):
//   emit("{.error = \"msg\"}");
// 수정:
//   emit("fl_error(\"msg\")");  // 또는 proper struct init
```

**파일 수정**:
- `cgc-main.fl` (cgc-throw 또는 cgc-emit-throw)
- C 런타임 (exception handling 방식)

**예상 시간**: 2-3시간 (exception ABI 이해 필요)

---

#### 7️⃣ trap-string-literal-1024 — 큰 문자열 tmpfile 실패

**현상**: 1024B 이상 문자열 리터럴이 PRoot에서 tmpfile() 실패

```lisp
;; ❌ PRoot에서 빌드 실패
(define HTML "<!-- 5000자 HTML 블록 -->")
;; → "cannot write temp file: Permission denied"

;; 원인: cgc가 큰 문자열을 tmpfile로 C 파일에 쓰려는데
;;      PRoot tmpdir이 제약됨
```

**근본 원인**: cgc codegen이 큰 리터럴을 tmpfile 통해 쓰는데, PRoot 권한 부재

**해결책**:
```c
// cgc-main.fl 또는 fl-build.sh
// 방법 1: tmpdir override
export TMPDIR=/root/tmp  // writable dir

// 방법 2: 큰 문자열 자동 청크 분할
if (length > 1024) {
  parts = chunk_string(1024);
  return (str part1 part2 part3 ...);  // 자동 concat
}

// 방법 3: 외부 파일로 분리
#include "data.h"  // 큰 상수 분리
```

**파일 수정**:
- `fl-build.sh` (TMPDIR 설정)
- `cgc-main.fl` (string literal chunking)

**예상 시간**: 2-3시간 (TMPDIR 우회 테스트 + 청킹 구현)

---

### Tier 4: 4-5시간 (어려움)

#### 8️⃣ trap-server-html-quote — HTML 파싱 오류

**현상**: HTML 안의 `"` 가 FL 문자열 파서 혼동

```lisp
;; ❌ 파싱 오류
(server-html "...<input onclick='name=\"John\"'>...")
;; FL 파서가 onclick 안의 " 를 FL 문자열 끝으로 인식

;; 정상: 이스케이프 또는 다른 인코딩
(server-html "...<input onclick='name=&#34;John&#34;'>...")
```

**근본 원인**: FL 문자열 파서가 내부 따옴표를 구분 못함

**해결책**:
```lisp
;; cgc-parse-string 수정 (line 115-135 근처)
;; 현재: " 만나면 종료
;; 수정: \" escape 문자 감지

(defn read-string-iter [$_cur $_res_acc $line $col]
  (loop [$cur $_cur $res_acc $_res_acc]
    (if (at-end? $cur) (emit ...)
      (let [[$c (peek $cur)]]
        (cond
          [(= $c "\"")       (emit ...)]  ;; 종료
          [(= $c "\\")       ;; escape 처리
           (let [[$st2 (advance $cur)] [$c2 (peek $st2)]]
             (recur (advance $st2) (str $res_acc (translate-esc $c2))))]
          [true              ;; 일반 문자
           (recur (advance $cur) (str $res_acc $c))])))))
```

**파일 수정**:
- `cgc-main.fl` (read-string-iter, line ~115)

**예상 시간**: 2-3시간 (escape 처리 + edge case 테스트)

---

### Tier 5: 5+시간 (복잡)

#### 9️⃣ trap-nested-defn-skip — 중첩 defn 컴파일 안 됨

**현상**: defn 내부에 정의한 중첩 함수가 컴파일되지 않음 (silent skip)

```lisp
;; ❌ 중첩 defn 무시
(defn outer [$x]
  (defn inner [$y]  ;; ← 이것이 무시됨
    (+ $x $y))
  (inner 10))

;; v11: 중첩 defn 지원
;; cgc: "defn inside expr — skip" → fl_nil() 생성
```

**근본 원인**: cgc가 defn을 top-level 구조로만 인식, expr 내부에서 감지 시 무시

**해결책**:
```lisp
;; cgc-main.fl의 defn 처리 개선
;; 방법 1: defn inside expr 에러
;;   → CGC_ERROR("nested defn not supported")
;;
;; 방법 2: 중첩 defn을 최상위로 자동 끌어올리기
;;   → Hoisting (JavaScript 스타일)

;; 방법 3: 람다로 변환
;;   (defn inner [$y] ...) → (define inner (fn [$y] ...))
```

**파일 수정**:
- `cgc-main.fl` (cgc codegen, expr 처리)
- 또는 ast 전처리 단계에서 호이스팅

**예상 시간**: 3-4시간 (호이스팅 로직 복잡)

---

#### 🔟 trap-cache-cgc-missing — cache.fl v11 전용

**현상**: cache.fl 로드는 성공하지만 호출 시 런타임 오류

```lisp
;; ❌ cache.fl은 v11 빌트인만 사용
(load "stdlib/cache.fl")
(cache-get "key")  ;; ← link error 또는 undefined
```

**근본 원인**: cache.fl 내부가 `cache-set`, `cache-get` v11 빌트인을 래핑하는데,
cgc 런타임에 이들 함수가 없음

**해결책**:
```lisp
;; 방법 1: cgc용 cache.fl 구현
;;   (atom 기반 in-memory cache)
;;
;; 방법 2: stdlib에서 cache.fl 제거 (cgc용 다른 구현 별도)
;;
;; 방법 3: 조건부 로드
;;   (if v11? (load "cache-v11.fl") (load "cache-cgc.fl"))
```

**파일 수정**:
- `stdlib/cache.fl` (cgc용 재구현)
- 또는 빌드 시 버전 분기

**예상 시간**: 2-3시간 (cache 로직 재구현)

---

#### 1️⃣1️⃣ trap-parallel-run-missing — parallel.fl 미구현

**현상**: parallel.fl 로드는 성공하지만 `run-parallel` 호출 시 오류

```lisp
;; ❌ parallel.fl 미구현
(load "stdlib/parallel.fl")
(run-parallel (list fn1 fn2))  ;; ← undefined
```

**근본 원인**: parallel.fl이 `run-parallel` 빌트인을 가정하는데, 미구현

**해결책**:
```lisp
;; 방법 1: run-parallel 구현
;;   → Go/Rust처럼 실제 병렬 실행?
;;   → 또는 동기 순차 실행 (모의)
;;
;; 방법 2: parallel.fl 미지원 플래그
;;   (fl_error "parallel not supported in cgc")
```

**파일 수정**:
- `runtime/*.c` (run-parallel 구현) 또는
- `stdlib/parallel.fl` (동기 시뮬레이션)

**예상 시간**: 2-3시간 (모의 구현 + 테스트)

---

## 📊 P1 구현 순서 (우선도)

```
1단계 (2-3h): inc/dec + count + str-includes-int
              → 스스로 검증 가능 (단순)

2단계 (4-6h): server-json + kebab-symbol + string-literal-1024
              → 중간 난이도

3단계 (6-8h): throw + nested-defn + cache/parallel
              → 복잡, 회귀 테스트 많음
```

**전체 예상**: 20-25시간 (순차 또는 병렬)

---

## 📁 수정 대상 파일

| 파일 | 함정 | 난이도 |
|------|------|--------|
| `cgc-main.fl` | kebab, nested-defn, throw, string-literal | ⭐⭐-⭐⭐⭐⭐ |
| `runtime/string.c` | str-includes-int | ⭐ |
| `runtime/server.c` | server-json | ⭐⭐ |
| `runtime/builtin.c` | inc/dec, count | ⭐ |
| `stdlib/cache.fl` | cache-cgc-missing | ⭐⭐ |
| `stdlib/parallel.fl` | parallel-run-missing | ⭐⭐ |
| `fl-build.sh` | string-literal-1024 (TMPDIR) | ⭐ |

---

**상태**: 분석 중...
