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

(mariadb_query db "SELECT * FROM table")
(mariadb_exec  db "INSERT INTO ...")
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

---

## 🐛 알려진 버그 & 수정 이력

| 날짜 | 버그 | 수정 | 커밋 |
|------|------|------|------|
| 2026-06-14 | json_stringify %g 6자리 절삭 | 정수float→%lld, 실수→%.17g | `34c10ea` |
| 2026-06-14 | json_stringify 64KB 고정 버퍼 잘림 | realloc 동적 확장 | `c5ee522` |
| 이전 | server_req_param으로 쿼리스트링 읽기 불가 | server_req_query 사용 | 문서화 |
