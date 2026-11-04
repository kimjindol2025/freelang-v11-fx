# V-22 node 의존 분류

## 범위와 분류 결과

V-21 표의 293개 글자 검색 행을 생성 당시의 검색 결과 TSV로 읽고 Bash `while IFS= read -r` 방식으로 분류했다. 범위는 V-21과 동일하게 FX 저장소와 별도 확인을 위해 포함한 AFJ DB95의 `scripts/`, `tests/`다. 한 행은 다섯 종류 중 하나에만 배정했다.

| 종류 | 건수 | 기준 |
|---|---:|---|
| 실행 경로 | 78 | 셸 스크립트에서 node/npm/npx를 실제 명령으로 호출 |
| 시험 입력 | 4 | `.js` 시험 파일 내부에서 검색된 행 |
| 빌드 설정 | 0 | Makefile 또는 package.json 설정 파일의 검색 행 |
| 문서·주석 | 27 | Markdown/text 문서 또는 주석 문법으로 시작하는 행 |
| 기타 | 184 | 위 네 종류에 포함되지 않은 코드·문자열 hit |
| 합계 | 293 | 78 + 4 + 0 + 27 + 184 |

## 실행·빌드 의존 목록

실행·빌드 의존 행은 아래 표에 모두 적었다. 실행 행은 셸이 실제 node 명령을 호출하는 경우이며, 실행된 적이 있는지까지 이번 분류만으로 단정하지 않는다. 대체 방법은 제안이며 이번 작업에서는 파일을 바꾸지 않았다. AFJ DB95 경로는 별도 저장소를 뜻한다.

| 파일:줄 | 원문 | 대체 방법 | 난이도 |
|---|---|---|---|
| `AFJ_DB95/tests/fault-injection-restart.sh:24` | `node "$bootstrap" run "$repo_root/tests/fault-injection-restart.fls" >"$log_file" 2>&1 &` | FreeLang | 중 |
| `AFJ_DB95/tests/fault-injection-restart.sh:50` | `if timeout 20s node "$bootstrap" run "$repo_root/tests/fault-injection-restart.fls" \` | FreeLang | 중 |
| `AFJ_DB95/tests/mariadb-afj-differential-runner.sh:52` | `node "$bootstrap" run "$project/tests/tcp-daemon.fls" >"$afj_log" 2>&1 &` | FreeLang | 중 |
| `AFJ_DB95/tests/mariadb-afj-differential-runner.sh:59` | `timeout 30s node "$bootstrap" run "$client" >"$afj_result" 2>&1` | FreeLang | 중 |
| `AFJ_DB95/tests/mariadb-afj-differential-runner.sh:61` | `node "$bootstrap" run "$project/src/engine.fls" >"$error_result" 2>&1` | FreeLang | 중 |
| `AFJ_DB95/tests/mariadb-afj-differential-runner.sh:63` | `node "$bootstrap" run "$project/src/transaction-engine.fls" >"$isolation_result" 2>&1` | FreeLang | 중 |
| `AFJ_DB95/tests/production-tls-boundary.sh:17` | `  node "$bootstrap" run "$repo_root/src/tcp-server.fls" >"$log_file" 2>&1; then` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:100` | `run_case "lock timeout" env AFJ_DB_LOCK_WAIT_TIMEOUT_MS=1000 node "$bootstrap" run "$repo_root/tests/tcp-lock-timeout.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:101` | `run_case "prepared JSON adapter" node "$bootstrap" run "$repo_root/tests/tcp-prepared.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:102` | `run_case "request limits" node "$bootstrap" run "$repo_root/tests/request-limits.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:103` | `run_case "slow request timeout" env AFJ_DB_REQUEST_TIMEOUT_MS=100 node "$bootstrap" run "$repo_root/tests/tcp-slow-timeout.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:104` | `run_case "auth lockout" node "$bootstrap" run "$repo_root/tests/auth-lockout.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:105` | `run_case "auth lockout reset" env AFJ_DB_AUTH_LOCKOUT_MS=1000 node "$bootstrap" run "$repo_root/tests/auth-lockout-reset.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:106` | `run_case "auth state recovery" env AFJ_DB_AUTH_STATE=/tmp/afj-db95-auth-state-test.jsonl node "$bootstrap" run "$repo_root/tests/auth-state-recovery.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:107` | `run_case "session cleanup" node "$bootstrap" run "$repo_root/tests/session-cleanup.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:108` | `run_case "session TTL" env AFJ_DB_SESSION_TTL_MS=1000 node "$bootstrap" run "$repo_root/tests/session-ttl.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:109` | `run_case "connection rate limit" env AFJ_DB_RATE_MAX_REQUESTS=2 node "$bootstrap" run "$repo_root/tests/connection-rate-limit.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:110` | `run_case "peer IP rate limit" env AFJ_DB_RATE_MAX_REQUESTS=2 node "$bootstrap" run "$repo_root/tests/ip-rate-limit.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:111` | `run_case "runtime peer IP rate limit" node "$repo_root/tests/ip-rate-limit-runtime.js"` | FreeLang | 상 |
| `AFJ_DB95/tests/release-gate.sh:112` | `run_case "fragmented TCP requests" node "$bootstrap" run "$repo_root/tests/tcp-fragmented-requests.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:113` | `run_case "SQL error mapping" node "$bootstrap" run "$repo_root/tests/error-mapping.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:115` | `run_case "native TLS listener" node "$repo_root/tests/native-tls-smoke.js"` | FreeLang | 상 |
| `AFJ_DB95/tests/release-gate.sh:116` | `run_case "native TLS mTLS/reload" node "$repo_root/tests/native-tls-policy-smoke.js"` | FreeLang | 상 |
| `AFJ_DB95/tests/release-gate.sh:117` | `run_case "production native TLS integration" node "$repo_root/tests/production-native-tls.js"` | FreeLang | 상 |
| `AFJ_DB95/tests/release-gate.sh:118` | `run_case "TLS adapter handshake/reload" node "$repo_root/tests/tls-proxy-smoke.js"` | FreeLang | 상 |
| `AFJ_DB95/tests/release-gate.sh:119` | `run_case "MariaDB wire packet unit" node "$repo_root/tests/mariadb-wire-unit.js"` | FreeLang | 상 |
| `AFJ_DB95/tests/release-gate.sh:120` | `run_case "MariaDB mysql_native_password SHA-1" node "$bootstrap" run "$repo_root/tests/mariadb-sha1.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:121` | `run_case "MariaDB wire prepared" node "$repo_root/tests/mariadb-wire-prepared.js"` | FreeLang | 상 |
| `AFJ_DB95/tests/release-gate.sh:122` | `run_case "MariaDB standard-client smoke" node "$repo_root/tests/mariadb-wire-smoke.js"` | FreeLang | 상 |
| `AFJ_DB95/tests/release-gate.sh:123` | `run_case "native MariaDB wire" node "$repo_root/tests/native-mariadb-smoke.js"` | FreeLang | 상 |
| `AFJ_DB95/tests/release-gate.sh:124` | `run_case "native MariaDB auth rejection" node "$repo_root/tests/native-mariadb-auth.js"` | FreeLang | 상 |
| `AFJ_DB95/tests/release-gate.sh:125` | `run_case "native MariaDB decoder" node "$bootstrap" run "$repo_root/tests/native-mariadb-decoder.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:126` | `run_case "native MariaDB prepared" node "$repo_root/tests/native-mariadb-prepared.js"` | FreeLang | 상 |
| `AFJ_DB95/tests/release-gate.sh:56` | `run_case "syntax: tcp-server" node "$bootstrap" check "$repo_root/src/tcp-server.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:57` | `run_case "syntax: afj-db95" node "$bootstrap" check "$repo_root/src/afj-db95.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:58` | `run_case "raw binary boundary" node "$bootstrap" run "$repo_root/tests/binary-raw.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:59` | `run_case "MariaDB packet codec" node "$bootstrap" run "$repo_root/tests/mariadb-wire-codec.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:60` | `run_case "MariaDB charset contract" node "$bootstrap" run "$repo_root/tests/mariadb-charset-contract.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:61` | `run_case "COLLATE contract" node "$bootstrap" run "$repo_root/tests/collation-contract.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:62` | `run_case "page/WAL smoke" env AFJ_DB_PAGE_CACHE_PAGES=2 node "$bootstrap" run "$repo_root/src/afj-db95.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:63` | `run_case "metrics contract" node "$bootstrap" run "$repo_root/tests/metrics-contract.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:65` | `run_case "large corpus" node "$bootstrap" run "$repo_root/tests/large-corpus-soak.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:66` | `run_case "TCP catalog pages" env AFJ_DB_WAL=/tmp/afj-db95-catalog-page-gate.wal AFJ_DB_CATALOG_PAGES=/tmp/afj-db95-catalog-page-gate.pages AFJ_DB_PAGE_POOL_PAGES=2 node "$bootstrap" run "$repo_root/tests/tcp-catalog-pages.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:67` | `run_case "TCP lazy catalog eviction" env AFJ_DB_WAL=/tmp/afj-db95-lazy-catalog-gate.wal AFJ_DB_CATALOG_PAGES=/tmp/afj-db95-lazy-catalog-gate.pages AFJ_DB_LAZY_TABLES=true AFJ_DB_LAZY_TABLE_CACHE=1 node "$bootstrap" run "$repo_root/tests/tcp-lazy-catalog.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:68` | `run_case "persistent recursive B+Tree" node "$bootstrap" run "$repo_root/tests/persistent-btree.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:69` | `run_case "bounded B+Tree range" node "$bootstrap" run "$repo_root/tests/btree-range.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:70` | `run_case "compound unique index" node "$bootstrap" run "$repo_root/tests/compound-index.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:71` | `run_case "compound predicate range" node "$bootstrap" run "$repo_root/tests/compound-range.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:72` | `run_case "schema MVCC merge" node "$bootstrap" run "$repo_root/tests/tcp-schema-mvcc.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:73` | `run_case "composite type order" node "$bootstrap" run "$repo_root/tests/composite-type-order.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:74` | `run_case "composite NULL order" node "$bootstrap" run "$repo_root/tests/composite-null-order.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:75` | `run_case "schema-aware composite order" node "$bootstrap" run "$repo_root/tests/composite-schema-order.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:76` | `run_case "mixed declared composite types" node "$bootstrap" run "$repo_root/tests/composite-mixed-type-order.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:77` | `run_case "composite reopen rebuild" node "$bootstrap" run "$repo_root/tests/composite-reopen-rebuild.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:78` | `run_case "schema-aware composite range" node "$bootstrap" run "$repo_root/tests/composite-schema-range.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:79` | `run_case "TCP concurrency" node "$repo_root/tests/tcp-concurrency-smoke.js"` | FreeLang | 상 |
| `AFJ_DB95/tests/release-gate.sh:80` | `run_case "TCP 5m performance envelope" env AFJ_SOAK_MS=300000 AFJ_SOAK_P95_LATENCY_MS=15000 AFJ_SOAK_MAX_LATENCY_MS=20000 node "$repo_root/tests/tcp-long-soak.js"` | FreeLang | 상 |
| `AFJ_DB95/tests/release-gate.sh:81` | `run_case "backup smoke" node "$bootstrap" run "$repo_root/src/backup.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:82` | `run_case "backup restore soak" node "$bootstrap" run "$repo_root/tests/backup-soak.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:83` | `run_case "compatibility scorecard" node "$bootstrap" run "$repo_root/src/compatibility.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:84` | `run_case "AFJ error token mapping" node "$bootstrap" run "$repo_root/tests/error-mapping.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:85` | `run_case "MariaDB remediation regression" node "$bootstrap" run "$repo_root/tests/mariadb-remediation-regression.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:86` | `run_case "MariaDB unsupported contract" node "$bootstrap" run "$repo_root/tests/mariadb-unsupported-contract.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:87` | `run_case "TCP database session scope" node "$bootstrap" run "$repo_root/tests/tcp-database-scope.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:88` | `run_case "WAL format" node "$bootstrap" run "$repo_root/tests/wal-format.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:90` | `run_case "TCP transaction forced-kill recovery" node "$repo_root/tests/tcp-transaction-kill-recovery.js"` | FreeLang | 상 |
| `AFJ_DB95/tests/release-gate.sh:91` | `run_case "transaction isolation" node "$bootstrap" run "$repo_root/tests/transaction-isolation.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:92` | `run_case "isolation levels" node "$bootstrap" run "$repo_root/tests/tcp-isolation-levels.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:93` | `run_case "MVCC conflicts" node "$bootstrap" run "$repo_root/tests/tcp-mvcc-conflicts.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:94` | `run_case "range predicate conflict" node "$bootstrap" run "$repo_root/tests/tcp-range-predicate.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:95` | `run_case "row-version MVCC core" node "$bootstrap" run "$repo_root/tests/mvcc-row-version.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:96` | `run_case "row-version WAL recovery" env AFJ_DB_WAL=/tmp/afj-db95-row-version-recovery.wal node "$bootstrap" run "$repo_root/tests/tcp-row-version-recovery.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:97` | `run_case "row-version catalog checkpoint" env AFJ_DB_WAL=/tmp/afj-db95-row-version-checkpoint.wal AFJ_DB_CATALOG_PAGES=/tmp/afj-db95-row-version-checkpoint.pages node "$bootstrap" run "$repo_root/tests/tcp-row-version-checkpoint.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:98` | `run_case "savepoint" node "$bootstrap" run "$repo_root/tests/tcp-savepoint.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/release-gate.sh:99` | `run_case "deadlock" node "$bootstrap" run "$repo_root/tests/tcp-deadlock.fls"` | FreeLang | 중 |
| `AFJ_DB95/tests/silent-fix-three-pass.sh:14` | `    node "$bootstrap" run tests/silent-batch-index.fls` | FreeLang | 중 |
| `AFJ_DB95/tests/silent-fix-three-pass.sh:16` | `    node "$bootstrap" run tests/silent-wal-recovery.fls` | FreeLang | 중 |
| `AFJ_DB95/tests/silent-fix-three-pass.sh:19` | `    node "$bootstrap" run tests/silent-tcp-wal-recovery.fls` | FreeLang | 중 |

## 옵션 확인

`scripts/verify-node-free.sh --exclude-docs`는 기본 탐지 결과를 유지하면서 Bash 분류 결과를 추가로 출력한다. 옵션 실행에서 `CLASS_EXECUTION=78`, `CLASS_INPUT=4`, `CLASS_BUILD=0`, `CLASS_DOCS=27`, `CLASS_OTHER=184`, `CLASS_SUM=293`, `NODE_REAL=78`이 나왔다. 여기서 `NODE_REAL`은 실행 경로와 빌드 설정 행의 합이다. 기본 모드는 `NODE_FREE=293`을 계속 출력하고 표의 SHA-256도 기존과 같은 `fc9d69416b0e1ba535a177efec595a78c364a59550000f985fed35a6f2582a3a`였다. PATH 제한 단위 시험은 `10 PASS / 0 FAIL / 10 총`, RC `0`이었다.

V-21 표에 포함된 AFJ DB95 쪽 node 실행 호출이 78건으로 분류됐다. FX 저장소와 AFJ DB95를 합친 이번 검색 범위의 실질 실행 의존 후보 수로 보고하며, 실사용 빈도나 제거 우선순위는 별도 조사가 필요하다.

V22_SUMMARY=3/3, NODE_REAL=78
