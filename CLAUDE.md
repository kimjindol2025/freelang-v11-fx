# FreeLang fx (C Native) — Claude 레퍼런스

> **상태**: 프로덕션 사용 가능 (2026-06-15 기준)  
> **런타임**: C 네이티브 ELF 바이너리 (Node.js 불필요)  
> **빌드**: `bash /home/kimjin/freelang-v11-fx/fl-build.sh server.fl output-binary`

---

## 📌 핵심 경로 & Gogs (세션 시작 시 참고)

| 항목 | 경로 / URL |
|------|-----------|
| **컴파일러 소스** | `/home/kimjin/freelang-v11/self/cgc-main.fl` |
| **컴파일러 바이너리** | `/home/kimjin/freelang-v11/bin/cgc-bin` |
| **런타임 소스** | `/home/kimjin/freelang-v11-fx/runtime/` |
| **빌드 스크립트** | `/home/kimjin/freelang-v11-fx/fl-build.sh` |
| **신규 앱 생성** | `/home/kimjin/freelang-v11-fx/fl-new.sh <앱명> <포트>` |
| **고정점 검증** | `/home/kimjin/freelang-v11-fx/verify-fixpoint.sh` |
| **고정점 로그** | `/home/kimjin/freelang-v11-fx/FIXPOINT_LOG.md` |
| **Gogs (컴파일러)** | `https://gogs.dclub.kr/kim/freelang-v11` |
| **Gogs (런타임/fx)** | `https://gogs.dclub.kr/kim/freelang-v11-fx` |

**현재 고정점 SHA**: `691b0aae79206814` (2026-06-15, ✅ 완전)

---

## 🔨 fx 앱 만들 때 이렇게 해라

### 1. 새 앱 생성

```bash
# fl-new.sh 로 보일러플레이트 자동 생성
bash /home/kimjin/freelang-v11-fx/fl-new.sh my-app 40290
# → 폴더 생성 + server.fl 템플릿 + .projectrc.json + PM2 등록

# 또는 수동
mkdir ~/kim/Desktop/kim/01_Active_Projects/my-app
cd ~/kim/Desktop/kim/01_Active_Projects/my-app
```

### 2. server.fl 작성 규칙

```lisp
;; ✅ fx 방식 (underscore)
(defn handle-index [$req]
  (server_json (json_stringify {"ok" true})))

(server_get "/" "handle-index")   ;; 핸들러는 반드시 문자열 이름
(server_start 40290)

;; ❌ v11 방식 (fx에서 컴파일 오류)
(server-get "/" (fn [$req] ...))  ;; 인라인 fn 불가, kebab-case 불가
```

### 3. 빌드 & 배포

```bash
# 빌드
bash /home/kimjin/freelang-v11-fx/fl-build.sh server.fl my-app

# PM2 배포
pm2 start ./my-app --name my-app

# 재빌드 + 무중단 재시작
bash /home/kimjin/freelang-v11-fx/fl-build.sh server.fl my-app && pm2 reload my-app
```

### 4. 컴파일러 변경 후 반드시

```bash
# cgc-main.fl 수정 시 — 고정점 검증 필수 (커밋 전)
bash /home/kimjin/freelang-v11-fx/verify-fixpoint.sh
# ✅ 완전: gen-a == gen-b == gen-c → 커밋 OK
# ⚠️ 부분: gen-b == gen-c → cgc-bin 교체 후 재검증
# ❌ 붕괴: gen-b != gen-c → 디버그 (커밋 금지)
```

### 5. 멀티파일 프로젝트 — (load "file.fl")

```lisp
;; 절대 경로 또는 상대 경로 모두 OK
(load "/home/kimjin/freelang-v11-fx/runtime/fx-std.fl")
(load "./helpers.fl")

;; 순환 참조 → 자동 감지·무시 (2026-06-15 cgc-bin)
;; 중복 load → 자동 무시 (한 번만 인라인)
;; 100개 파일 → 59ms (성능 문제 없음)
```

---

## ⚠️ 실전 함정 (fx-live-monitor 2026-06-15 발견)

### 1. extern FLValue 함수 — defn 내부 직접 호출 불가

`runtime.h`에 `extern FLValue`로 선언된 것들은 FL 런타임 함수 값. C 함수가 아님.
cgc-bin이 `defn` 안에서 이를 C 함수처럼 호출하는 코드를 생성하면 빌드 실패.

| FL 이름 | 상태 | 대안 |
|---------|------|------|
| `sqlite_exec`, `sqlite_query`, `sqlite_one` | `extern FLValue` ❌ | `fxb_sqlite_exec` ✅ |
| `sqlite_open`, `sqlite_close` | `extern FLValue` ❌ | `fxb_sqlite_open` ✅ |
| `server_req_body` | `extern FLValue` ❌ | `fxb_server_req_body` 또는 `(get $req "body")` ✅ |

### 2. 람다 클로저 캡처 버그 (defn 파라미터)

`defn`의 파라미터가 람다 안에서 참조될 때 C 클로저 env에 추가되지 않는 버그.

```lisp
;; ❌ 컴파일 에러 — broadcast 함수 파라미터 $msg가 람다에서 미등록
(defn broadcast [$msg]
  (reduce (fn [$acc $ws] (ws_send $ws $msg)) [] @$clients))

;; ✅ 우회: 파라미터를 atom에 저장 후 deref
(define $bcast-msg (atom ""))
(defn broadcast [$json]
  (swap! $bcast-msg (fn [$_] $json))
  (reduce (fn [$acc $ws]
            (let [$m @$bcast-msg]
              (try (do (ws_send $ws $m) (push $acc $ws))
                   (catch $e $acc))))
          [] @$clients))
```

### 2-b. 람다 클로저 — map 키 이름이 변수명과 같으면 충돌

`reduce` lambda 안에서 맵 리터럴의 **키 이름**이 lambda 내부 let 바인딩 변수명과 같으면
cgc-bin이 해당 키를 자유변수로 분류해 클로저 env에 추가한다.
결과: gcc에서 `'acc' undeclared` 같은 오류 발생.

```lisp
;; ❌ "acc", "id" 키가 let 바인딩 $acc, $id와 이름 충돌
(reduce
  (fn [$st $entry]
    (let [[$acc (get $st "acc")]
          [$id  (get $st "id")]]
      {"acc" (str $acc ...) "id" (+ $id 1)}))
  {"acc" "" "id" 0}
  $list)

;; ✅ 키 이름을 변수명과 다르게
(reduce
  (fn [$st $entry]
    (let [[$xacc (get $st "xacc")]
          [$xid  (get $st "xid")]]
      {"xacc" (str $xacc ...) "xid" (+ $xid 1)}))
  {"xacc" "" "xid" 0}
  $list)
```

**규칙**: `reduce` 상태 맵의 키는 lambda 내 변수명과 절대 겹치지 않게 짓는다.
예: `"acc"` → `"xacc"/"istr"`, `"id"` → `"xid"/"iid"`.

### 3. server_json은 JSON 문자열만 받음

```lisp
;; ❌ 직접 맵 전달 → Content-Length: 0 (빈 응답)
(server_json {"ok" true "data" $rows})

;; ✅ json_stringify 필수
(server_json (json_stringify {"ok" true "data" $rows}))
```

### 4. WS 핸들러 시그니처 — 1인자, 내부 recv 루프

```lisp
;; WSHandlerFn typedef: FLValue (*)(FLValue ws) — 1인자만!
(defn handle-ws [$ws]
  ;; recv 루프는 직접 구현 (TCO 재귀 권장)
  (defn recv-loop [$ws]
    (let [$data (ws_recv $ws)]
      (if (null? $data)
        (cleanup $ws)
        (do (handle-frame $ws $data) (recv-loop $ws)))))
  (swap! $clients (fn [$cs] (push $cs $ws)))
  (recv-loop $ws))
```

---

## ⚡ v11 vs fx 핵심 차이

| 항목 | FreeLang v11 (인터프리터) | FreeLang fx (C 네이티브) |
|------|--------------------------|------------------------|
| 실행 방식 | `node bootstrap.js run server.fl` | `./fx-binary` (ELF) |
| 메모리 | 70-90MB | **3-5MB** |
| 속도 (fib30) | 2분 11초 | **0.4초** (330x) |
| 빌드 과정 | 없음 | `.fl → C → gcc → ELF` |
| 함수명 스타일 | kebab-case (`server-get`) | underscore (`server_get`) |

---

## 🔌 HTTP 서버 API

### 라우트 등록 (함수명 문자열 방식)

```lisp
;; ✅ fx 방식 — 함수명을 문자열로 전달
(defn handle-get [$req]
  (server_json (json_stringify {"ok" true})))

(server_get    "/path"       "handle-get")
(server_post   "/path"       "handle-post")
(server_put    "/path/:id"   "handle-put")
(server_delete "/path/:id"   "handle-delete")
(server_patch  "/path"       "handle-patch")

;; ❌ v11 방식 금지 (fx에서 컴파일 오류)
(server-get "/path" (fn [$req] ...))
```

### 요청 파라미터 읽기

```lisp
;; 경로 파라미터 (/api/users/:id)
(server_req_param $req "id")

;; 쿼리스트링 (?name=foo&limit=10)
(server_req_query $req "name")
(server_req_query $req "limit")

;; Request body (POST/PUT) — 두 가지 방법
(let [$body (get $req "body")]       ;; Content-Type: application/json → 이미 파싱된 맵
  (get $body "field"))

;; 또는 (server_req_body는 extern FLValue → defn 내부 호출 시 fxb_ 사용)
(let [$body (json_parse (fxb_server_req_body $req))]
  (get $body "field"))

;; 헤더
(server_req_header $req "X-User-Id")
(server_req_header $req "Authorization")
```

> **⚠️ 함정**: `server_req_param`은 path param 전용. 쿼리스트링에 쓰면 nil 반환.

### 응답

```lisp
;; JSON 응답 — json_stringify 필수!
(server_json (json_stringify {"ok" true "data" $rows}))

;; HTML 응답
(server_html "<!DOCTYPE html><html>...")

;; 상태 코드 지정
(server_status 400 "{\"error\":\"bad request\"}")
(server_status 401 "{\"error\":\"unauthorized\"}")
(server_status 404 "{\"error\":\"not found\"}")

;; 리다이렉트
(server_redirect "/login")

;; 텍스트
(server_text "Hello World")
```

> **⚠️ 함정**: `server_json`은 이미 직렬화된 문자열을 받음.  
> v11처럼 `(server-json {"key" "val"})` 불가 → `(server_json (json_stringify {...}))` 필수.

---

## 🌐 HTTP 클라이언트 API (outbound)

> 2026-06-14 추가. 기존엔 서버만 있었음. 순수 C 소켓, libcurl·Node 무의존.

```lisp
;; GET — 응답 본문을 문자열로 반환 (실패 시 nil)
(let [[$body (http_get "http://localhost:18080/api")]]
  (if (nil? $body) (println "요청 실패") (println $body)))
```

- **HTTP 전용 — HTTPS(TLS) 미지원.** `https://` 주면 nil + 진단.
- **실패 시 nil을 반환하되, 원인을 stderr에 항상 출력**한다 (FL_DEBUG 무관):
  `[http-get] 연결 실패 localhost:19999 (후보 2개 시도) — Connection refused`
  진단을 끄려면 환경변수 `FL_HTTP_QUIET=1`.
- `localhost`가 IPv6 `::1`로 해석돼도 IPv4로 자동 fallback (getaddrinfo 순회).
- cgc-bin에 예약된 빌트인: `http_get` / `http_get_headers` / `http_post` / `http_post_headers`
  (현재 런타임 구현은 `http_get`).

---

## 🗃️ SQLite API

```lisp
;; DB 열기 (최상단 define으로 싱글톤)
(define db (sqlite_open "data/app.db"))

;; 테이블 생성 / DDL
(sqlite_exec db "CREATE TABLE IF NOT EXISTS users (id TEXT PRIMARY KEY, name TEXT)")

;; INSERT / UPDATE / DELETE — ? 바인딩 (SQL 인젝션 방어)
(sqlite_exec_p db "INSERT INTO users VALUES (?, ?)" (list $id $name))
(sqlite_exec_p db "UPDATE users SET name=? WHERE id=?" (list $name $id))
(sqlite_exec_p db "DELETE FROM users WHERE id=?" (list $id))

;; SELECT → 배열 반환
(sqlite_query_p db "SELECT id, name FROM users WHERE active=? ORDER BY name" (list true))

;; SELECT → 단일 행 반환 (없으면 nil)
(sqlite_one_p db "SELECT * FROM users WHERE id=?" (list $id))

;; 파라미터 없는 경우 (기존 패턴도 사용 가능)
(sqlite_query db "SELECT id, name FROM users ORDER BY name")
(sqlite_exec  db "CREATE TABLE IF NOT EXISTS ...")
```

> **✅ fx에 `?` 바인딩 있음!** `sqlite_exec_p` / `sqlite_query_p` / `sqlite_one_p` 사용.  
> 3번째 인자로 `(list ...)` 전달. nil/bool/int/float/string 자동 바인딩.  
> FTS5 `MATCH ?` 도 지원됨. esc() 헬퍼는 더 이상 불필요.
>
> **⚠️ defn 내부에서 `sqlite_exec` 호출 실패** (2026-06-15 발견):  
> `sqlite_exec`는 `extern FLValue` (FL 런타임 값)으로 선언돼 있어 `defn` 안에서  
> 직접 C 함수 호출 시 `called object is not a function` 에러 발생.  
> **Fix**: `fxb_sqlite_open` / `fxb_sqlite_exec` / `fxb_sqlite_query` 사용 (실제 C 함수).
>
> ```lisp
> ;; ✅ defn 내부에서도 안전
> (define db (fxb_sqlite_open DB_PATH))
> (fxb_sqlite_exec db "CREATE TABLE IF NOT EXISTS ...")
> (fxb_sqlite_query db "SELECT * FROM t")
> ;; _p 바인딩 버전도 동일
> (fxb_sqlite_exec_p db "INSERT INTO t VALUES (?,?)" (list $a $b))
> (fxb_sqlite_query_p db "SELECT * FROM t WHERE id=?" (list $id))
> ```

### ❌ 구 패턴 (사용 금지)

```lisp
;; 절대 금지 — SQL 인젝션 위험
(sqlite_exec db (str "INSERT INTO t VALUES ('" $val "')"))

### 시간 함수 패턴

```lisp
;; ISO 타임스탬프 (SQLite 함수 활용)
(defn now-iso []
  (get (sqlite_one db "SELECT datetime('now','localtime') as t") "t"))

;; Unix 밀리초
(defn now-ms []
  (get (sqlite_one db "SELECT (strftime('%s','now')*1000) as ms") "ms"))

;; now_ms 내장 함수도 있음
;; (fl_now_ms) → 밀리초 정수
```

---

## 📦 MariaDB API

```lisp
;; ⚠️ v11과 인자 순서 다름!
;; v11: mariadb_connect host user pass db    (4인자)
;; fx:  mariadb_connect host port user pass db  (5인자 — port 추가!)
(define db (mariadb_connect "localhost" 3306 "user" "pass" "dbname"))

;; 파라미터 없는 쿼리
(mariadb_query db "SELECT * FROM table")
(mariadb_exec  db "INSERT INTO ...")

;; ? 바인딩 (SQL 인젝션 방어) — 2026-06-14 추가
(mariadb_exec_p  db "INSERT INTO t VALUES (?,?)" (list $a $b))
(mariadb_query_p db "SELECT * FROM t WHERE id=?" (list $id))
(mariadb_one_p   db "SELECT * FROM t WHERE id=?" (list $id))
```

---

## 🔤 JSON API

```lisp
;; 직렬화 (동적 버퍼, 크기 제한 없음 — 2026-06-14 수정)
(json_stringify {"key" "value" "num" 42 "arr" (list 1 2 3)})

;; 파싱
(let [[$obj (json_parse (server_req_body $req))]]
  (get $obj "field"))

;; 부동소수점 정밀도 (2026-06-14 수정)
;; 정수값 float → 정수로 직렬화 (9227465.0 → 9227465)
;; 실수 → %.17g (전체 정밀도 보존)
```

---

## 🧵 빌드 & 배포

```bash
# 빌드
bash /home/kimjin/freelang-v11-fx/fl-build.sh server.fl output-name

# PM2 배포
pm2 start ./output-name --name my-app

# 재빌드 후 재시작
bash /home/kimjin/freelang-v11-fx/fl-build.sh server.fl my-app && pm2 reload my-app
```

### .projectrc.json 예시

```json
{
  "name": "fx-notes",
  "version": "1.0.0",
  "language": "FreeLang fx (C Native)",
  "status": "in_progress",
  "ports": [40284],
  "entry": "server.fl",
  "commands": {
    "build": "bash /home/kimjin/freelang-v11-fx/fl-build.sh server.fl fx-notes",
    "start": "pm2 start ./fx-notes --name fx-notes"
  }
}
```

---

## 📋 11절 코딩규칙 적용 (fx 특화)

### P0 규칙 (반드시 지킬)

```lisp
;; P0.1 — $ 파라미터 필수
(defn handle [$req]    ;; ✅
  (let [[$body (json_parse (server_req_body $req))]]
    (get $body "name")))

;; P0.4 — let 바인딩 한 줄에 하나
(let [[$id    (server_req_param $req "id")]  ;; ✅
      [$body  (json_parse (server_req_body $req))]
      [$name  (get $body "name")]]
  ...)

;; P0.5 — 문자열 보간 (str)
(str "Hello " $name "!")
```

### fx 전용 주의사항

```lisp
;; ❌ v11 스타일 (fx 컴파일 오류)
(server-get "/path" (fn [req] ...))     ;; 인라인 fn 핸들러
(server-json {"key" "val"})             ;; kebab-case API
(json-stringify $obj)                   ;; 하이픈 함수명

;; ✅ fx 올바른 패턴
(server_get "/path" "handle-name")      ;; 문자열 핸들러
(server_json (json_stringify {"key" "val"}))  ;; 언더스코어 + stringify
(json_stringify $obj)                   ;; 언더스코어
```

---

## ✅ 검증된 fx 앱 목록 (2026-06-14)

| 앱 | 포트 | 메모리 | Gogs |
|----|------|--------|------|
| fx-bench | 40280 | 3.4MB | kim/fx-bench |
| fx-metric-store | 40281 | 3.4MB | kim/fx-metric-store |
| fx-notes | 40284 | 4.5MB | kim/fx-notes |
| fl-kv | (기존) | 3.9MB | — |
| kim-notes | (기존) | 5.1MB | — |
| kim-short | (기존) | 3.4MB | — |
| fl-claude-chat | (기존) | 1.7MB | — |
| fx-subscription | 40285 | ~4MB | kim/fx-subscription |

---

## ⚠️ 함정 목록 (실제 삽질로 발견)

### 0. ⭐ `defn` 본문은 **단일 표현식** — 다중은 `do`/`let`로 감싸라

cgc-bin은 함수 본문을 단일 표현식으로 컴파일한다. 본문에 표현식을 여러 개
나열하면 **첫 번째만 실행되고 나머지는 조용히 무시**된다 (Clojure식 implicit-do 아님).

```lisp
;; ❌ "1"만 출력. "2" "3"은 사라짐 — 에러도 없이!
(defn f [] (println "1") (println "2") (println "3"))

;; ✅ do 로 감싸기
(defn f [] (do (println "1") (println "2") (println "3")))

;; ✅ let 본문은 다중 표현식 OK (비대칭 주의)
(defn f [] (let [[$x 1]] (println "1") (println "2") (println "3")))
```

**증상**: 함수 중간 로직이 통째로 실행 안 됨. 디버깅하면 마치 그 줄에서
프로그램이 죽은 것처럼 보이지만(exit 0), 실제로는 컴파일이 그 줄을 버린 것.
fx 빌트인(`http_get` 등)을 의심하기 전에 **본문이 `do`로 감싸였는지 먼저 확인**.

---

### 1. `replace_all`이 함수명도 치환

```bash
# ❌ replace_all로 테이블명만 바꾸려 했는데 함수명도 바뀜
# plans → fx_plans 치환 시 defn plans-all → defn fx_plans-all 됨
```

**대처**: 치환 후 반드시 `grep -n "defn fx_"` 로 함수명 오염 확인. SQL 문자열만 바꾸려면 패턴을 더 구체적으로 지정.

---

### 2. MariaDB `COUNT(*)` 비교 함정

```lisp
;; ❌ 오류 — COUNT 반환값 타입이 float일 수 있음
(if (= $cnt 0) ...)

;; ✅ 안전한 비교 — 문자열로 변환 후 비교
(if (or (nil? $cnt) (= (str $cnt) "0")) ...)
```

---

### 3. server_html 내부 더블쿼트 → FL 파서 오류

```lisp
;; ❌ FL 문자열 파서가 HTML 안의 " 를 문자열 끝으로 인식
(server_html "...<div onclick='selPlan=\"pro\"'>...")

;; ✅ 올바른 패턴 — HTML 속성은 single-quote, JS는 var/createElement
(server_html "...<div onclick='selPlan=p.id'>...")
;; 또는 DOM API 방식으로 onclick 설정
```

---

### 4. ~~MariaDB `mariadb_exec_p` 없음~~ → **추가됨 (2026-06-14)**

```lisp
;; ✅ ? 바인딩 사용 가능 (2026-06-14 추가)
(mariadb_exec_p  db "INSERT INTO t VALUES (?,?)" (list $a $b))
(mariadb_query_p db "SELECT * FROM t WHERE id=?" (list $id))
(mariadb_one_p   db "SELECT * FROM t WHERE id=?" (list $id))
```

> **참고**: `mariadb_exec_p`는 `mysql_real_escape_string` 기반으로 ? 치환. nil/bool/int/float/string 자동 처리.

---

### 5. server_html 내 JSON 이스케이프 문자 주의

```lisp
;; ❌ FL 문자열 안의 \" 가 파서 혼동 유발
(mariadb_exec db "INSERT INTO t(f) VALUES ('[\"a\",\"b\"]')")

;; ✅ JSON 없이 단순 문자열 사용하거나 features를 CSV로 저장
(mariadb_exec db "INSERT INTO t(f) VALUES ('a,b,c')")
```

---

## 🔀 Threading Macro (-> / ->>)

```lisp
;; -> : 결과를 다음 표현식의 첫 번째 인자로 삽입
(-> 3 add1 (+ 10) str)
;; = (str (+ (add1 3) 10))
;; = "14"

;; ->> : 결과를 다음 표현식의 마지막 인자로 삽입
(->> (list 1 2 3)
     (filter (fn [$x] (> $x 1)))
     (map (fn [$x] (* $x 2))))
;; = [4, 6]

;; bare symbol 및 call form 모두 지원
(-> data trim-spaces parse-json (get "field"))
```

**cgc-bin 버전**: cond flat pair + -> / ->> + mariadb_*_p 포함 (2026-06-14, SHA `52e7e185`)

---

## 🐛 알려진 버그 & 수정 이력

| 날짜 | 버그 | 수정 | 커밋 |
|------|------|------|------|
| 2026-06-15 | expand-loads 순환/중복 load 무한 재귀 | loaded-paths-atom visited set 추가 | `4efb40f5` |
| 2026-06-15 | 문자열 보간 `${var}` C 생성 오류 3종 | :value 필드 / push / c-name 수정 | `915c8a33` |
| 2026-06-14 | cond flat pair 미지원 (→ nil 반환) | cgc-cond-flat 추가, cgc-bin 교체 | SHA `52e7e185` |
| 2026-06-14 | -> / ->> threading macro 추가 | cgc-tf-build/cgc-tl-build 구현 | SHA `52e7e185` |
| 2026-06-14 | mariadb_exec_p / query_p / one_p 추가 | mysql_real_escape_string 기반 | mariadb.c |
| 2026-06-14 | json_stringify %g 6자리 절삭 | 정수float→%lld, 실수→%.17g | `34c10ea` |
| 2026-06-14 | json_stringify 64KB 고정 버퍼 잘림 | realloc 동적 확장 | `c5ee522` |
| 이전 | server_req_param으로 쿼리스트링 읽기 불가 | server_req_query 사용 | 문서화 |
