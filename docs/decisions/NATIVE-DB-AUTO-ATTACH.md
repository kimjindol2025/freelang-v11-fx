# Native DB Auto-Attach

상태: 구현 중인 1차 계약 (2026-10-07)

## 공개 호출

```lisp
(define db (db "app"))
(db-exec db "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT)" [])
(db-query db "SELECT * FROM users WHERE id = ?" [1])
(db-transaction db [{"sql" "INSERT INTO users (id, name) VALUES (?, ?)" "params" [1 "Kim"]}])
```

`db`는 프로젝트 로컬 SQLite provider를 논리 이름으로 연다. 기본 경로는
`./.freelang/db/<name>.db`이며 `FL_DB_DIR`로 프로젝트 기준 상대 디렉터리를
명시할 수 있다. 동일 경로는 기존 native SQLite connection pool/cache에서
재사용한다.

## 보안 경계

- 이름은 ASCII 영숫자, `_`, `-`만 허용하며 길이는 1–96자다.
- 절대 경로와 `..`가 들어간 `FL_DB_DIR`는 거부한다.
- 자동 연결은 네트워크·MariaDB·외부 프로세스를 사용하지 않는다.
- 원격 provider는 별도 명시 API와 capability 계약이 필요하다.
- 실패 표현은 두 런타임에서 동일한 `[DB_*]` 오류 문자열을 사용한다.

## 런타임 소유권

| 경로 | 구현 |
|---|---|
| FX native | `runtime/sqlite.c`의 `db(FLValue)` resolver + 기존 SQLite pool |
| v11 interpreter | `bootstrap.js`의 MariaDB/SQLite module resolver |
| FreeLangScript | v11 evaluator를 통해 동일 builtin 상속 |
| AFJ-DB95 | SQL·WAL·복구·호환성 검증 기준선; 자동 연결 provider 아님 |

## 현재 증거

- `tests/db-auto.fl`: 두 런타임에서 생성·쓰기·조회·핸들 재사용 PASS
- `tests/db-auto-edge.fl`: traversal/공백 이름 차단 PASS
- `tests/db-auto-tx.fl`: 두 런타임에서 COMMIT/ROLLBACK 일치 PASS
- `tests/db-auto-reopen.fl`: 프로세스 재시작 후 persisted row 재조회 PASS
- `verify-fixpoint.sh`: `db`/`db-transaction` compiler mapping 포함 3세대 hash 일치 PASS
- 전체 상용 DB gate의 PASS는 이 언어 통합의 PASS로 대체하지 않는다.

## 남은 작업

1. FX native 호출 경로에서 WAL·복구·동시 접근을 회귀 검증
2. 적대적 입력·경로·권한 검수 후 버전 판정

FreeLangScript conformance는 `freelang-script`의 정식 allowlist에 추가했고,
CRUD·트랜잭션·권한 범위·위조 handle·재시작 persistence·동시 writer를 실제
검증했다. 이 결과는 FX native 경로의 상용 release gate PASS를 의미하지 않는다.
