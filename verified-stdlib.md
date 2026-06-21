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

**Status: VERIFIED (2026-06-20/21)**

**Evidence:**

| 경로 | 결과 | 증거 |
|------|------|------|
| v11 interpreter | ✅ VERIFIED | retry-result/retry-safe 실측. max-tries=5, fallback, -int-pow 모두 정상 |
| cgc C 컴파일 | ✅ VERIFIED | hash_map shim(fd7a57b) 추가 후 빌드·실행 성공. 출력: true/success!/fallback/1/2/8 |
| 실서비스 | ❌ 없음 | fx 앱 중 retry.fl 사용 사례 없음 |

**cgc 실행 증거 (root 노드, 2026-06-21):**
```
true        ← retry-result ok=true
success!    ← retry-result value
fallback    ← retry-safe (always-throw fn → fallback)
1           ← -int-pow 2 0
2           ← -int-pow 2 1
8           ← -int-pow 2 3
```

**수정 내역 (원본 retry.fl → 새 버전):**
1. `:keyword` 키 → `"string"` 키 (cgc 호환)
2. `try + {map}` → `try + (hash-map ...)` (v11 파싱 오류 수정)
3. `js-eval(Atomics.wait)` → `(sleep d)` (cgc 호환)
4. `pow()` 없음 → `-int-pow` (loop/recur 수동 계산)
5. `rand-int` 없음 → jitter 기능 제거
6. `-retry-core`: 절대 throw 하지 않음 (defn-throw-rerun 트랩 우회)
7. `(get m k default)` 3인자 → `-rget` 헬퍼 (cgc get은 2인자)

**runtime 변경사항 (fx-builtin-shim.c / runtime.h):**
- `throw` shim 추가 (커밋 2c98a8a): old cgc-bin `fl_fn_call(throw,...)` → `fl_throw()`
- `hash_map` shim 추가 (커밋 fd7a57b): old cgc-bin `fl_fn_call(hash_map,...)` → `fl_map_new/fl_map_set`

**[v11 주의] defn-throw-rerun 트랩:**
- v11에서 defn 내부 throw → 함수 전체 재실행
- retry-result/retry-safe: -retry-core가 throw 없음 → 정확히 max-tries 회 이하 호출
- retry(): v11에서 throw로 propagate 시 fn을 2×max-tries 회 호출 가능 (부작용 있는 fn은 retry-result 사용)

---

### 2. cache.fl

**Status: VERIFIED (2026-06-21)**

**Evidence:**

| 경로 | 결과 | 증거 |
|------|------|------|
| v11 interpreter | ✅ VERIFIED | 전 함수 실측. set/get/has/del/clear/stats/TTL/get-or-set 모두 정상 |
| cgc C 컴파일 | ✅ VERIFIED | 빌드 성공. 15항목 출력 v11과 동일 (nil 표현 차이만) |
| 실서비스 | ❌ 없음 | fx 앱 중 cache.fl 사용 사례 없음 |

**실행 증거 (v11+cgc, 2026-06-21):**
```
hello     ← cache-get "a" ✓
42        ← cache-get "b" (int) ✓
true      ← cache-has "a" ✓
false     ← cache-has "missing" ✓
2         ← cache-size ✓
false     ← cache-has after del ✓
1         ← size after del ✓
4         ← hits ✓
1         ← misses ✓
true      ← cache-has TTL before expire ✓
false     ← cache-has TTL after expire ✓
nil/null  ← cache-get expired = nil ✓
loaded-val ← cache-get-or-set (loader called) ✓
loaded-val ← cache-get (hit after set) ✓
0         ← cache-size after clear ✓
```

**구현 방식 (순수 FL, v11 내장 cache-* 미사용):**
- 핸들 = atom containing `{"entries" {key→{"val" v "exp" ms|nil}} "hits" N "misses" N "max" N}`
- TTL = `now_ms` 비교 (v11+cgc 공통)
- max-size 저장 but LRU 강제 없음 (순수 FL 한계)

**발견된 버그 + 수정 (cgc GC 함정):**
- 원인: `fl_atom_deref` = raw pointer (RC 증가 없음). `fl_atom_reset` = old값 RC-- → 0이면 재귀 free.
  reset! 후 old 포인터(`entry`) 접근 → freed memory → nil 반환
- 수정: hit 케이스에서 reset 후 `@ch` re-deref로 새 heap-safe 포인터에서 val 읽기
- 신규 트랩 등록: `trap-atom-deref-gc` (FX-TRAPS.airc #25)

**API 변경 (v11 원본 대비):**
- `cache-set [ch key val ttl-ms]` 4인자 → `cache-set-ttl [ch key val ttl-ms]` 로 분리 (FL 고정 arity)

---

### 3. queue.fl

**Status: VERIFIED (2026-06-20)**

**Evidence:**

| 경로 | 결과 | 증거 |
|------|------|------|
| v11 interpreter | ✅ VERIFIED | queue/stack 전 함수 실측. push/pop/peek/size/empty?/->list 모두 정상 |
| cgc C 컴파일 | ✅ VERIFIED | 빌드·실행 성공. queue 3항목 push→pop 순서 정확, stack LIFO 정확 |
| 실서비스 | ❌ 없음 | fx 앱 중 queue.fl 사용 사례 없음 |

**cgc 실행 증거 (root 노드, 2026-06-20):**
```
3       ← queue-size 3항목
a       ← queue-pop first item (FIFO)
3       ← stack-size
c       ← stack-pop last item (LIFO)
```

**수정 내역 (원본 queue.fl → 새 버전):**
1. `:keyword` 키 → `"string"` 키 (cgc 호환)
2. `(inc n)` → `(+ n 1)`, `(dec n)` → `(- n 1)` (inc/dec 빌트인 없음)
3. `butlast` → `(take (- n 1) items)` (butlast 빌트인 없음)

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
| retry.fl | ✅ VERIFIED | ✅ VERIFIED | ❌ 없음 | **VERIFIED** |
| queue.fl | ✅ VERIFIED | ✅ VERIFIED | ❌ 없음 | **VERIFIED** |
| cache.fl | ✅ VERIFIED | ✅ VERIFIED | ❌ 없음 | **VERIFIED** |
| debug-tools.fl | ⚠️ PARTIAL | ❌ 미검증 | ❌ 없음 | **PARTIAL** |
| http_get (client) | ✅ 동작 | ❌ BROKEN | ❌ 없음 | **PARTIAL** |
| parallel.fl | ❌ BROKEN | ❌ BROKEN | ❌ 없음 | **BROKEN** |
| server_start_tls | ❌ 미검증 | ❌ 미검증 | ❌ 없음 | **CLAIMED** |

**VERIFIED: 3/7 (43%)**  
**PARTIAL 이상: 5/7 (71%)**  
**완전 BROKEN: 1/7 (parallel만)**  
**CLAIMED: 1/7 (server_start_tls)**

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
