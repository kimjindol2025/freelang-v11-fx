# FreeLang fx (C Native) — Claude 레퍼런스

> **상태**: 프로덕션 사용 가능 (2026-06-14 기준)  
> **런타임**: C 네이티브 ELF 바이너리 (Node.js 불필요)  
> **빌드**: `bash fl-build.sh server.fl output-binary`

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

;; Request body (POST/PUT)
(let [[$body (json_parse (server_req_body $req))]]
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
| 2026-06-14 | cond flat pair 미지원 (→ nil 반환) | cgc-cond-flat 추가, cgc-bin 교체 | SHA `52e7e185` |
| 2026-06-14 | -> / ->> threading macro 추가 | cgc-tf-build/cgc-tl-build 구현 | SHA `52e7e185` |
| 2026-06-14 | mariadb_exec_p / query_p / one_p 추가 | mysql_real_escape_string 기반 | mariadb.c |
| 2026-06-14 | json_stringify %g 6자리 절삭 | 정수float→%lld, 실수→%.17g | `34c10ea` |
| 2026-06-14 | json_stringify 64KB 고정 버퍼 잘림 | realloc 동적 확장 | `c5ee522` |
| 이전 | server_req_param으로 쿼리스트링 읽기 불가 | server_req_query 사용 | 문서화 |
