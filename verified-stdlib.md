# FreeLang fx Verified Stdlib

> 작성: 2026-06-20  
> 목적: FreeLang 성숙화 지시서 Phase 1 산출물  
> 원칙: claimed ≠ observed. 실행 증거만 VERIFIED 승격.

---

## 판정 기준

| Status | 의미 |
|--------|------|
| **VERIFIED** | v11 + cgc + 실서비스 3가지 중 ≥2 조건 실측 증거 있음 |
| **PARTIAL** | 조건 1개 또는 일부 기능만 실측 |
| **BROKEN** | 실행 시 오류 (문서/주석과 달리 동작 안 함) |
| **CLAIMED** | 코드/문서에 존재하나 실측 증거 없음 |

---

## Phase 1 검증 결과

---

### 1. retry.fl

**Status: BROKEN**

**Evidence:**

| 경로 | 결과 | 증거 |
|------|------|------|
| v11 interpreter | ❌ BROKEN | `(try {:map...} (catch...))` → 파싱 오류 (map literal이 try 첫 body 위치 불가) |
| cgc C 컴파일 | ❌ BROKEN | `defn inner` 중첩 → `/* defn inside expr — skip */fl_nil()` 생성. js-eval 사용 (cgc 비호환) |
| 실서비스 | ❌ 없음 | fx 앱 중 retry.fl 사용 사례 없음 |

**구체적 오류:**
```
[E_PARSE_UNEXPECTED_TOKEN] [51:22] Expected RParen, got LBrace
— (try {:ok true ...} (catch err {...})) 형식에서 발생
```

cgc 생성 C:
```c
FLValue retry_simple(...) {
    return /* defn inside expr — skip */fl_nil();
}
```

**근본 원인:**
1. v11: `try` 본문에 map literal `{...}` 직접 사용 불가 (괄호 형태만 허용)
2. cgc: 중첩 `defn` (`defn fn` 안에 `defn inner`) → cgc가 건너뜀
3. `js-eval("Atomics.wait(...)")` 딜레이 → cgc C 네이티브에서 실행 불가

**수정 방향:**
```lisp
;; v11: {:map} → (hash-map "ok" true ...)
;; cgc: 중첩 defn → 최상위 defn으로 분리
;; delay: sleep_ms 빌트인 사용
```

---

### 2. cache.fl

**Status: PARTIAL (v11 내장 함수만)**

**Evidence:**

| 경로 | 결과 | 증거 |
|------|------|------|
| v11 내장 (`cache-create/get/has`) | ✅ PARTIAL | `(define ch (cache-create 10))` + get/has 동작 확인 |
| cgc C 컴파일 | ❌ BROKEN | `cache_create` 등 cgc 런타임에 없음 → 링크 실패 |
| 실서비스 | ❌ 없음 | fx 앱 중 cache.fl 사용 사례 없음 |

**동작 확인된 함수 (v11):**
```lisp
(cache-create 10)        ; → 핸들 반환 ✅
(cache-set ch "k" "v")  ; → 저장 ✅
(cache-get ch "k")      ; → "v" ✅
(cache-has ch "k")      ; → true ✅
(cache-has ch "no")     ; → false ✅
```

**cgc 실패 원인:**
- `cache_create`, `cache_set`, `cache_get` 등이 v11 인터프리터 전용 빌트인
- `runtime.h`, `aliases.c`에 선언 없음
- cgc 생성 C에서 `fl_fn_call(cache_create, ...)` → undefined symbol

**TTL 4인자 `cache-set`**: 미검증

---

### 3. queue.fl

**Status: PARTIAL (cgc 핵심 로직)**

**Evidence:**

| 경로 | 결과 | 증거 |
|------|------|------|
| v11 interpreter | ❌ BROKEN | `inc`/`dec` 함수 없음 → 런타임 오류 |
| cgc C 컴파일 | ✅ PARTIAL | `inc`→`(+ n 1)` 수동 대체 시 컴파일 성공, 실행 정상 |
| 실서비스 | ❌ 없음 | fx 앱 중 queue.fl 사용 사례 없음 |

**cgc 실행 결과:**
```
3     ; queue-size 정상
a     ; queue-pop 정상
a     ; queue-peek 정상
```

**v11 실패 원인:**
```
[queue-push] [inc] 'inc' 함수 없음
```
- queue.fl 내부에서 `(inc (get q :size))` 사용
- v11에서 `inc` 빌트인 없음 (`(+ x 1)` 사용해야 함)

**queue.fl 수정 필요:**
```lisp
;; 현재 (broken)
{:size (inc (get q :size))}

;; 수정
{"size" (+ (get q "size") 1)}
;; 참고: 키 스타일도 :keyword → "string" 로 맞춰야 cgc 호환
```

---

### 4. parallel.fl

**Status: BROKEN**

**Evidence:**

| 경로 | 결과 | 증거 |
|------|------|------|
| v11 interpreter | ❌ BROKEN | `run-parallel` 빌트인 없음 |
| cgc C 컴파일 | ❌ BROKEN | 동일 (빌트인 없음) |
| 실서비스 | ❌ 없음 | fx 앱 중 parallel.fl 사용 사례 없음 |

**v11 오류:**
```
[parallel-run] [run-parallel] 'run-parallel' 함수 없음
```

**파일 내용 (2줄):**
```lisp
(defn parallel-run [fns]
  (run-parallel fns 0))
```

`run-parallel`이 존재하지 않는 빌트인을 래핑. 전체 파일이 dead code.

---

### 5. debug-tools.fl

**Status: PARTIAL (v11 일부 기능)**

**Evidence:**

| 경로 | 결과 | 증거 |
|------|------|------|
| v11 interpreter (load) | ✅ PARTIAL | 로드 성공, inspect/assert 동작 확인 |
| cgc C 컴파일 | ❌ 미검증 | 테스트 미수행 |
| 실서비스 | ❌ 없음 | fx 앱 중 debug-tools.fl 사용 사례 없음 |

**동작 확인된 함수 (v11):**
```lisp
(inspect "x" 42)
; → 🔍 inspect: x / 타입: number / 값: 42

(assert (= 1 1) "메시지")
; → ✅ assertion passed: 메시지

(log-vars "a" 1 "b" "hello")
; → 📋 === 변수 로그 === (출력됨)
```

**미검증 기능:** `watch`, `update-watched`, `trace`, `breakpoint`

---

### 6. http_get / HTTP 클라이언트

**Status: PARTIAL**

**Evidence:**

| 경로 | 결과 | 증거 |
|------|------|------|
| v11 interpreter | ✅ VERIFIED | 로컬 HTTP 서버 200 응답 + body 수신 확인 |
| cgc C 컴파일 | ❌ BROKEN | `http_get` → `fl_http_get` (문자열 반환) 잘못 매핑됨 |
| 실서비스 | ❌ 없음 | outbound HTTP 사용 fx 앱 없음 |

**v11 동작 확인:**
```lisp
; FL_HTTP_ALLOW=127.0.0.1 필요 (SSRF 보호)
(define r (http_get "http://127.0.0.1:19998/"))
(get r "status") ; → 200 ✅
(get r "body")   ; → HTML body ✅
```

**v11에서 존재하는 함수:**
```lisp
(fn? http_get)   ; → true ✅
(fn? http_post)  ; → true ✅
(fn? http_put)   ; → true ✅
(fn? http_del)   ; → false ❌ (없음)
(fn? http_req)   ; → false ❌ (없음)
```

**cgc 실패 원인 (중요):**
- cgc 컴파일러 `cgc-main.fl:992`: `"http-get"` → `fl_http_get` 하드코딩
- `http_get` (underscore)도 동일하게 `fl_http_get`으로 변환됨
- `fl_http_get` (http.c)는 string 반환, SSRF 없음, 로컬 nil 반환 버그 있음
- `http_client.c`의 진짜 `http_get` (map 반환)이 cgc FL에서 노출 안 됨

**수정 방향:**
```
cgc-main.fl에서 http_get → http_get (C direct call) 또는 fxb_http_get 래핑 필요
```

---

### 7. HTTPS server (server_start_tls)

**Status: CLAIMED**

**Evidence:**

| 경로 | 결과 | 증거 |
|------|------|------|
| v11 interpreter | ❌ 미검증 | |
| cgc C 컴파일 | ❌ 미검증 | http.c:1351에 `server_start_tls` 구현 존재하나 실행 테스트 안 됨 |
| 실서비스 | ❌ 없음 | TLS 사용 fx 앱 없음 |

---

## 요약표

| stdlib | v11 | cgc | 실서비스 | **판정** |
|--------|-----|-----|---------|---------|
| retry.fl | ❌ BROKEN | ❌ BROKEN | ❌ 없음 | **BROKEN** |
| cache.fl | ⚠️ PARTIAL | ❌ BROKEN | ❌ 없음 | **PARTIAL** |
| queue.fl | ❌ BROKEN | ⚠️ PARTIAL | ❌ 없음 | **PARTIAL** |
| parallel.fl | ❌ BROKEN | ❌ BROKEN | ❌ 없음 | **BROKEN** |
| debug-tools.fl | ⚠️ PARTIAL | ❌ 미검증 | ❌ 없음 | **PARTIAL** |
| http_get (client) | ✅ 동작 | ❌ BROKEN | ❌ 없음 | **PARTIAL** |
| server_start_tls | ❌ 미검증 | ❌ 미검증 | ❌ 없음 | **CLAIMED** |

**Claimed → Verified 달성률: 0/7 (0%)**  
**PARTIAL 이상: 4/7 (57%)**  
**완전 BROKEN: 2/7 (retry, parallel)**

---

## 공통 결함 패턴

### P1. try 안에 map literal 불가 (v11)
```lisp
;; ❌ 파싱 오류
(try {:ok true} (catch err {:ok false}))

;; ✅ 우회
(try (hash-map "ok" true) (catch err (hash-map "ok" false "err" err)))
```

### P2. cgc에서 중첩 defn 건너뜀
```lisp
;; ❌ defn 안 defn → fl_nil() 반환
(defn outer [] (defn inner [] 42) (inner))

;; ✅ 최상위로 분리
(defn inner [] 42)
(defn outer [] (inner))
```

### P3. v11 전용 빌트인 목록 (cgc에서 없음)
- `inc`, `dec` → `(+ x 1)`, `(- x 1)` 사용
- `cache-create`, `cache-set`, `cache-get` 계열
- `run-parallel`

### P4. cgc http_get → fl_http_get 잘못 매핑
- 수정 전: `(http_get url)` → string 반환
- 기대: `{status body headers}` map 반환

---

---

## Phase 3 — 최소 필수 API 추가 (2026-06-20)

### fx-std.fl에 추가된 함수

**추가 기준 충족 확인:**
- `json_safe_get` / `get-in-2`: fx-kv 8회, fx-auth 6회, fx-sqlite-browser 3회 반복 패턴
- `upsert`: fx-kv 1회 `INSERT OR REPLACE` 직접 사용, 향후 확장 필요

| 함수 | cgc VERIFIED | v11 | 설명 |
|------|-------------|-----|------|
| `json_safe_get $m $k $default` | ✅ 실측 | ✅ | 맵에서 키 추출, nil이면 기본값 |
| `get-in-2 $m $k1 $k2` | ✅ 실측 | ✅ | 2단계 중첩 맵 접근 |
| `upsert $db $table $cols $vals` | ✅ 실측\* | - | SQLite INSERT OR REPLACE |

\* 이 노드(root) cgc: `sqlite_exec_p` (shim). 73노드 cgc: `fxb_sqlite_exec_p` (direct). 현재 fx-std.fl은 `fxb_` 사용 중 — 73노드에서 검증 필요.

**cgc 실행 증거 (root 노드, 2026-06-20):**
```
입력: json_safe_get 결과가 맵 기본값, get-in-2 중첩 접근
출력:
2        ← upsert 후 row 수
updated  ← upsert 덮어쓰기 확인
val2     ← json_safe_get 정상
42       ← get-in-2 정상
```

---

## 다음 단계 (Priority)

1. **retry.fl 수정** (P1, P2 적용): try 우회 + 중첩 defn 분리 + sleep_ms 딜레이
2. **queue.fl 수정** (inc → +1): v11/cgc 양쪽 동작 가능
3. **parallel.fl 삭제 또는 재구현**: run-parallel 빌트인 존재 여부 재확인
4. **cgc http_get 매핑 수정**: cgc-main.fl:992 `http_get` → `http_get` C direct call
5. **cache 빌트인 cgc 이식**: runtime/aliases.c에 cache_create 등 추가
6. **upsert 73노드 검증**: fxb_ 버전으로 빌드 확인
