# FX Result 관례

상태: `Phase 0` (문서·관례만)  
기록일: 2026-10-12  
상위: [`decisions/SIX-ITEMS-BY-DIALECT.md`](decisions/SIX-ITEMS-BY-DIALECT.md) 3항  
소스: `fx-std.fl` §6, `fx-std/safe.fl`  
강제: **컴파일러 강제 없음** (AFJ에서 먼저 — 결정 문서 §2.2). FX는 병행 API·관례만.

## 1. Result 형태

```text
성공: [값   nil]
실패: [nil  메시지]
```

`fx-std.fl` 실코드:

```freelang
(defn result-ok [$v]    [$v nil])
(defn result-fail [$e]  [nil $e])
(defn result-ok? [$r]   (and (not (null? $r)) (null? (get $r 1))))
(defn result-err? [$r]  (or (null? $r) (not (null? (get $r 1)))))
```

주석에 적힌 `(ok …)` / `(fail …)` / `(ok? …)` 별칭은 **이 파일에 defn으로 없음** — 실제 이름은 `result-ok` / `result-fail` / `result-ok?` / `result-err?`.

패턴:

```freelang
(let [[$v $e] (db-query-safe $db $sql $params)]
  (if $e (handle-err $e) (use $v)))
```

## 2. `*-safe` / Result 래퍼 목록 (`fx-std.fl`)

| 이름 | 감싸는 것 | 실패 판정 (코드 기준) |
|------|-----------|------------------------|
| `db-query-safe` | `fxb_sqlite_query_p` | 반환이 `string?` → fail |
| `db-exec-safe` | `fxb_sqlite_exec_p` | 반환이 `string?` → fail |
| `db-one-safe` | `fxb_sqlite_query_p` 후 첫 행 | 문자열이면 fail; 0행이면 **ok + nil** |
| `http-get-safe` | `http_get` | `status=0` 또는 `status>=400` → fail |
| `json-parse-safe` | `json_try_parse` | C Result 그대로 |
| `file-read-safe` | `file-read` | `null?` → fail `"파일 없음"` |
| `shell-run-safe` | `shell_run` | `null?` → fail (아래 §4 실측 주의) |
| `mariadb-query-safe` | `mariadb_query` | `string?` → fail |
| `mariadb-exec-safe` | `mariadb_exec` | `string?` → fail |
| `mariadb-connect-safe` | `mariadb_connect` | `string?` → fail |

부가 (`fx-std.fl` / `fx-std/safe.fl`, Result 또는 기본값):

| 이름 | 역할 |
|------|------|
| `json_safe_get` / `get-in-2` | 맵 접근, 없으면 기본값·nil (Result 아님) |
| `safe-int` / `safe-float` | 변환 실패 시 기본값 |
| `safe-nth` / `safe-nth-or` | 범위 밖 → Result fail / 기본값 |
| `require-val` / `require-row` / `require-fields` | nil·빈 필드 → Result fail |
| `retry-safe` (`fx-std/retry.fl`) | 재시도 후 기본값 |
| `safe-tick-run` | set_interval 틱 throw 삼킴 (Result 아님) |

### 2.1 래퍼 실측 (2026-10-12, 설치본 cgc SHA `63411319…`)

```text
(json-parse-safe "{bad")
  → [nil, "[json] 파싱 실패: …"]  result-err?=true

(file-read-safe "/tmp/fx-no-such-file-xyz-4242")
  → [nil, "파일 없음"]  result-err?=true

(http-get-safe "http://127.0.0.1:1/")
  → [nil, "연결 실패: Couldn't connect to server"]  result-err?=true

(db-query-safe $c "NOT SQL" (list))
  → [nil, "[sqlite] 준비 실패: near \"NOT\": syntax error"]  result-err?=true
```

명령: `fl-build` + `/tmp/fx-res-std.fl` (load `fx-std.fl`).

## 3. 실패 시 nil을 돌려주는 빌트인 — **실측분만**

아래는 **실패(또는 부재)를 만들어 실행했을 때** `nil?`가 true인 것만 넣는다.  
소스에 `return fl_nil()`이 있어도 실패 경로를 안 돌렸으면 넣지 않는다.

| 빌트인 | 실패/부재 입력 | 실측 출력 | 비고 |
|--------|----------------|-----------|------|
| `file-read` | 없는 경로 `/tmp/fx-no-such-file-xyz-4242` | `nil` (`nil?=true`) | kebab `file-read` → `fl_file_read`. underscore `file_read`는 이 노드에서 **gcc 실패**(확인됨) |
| `get` | 없는 키 `(get {"a" 1} "b")` | `nil` | 부재 ≠ 예외 |
| `nth` | `(nth [10 20] 99)` | `nil` | 범위 밖 |
| `first` | `(first [])` | `nil` | 빈 벡터 |
| `env_get` | 없는 환경변수명 | `nil` | |
| `regex_find` | 패턴 불일치 `(regex_find "a+" "bbb")` | `nil` | |
| `fxb_sqlite_one` | 0행 SQL `SELECT 1 WHERE 0` | `nil` | **에러가 아니라 빈 결과**. 잘못된 SQL은 아래 §4 |

`json_try_parse` 실패는 nil 단일이 아니라 **Result 벡터** `[nil, 메시지]` (실측).

## 4. 실패해도 nil이 아닌 것 — 실측 (혼동 방지)

| 빌트인 | 실패 입력 | 실측 | 형태 |
|--------|-----------|------|------|
| `json_parse` | `"{not-json"` | Uncaught `[json] 파싱 실패: …` | **throw** |
| `http_get` | `http://127.0.0.1:1/` | 맵 `status=0`, `error=…` | nil 아님 (`nil?=false`) |
| `fxb_sqlite_query` | `"NOT SQL"` | 문자열 `[sqlite] 준비 실패: …` | `nil?=false` |
| `(db "../escape")` | 잘못된 DB 이름 | 문자열 `[DB_NAME_INVALID] …` | `nil?=false` |
| `mariadb_connect` | 이 노드 | 문자열 `[mariadb] 라이브러리 없음` (libmysqlclient 없음) | `nil?=false` |
| `regex_match` | 불일치 | `false` | nil 아님 |
| `shell_run` | `false` / 없는 명령 | 빈 문자열로 보임 (`nil?=false`) | **`shell-run-safe`의 null? 판정과 어긋날 수 있음** — 래퍼 실효는 확인 안 됨 |

## 5. 관례 요약 (AI용 한 줄)

```text
원시 빌트인 실패 ≠ 항상 nil.
nil인 것: file-read 부재, get/nth/first 부재, env 부재, regex_find 불일치, sqlite_one 0행.
문자열 에러: sqlite query/exec 실패, db 이름 거부, mariadb(이 노드).
throw: json_parse.
맵 status: http_get.
명시적 Result가 필요하면 fx-std *-safe / json_try_parse 를 쓴다.
컴파일러는 Result 미처리를 거부하지 않는다 (Phase 0).
```

## 6. Phase 0 / 이후

| 층 | 내용 |
|----|------|
| Phase 0 (지금) | 이 문서 + 실측 표 |
| 하지 않음 | cgc Result 강제, AFJ 계약 이식, runtime 실패 의미 통일 |
| AFJ | Result·match 강제 실험은 **AFJ 쪽은 따로** |
