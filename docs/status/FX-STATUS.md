# FreeLang fx — 현황 문서 (단일 진실)

> **생성**: 2026-06-16 · **최종 갱신**: 2026-09-27
> **고정점**: `aa8bed315d2bae91630fcff72d7edcafb9afafd84dc357133bb271c3d1bda4fd` (gen-a==gen-b==gen-c)
> **검증 커밋**: `10848c4` · 상태 **VERIFIED**
> **읽어야 하는 대상**: fx로 앱을 작성하거나, 컴파일러를 수정하거나, 다음 세션을 이어받는 Claude

---

## 1. 빠른 진입 체크리스트

```bash
# 1. 현재 고정점 확인 (컴파일러 수정 전 필수)
./verify-fixpoint.sh

# 2. 앱 빌드
bash ./fl-build.sh server.fl ./my-app

# 3. PM2 등록
pm2 start ./my-app --name my-app

# 4. 컴파일러 수정 후 고정점 복구
# → cgc-main.fl 변경 → verify-fixpoint.sh → PASS면 fx-upgrade-cgc.sh
# → top-level 코드젠 변경이면 반드시 2단계 부트스트랩 (수동 스크립트)
```

---

## 2. ✅ 구현 완료 목록

### 2-A. 언어 핵심

| 기능 | 상태 | 비고 |
|------|------|------|
| 기본 타입 (int/float/bool/nil/string/vector/map) | ✅ | |
| 산술 (+/-/*//) | ✅ | 0 나눗셈 → fl_throw (스택 포함) |
| 비교 (=/!=/</>/<=/>= ) | ✅ | |
| 논리 (and/or/not) | ✅ | and/or 단락평가 |
| 문자열 보간 `"#{$x}"` | ✅ | |
| if / cond / when / unless | ✅ | |
| let 바인딩 | ✅ | `[$x val]` 쌍 방식 |
| $_ 재선언 허용 | ✅ | `__fl_ign_N` 자동 rename |
| defn | ✅ | push/pop 스택 트레이스 자동 포함 |
| 직접재귀 TCO (recur / goto) | ✅ | 스택오버플로 없음 |
| 익명함수 fn | ✅ | |
| 클로저 (중첩 fn) | ✅ | outer-params-atom 캡처 |
| loop / recur | ✅ | |
| do / begin | ✅ | |
| try / catch / finally | ✅ | |
| throw | ✅ | 줄 번호 + 스택 트레이스 |
| -> / ->> threading macro | ✅ | |
| case / when / unless / dotimes / doto | ✅ | |
| for (comprehension) | ✅ | `(for [$x coll] body)` |
| doseq (side-effect loop) | ✅ | |
| while | ✅ | |
| define (전역 변수) | ✅ | |
| atom / reset! / swap! / deref | ✅ | thread-safe |
| future / deref-timeout | ✅ | pthread + mutex/cond |
| load (파일 인클루드) | ✅ | expand-loads 전처리 |
| #line 지시자 (에러 위치) | ✅ | defn + top-level 모두 |

### 2-B. 컬렉션

| 함수 | 상태 | 비고 |
|------|------|------|
| vector 리터럴 `[1 2 3]` | ✅ | |
| map 리터럴 `{:k v}` | ✅ | |
| get / get-in | ✅ | |
| assoc / dissoc | ✅ | |
| **assoc-in** | ✅ 2026-06-16 | 중첩 경로 자동 맵 생성 |
| **update-in** | ✅ 2026-06-16 | `(update-in m [:a :b] fn)` |
| push / append | ✅ | push=원소추가, append=벡터합치기 |
| map / filter / reduce | ✅ | |
| map-indexed / map-vals / map-entries | ✅ | |
| filter / count-if / keep | ✅ | |
| any? / every? / none? | ✅ | |
| first / last / rest / nth | ✅ | |
| slice / take / drop | ✅ | |
| range / repeat | ✅ | |
| reverse / sort / sort-by | ✅ | |
| flatten / flatten-1 | ✅ | |
| distinct / frequencies | ✅ | |
| group-by / max-by / min-by | ✅ | |
| zip / zip-with | ✅ | |
| concat / into / conj | ✅ | |
| entries / keys / vals | ✅ | |
| contains? | ✅ | 맵/벡터/문자열 전부 |
| apply | ✅ | `(apply fn vec)` |
| find-first / comp | ✅ | |
| reduce / mapcat | ✅ | |
| vec-builder / freeze-builder | ✅ | O(1) amortized 빌더 |

### 2-C. 문자열

| 함수 | 상태 |
|------|------|
| str (concat) | ✅ |
| str-length / str-slice / substring | ✅ |
| str-split / str-join | ✅ |
| str-includes / str-starts-with / str-ends-with | ✅ |
| str-to-upper / str-to-lower | ✅ |
| str-trim / str-pad-left / str-pad-right | ✅ |
| str-replace / str-replace-re / str-replace-all-re | ✅ |
| str-match / str-match-all / str-test | ✅ (POSIX regex) |
| str-index-of / str-repeat | ✅ |
| str-to-num / char-at / char-code-at | ✅ |
| html-escape | ✅ |
| num (to-string) | ✅ |

### 2-D. I/O & 디버깅

| 함수 | 상태 | 비고 |
|------|------|------|
| println / print | ✅ | |
| **inspect** | ✅ 2026-06-16 | 8KB 버퍼 pretty-print → 문자열 반환 |
| **pp** | ✅ 2026-06-16 | stderr에 `[pp] val` + 값 반환 (파이프용) |
| file-read / file-write / file-append | ✅ | |
| file-exists? / file-is-dir? / file-is-file? | ✅ | |
| file-list / file-mkdir / file-delete | ✅ | |

### 2-E. HTTP 서버

| 기능 | 상태 | 비고 |
|------|------|------|
| server_get / server_post | ✅ | 핸들러는 반드시 **문자열** 이름 |
| server_route (임의 메서드) | ✅ | |
| server_start / server_stop | ✅ | |
| server_json / server_html / server_html_cookie | ✅ | |
| server_redirect / server_status | ✅ | |
| server_set_cookie | ✅ | |
| server_req_param (경로 파라미터) | ✅ | |
| server_req_query (쿼리스트링) | ✅ | |
| server_req_body / server_req_json | ✅ | |
| server_req_header | ✅ | |
| WebSocket (ws_send / ws_recv / ws_broadcast) | ✅ | RFC 6455 |
| HTTPS (TLS) | ✅ | SSL/OpenSSL |

### 2-F. HTTP 클라이언트

| 함수 | 상태 | 반환 |
|------|------|------|
| http-get | ✅ | `{status body headers}` 맵 |
| http-post | ✅ | `{status body headers}` 맵 |
| http-get-headers / http-post-headers | ✅ | 커스텀 헤더 포함 |
| http-post-stream / http-stream-collect | ✅ | SSE/streaming |

**⚠️ 함정**: `(get (http-get url) "body")` 필수 — http-get 결과가 문자열이 아닌 맵

### 2-G. SQLite

| 함수 | 상태 | 비고 |
|------|------|------|
| fxb_sqlite_open | ✅ | 절대경로 필수 |
| fxb_sqlite_query | ✅ | SELECT → 벡터 |
| fxb_sqlite_exec | ✅ | INSERT/UPDATE/DELETE → nil |
| fxb_sqlite_query_p / fxb_sqlite_exec_p | ✅ | Prepared (SQL 인젝션 방어) |
| fxb_sqlite_one | ✅ | 단일 row → 맵 |
| fxb_sqlite_close | ✅ | |

**⚠️ 함정**: 외부 입력이 포함된 SQL은 반드시 `_p` 버전 사용

### 2-H. MariaDB

| 함수 | 상태 |
|------|------|
| mariadb_connect | ✅ (5인자: host port user pass db) |
| mariadb_query / mariadb_exec | ✅ |
| mariadb_query_p / mariadb_exec_p | ✅ (Prepared) |
| mariadb_close | ✅ |

### 2-I. 기타 런타임

| 함수 | 상태 |
|------|------|
| json-stringify / json-parse / json-pretty | ✅ |
| sha256 / md5 | ✅ |
| uuid | ✅ |
| now (ISO8601) / now-ms (epoch ms) | ✅ |
| sleep (ms) | ✅ |
| random | ✅ |
| math-sqrt / floor / ceil / abs / clamp | ✅ |
| process-run / shell_run | ✅ |
| process-pid / process-cwd | ✅ |
| env-get | ✅ |
| cli-args | ✅ |
| auth-jwt-sign / auth-jwt-verify / auth-jwt-decode | ✅ |
| auth-hash-password / auth-verify-password | ✅ |
| smtp_send / smtp_send_html | ✅ |
| run-parallel (future 여러 개) | ✅ |
| bit-and/or/xor/shl/shr | ✅ |

### 2-J. 컴파일 타임 경고 (2026-06-16 추가)

| 경고 | 설명 |
|------|------|
| W1 arity 불일치 | `(defn f [$x $y])` → `(f 1 2 3)` 호출 시 경고 |
| W2 nil 전파 | `(+ (get m "k") 1)` — nil? 체크 없이 get 결과 산술 사용 |
| W3 타입 불일치 | `(+ "hello" 1)` — 문자열+숫자 리터럴 혼용 |
| 미정의 이름 | `$undefined-var` 사용 시 경고 |

### 2-K. 스택 트레이스 (2026-06-16 추가)

```
Uncaught error: ArithmeticError: 0으로 나눌 수 없습니다
Stack trace:
  at div_by_zero
  at calc
  at run
```

- 모든 `defn`에 자동으로 push/pop 삽입 (코드 작성 불필요)
- `fl_throw` 경로 에러만 표시 (catch된 에러는 표시 안 함)
- 함수명은 C-mangled 형식 (my-func → `my_func`)

---

## 3. ❌ 미구현 목록

### 3-A. 언어 기능

| 기능 | 우선순위 | 대안 |
|------|---------|------|
| **다중 반환값** | P2 | map 반환으로 우회 |
| **가변 인자 (varargs)** | P2 | 벡터 인자로 우회 `(fn [$args] ...)` |
| **구조체 destructuring** | P2 | `(let [[$x (get $m :x)]] ...)` |
| **매크로 정의 (defmacro)** | P3 | — |
| **멀티메서드** | P3 | cond로 우회 |
| **lazy sequence** | P3 | — |
| **네임스페이스** | P3 | 파일 분리 + load로 우회 |
| **메타데이터** | P3 | — |
| **protocol / interface** | P3 | — |
| **스택 트레이스 — catch 경로** | P2 | uncaught 에러만 표시 |
| **스택 트레이스 — C 함수명 → FL 변환** | P3 | `my_func` 표시 (kebab 복원 안 됨) |

### 3-B. 런타임 함수

| 함수 | 우선순위 | 대안 |
|------|---------|------|
| `reduce-kv` | P2 | `map-entries`로 우회 |
| `merge` (맵 합치기) | P2 | `obj-merge`로 우회 |
| `select-keys` | P2 | `obj-pick`으로 우회 |
| `partition` / `partition-all` | P2 | `for`+`take`로 수동 구현 |
| `interleave` / `interpose` | P3 | — |
| `printf` / `format` | P2 | `(str "... #{$x}")` 으로 우회 |
| `read-line` (stdin) | P2 | — |
| `parse-date` / `format-date` | P2 | shell_run "date" 로 우회 |
| `base64-encode / decode` | P2 | — |
| `gzip / inflate` | P3 | — |
| `tcp-connect` (raw TCP) | P3 | http-* 로 대부분 해결 |
| Redis 클라이언트 | P2 | — |

### 3-C. 개발 도구

| 기능 | 우선순위 | 비고 |
|------|---------|------|
| **REPL** (대화형) | P2 | fl-repl.sh 있지만 미완성 |
| **핫 리로드** (fl-watch.sh) | P2 | 파일 변경 감지 → 재빌드 |
| **타입 체커** | P3 | 컴파일 타임 타입 추론 |
| **단계별 디버거** | P3 | 현재: pp + 스택 트레이스 |
| **성능 프로파일러** | P3 | — |
| **메모리 분석기** | P3 | — |

---

## 4. ⚠️ 조심해야 할 것 (실제 삽질 목록)

### C-1. defn 본문은 단일 표현식 ⭐⭐⭐⭐⭐

```lisp
;; ❌ 조용히 두 번째 표현식 무시됨 (컴파일 오류 없음!)
(defn bad [$x]
  (println "hello")  ;; 이것만 실행됨
  $x)                ;; 이것은 무시

;; ✅ do로 감싸기
(defn good [$x]
  (do
    (println "hello")
    $x))
```

### C-2. 서버 핸들러는 반드시 문자열 이름 ⭐⭐⭐⭐⭐

```lisp
;; ❌ 인라인 fn 불가
(server_get "/" (fn [$req] (server_json "ok")))

;; ✅ 이름 있는 defn + 문자열 전달
(defn handle-index [$req] (server_json (json_stringify "ok")))
(server_get "/" "handle-index")
```

### C-3. kebab-case vs underscore ⭐⭐⭐⭐⭐

```lisp
;; ❌ v11 습관 (fx에서 미정의 이름)
(server-get "/" "h")
(json-stringify $m)

;; ✅ fx 방식 (underscore)
(server_get "/" "h")
(json_stringify $m)
```

### C-4. http-get 반환값은 맵 ⭐⭐⭐⭐

```lisp
;; ❌ 문자열이라 착각
(let [[$res (http-get url)]
      [$data (json_parse $res)]]  ;; 오류: $res는 맵

;; ✅ body 꺼내기
(let [[$res (http-get url)]
      [$body (get $res "body")]
      [$data (json_parse $body)]]
```

### C-5. SQL 인젝션 — 외부 입력은 반드시 _p ⭐⭐⭐⭐⭐

```lisp
;; ❌ SQL 인젝션 위험
(fxb_sqlite_exec db (str "INSERT INTO t VALUES ('" $user-input "')" ))

;; ✅ Prepared statement
(fxb_sqlite_exec_p db "INSERT INTO t VALUES (?)" [$user-input])
```

### C-6. defn main → C main() 충돌 ⭐⭐⭐⭐

```lisp
;; ❌ C 컴파일 에러
(defn main [] ...)

;; ✅ 다른 이름 사용
(defn run [] ...)
(defn start [] ...)
(defn app-main [] ...)
```

### C-7. loop 바인딩 변수 — 현재 세션에서 수정됨

과거: `(loop [$i 0 $acc []] ...)` 에서 `$i`, `$acc` 미정의 경고 발생  
현재: `cgc-loop`가 `outer-params-atom`에 자동 등록 → 경고 없음 ✅

### C-8. atom이 Arena-allocated 값 보유 시 dangling ⭐⭐⭐

```lisp
;; 문제: 요청 완료 후 Arena 해제 → atom 보유값 dangling
;; 해결: atom에 저장 전 fl_heap_copy (RC-Heap 복사)
;; → fx Runtime이 자동 처리 (aliases.c RC-Heap 구현)
```

### C-9. fxb_sqlite_query vs fxb_sqlite_one ⭐⭐⭐

```lisp
;; fxb_sqlite_query → 항상 벡터 반환 (빈 결과 = [])
;; fxb_sqlite_one → 단일 맵 반환 (결과 없으면 nil)
;; COUNT 결과 읽기:
(get (fxb_sqlite_one db "SELECT COUNT(*) AS c FROM t") "c")
```

### C-10. WS 핸들러 — 1인자, 내부 recv 루프 직접 구현 ⭐⭐⭐

```lisp
;; WS 핸들러는 연결 소켓을 인자로 받는 함수
;; 내부에서 loop + ws_recv + ws_send 로 직접 처리
(defn ws-handle [$sock]
  (loop []
    (let [[$msg (ws_recv $sock)]]
      (if (null? $msg) nil
        (do (ws_send $sock (str "echo: " $msg))
            (recur))))))
```

### C-11. pp / inspect 함수명 충돌 주의 ⭐⭐

```lisp
;; pp는 FL 변수명으로도 쓸 수 있으나 built-in과 충돌
;; (defn pp ...) 금지 — built-in pp(디버그 출력)를 덮어씀
```

---

## 5. 🔮 향후 계획 (우선순위 순)

### Phase A — P1 (다음 작업 후보)

| 항목 | 이유 |
|------|------|
| `merge` 함수 추가 | 맵 합치기가 너무 자주 필요 |
| `printf` / `format` 추가 | 숫자 포맷팅 (`%04d`, `%.2f`) 이 없음 |
| `base64-encode / decode` | JWT payload, HTTP Basic Auth 등 |
| `partition` 추가 | 배치 처리 패턴에 필수 |
| REPL 완성 (fl-repl.sh) | 개발 속도 향상 |
| 스택 트레이스 — catch 경로 | longjmp 후 depth 복원 필요 |

### Phase B — P2 (중기)

| 항목 | 이유 |
|------|------|
| Redis 클라이언트 | 캐시/세션 서비스 필요 |
| `read-line` (stdin) | CLI 도구 작성 가능 |
| 날짜 함수 (`parse-date`, `format-date`) | 비즈니스 앱에 자주 필요 |
| `select-keys` / `reduce-kv` | Clojure 개발자 습관 |
| 타입 에러 더 많이 (컴파일 타임) | 런타임에서만 터지는 것들을 앞당김 |

### Phase C — P3 (장기)

| 항목 | 이유 |
|------|------|
| 단계별 디버거 | 현재 pp + println이 유일한 도구 |
| gzip / base64 | 파일 압축/전송 |
| 타입 추론 | 안전성 향상 |
| 네임스페이스 | 대형 앱 구조화 |

---

## 6. 컴파일러 수정 시 절차

### 일반 수정 (dispatch 추가, 헬퍼 함수 추가 등)

```bash
# 1. cgc-main.fl 수정
# 2. 고정점 검증
./verify-fixpoint.sh
# → gen-a == gen-b == gen-c PASS이면 완료
# → cgc-bin은 자동 교체하지 않음
```

### top-level 코드 생성 방식 변경 시 (cgc-top-level, cgc-defn-stmts 등)

동일한 `verify-fixpoint.sh` 흐름으로 3세대 재현을 확인한다:

```bash
CGC_MAIN="$(pwd)/self/cgc-main.fl"
CGC_BIN="${CGC_BIN:-../freelang-afj/bin/cgc-bin}"
RUNTIME="$(pwd)/runtime"
TMPDIR=$(mktemp -d /tmp/cgc-2stage-XXXXXX)
RUNTIME_SRCS="$RUNTIME/core.c $RUNTIME/collection.c $RUNTIME/io.c $RUNTIME/math.c \
  $RUNTIME/error.c $RUNTIME/process.c $RUNTIME/json.c $RUNTIME/aliases.c \
  $RUNTIME/user-fns.c \
  $RUNTIME/cgc-bridge.c $RUNTIME/gc.c $RUNTIME/http.c $RUNTIME/websocket.c \
  $RUNTIME/sqlite.c $RUNTIME/mariadb.c $RUNTIME/debug.c $RUNTIME/http_client.c \
  $RUNTIME/regex.c $RUNTIME/smtp.c"
LINK="-lm -lpthread -ldl -lsqlite3 -lssl -lcrypto -lcurl"

# 검증 흐름: cgc-bin → gen-a → gen-a-bin → gen-b → gen-b-bin → gen-c
# 검증: SHA(gen-a) == SHA(gen-b) == SHA(gen-c)
# cgc-bin 공식 교체는 자동으로 수행하지 않음
```

### 새 런타임 함수 추가 시 체크리스트

```
[ ] runtime/aliases.c 또는 해당 .c 파일에 구현
[ ] runtime/runtime.h에 선언 추가 (누락 시 → gcc가 int 반환으로 추론 → 쓰레기값)
[ ] self/cgc-main.fl dispatch에 추가 [(= $op "함수명") ...]
[ ] verify-fixpoint.sh 통과 확인
[ ] 테스트 코드 작성
```

---

## 7. 핵심 경로

| 항목 | 경로 |
|------|------|
| 컴파일러 소스 | `self/cgc-main.fl` |
| 컴파일러 바이너리 | `CGC_BIN` 또는 `../freelang-afj/bin/cgc-bin` |
| 런타임 소스 | `runtime/` |
| 별칭/헬퍼 함수 | `runtime/aliases.c` |
| 빌드 스크립트 | `./fl-build.sh` |
| 신규 앱 생성 | `bash ./fl-new.sh <앱명> <포트>` |
| 고정점 검증 | `./verify-fixpoint.sh` |
| 트랩 명세 (AIRC) | `FX-TRAPS.airc` |
| 기존 CLAUDE.md | `CLAUDE.md` (API 레퍼런스) |
| Gogs (컴파일러) | `https://gogs.dclub.kr/kim/freelang-v11` |
| Gogs (런타임/fx) | `https://gogs.dclub.kr/kim/freelang-v11-fx` |

---

## 8. 현재 고정점 이력

| SHA | 날짜 | 내용 |
|-----|------|------|
| `10848c4` | 2026-09-27 | 검증기 경로·user-fns.c 수정, 3세대 SHA 고정점·clean checkout 재현 ✅ |
| `d676d4bb` | 2026-06-16 | W1/W2/W3 경고 추가 |
| `dd1784c7` | 2026-06-16 | loop 변수 false positive 수정 + #line 지시자 |
| `2dd57513` | 2026-06-16 | 스택 트레이스 + assoc-in + inspect/pp (historical) |

---

_이 문서는 fx 상태가 변경될 때마다 업데이트한다._  
_최신 상태: `git log --oneline -5 self/cgc-main.fl`_
