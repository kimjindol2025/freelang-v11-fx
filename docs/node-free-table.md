# Node-free 의존성 탐지표

검색 대상: `scripts/`, `tests/`, `self/`, `runtime/`, `Makefile`, AFJ DB95의 `scripts/`, `tests/`. 글자 검색어: `node `, `npm`, `npx`, `.js`, `.ts`, `fl-cli`, `package.json`.

AFJ native-release-gate `.js` 분기 탐지: 1; AFJ `.js` run_case 수: 14.

| 어디서 | 무엇을 | 종류 | 대체 방법 제안 |
|---|---|---|---|
| `AFJ_DB95/tests/auth-state-recovery.fls:2` | literal '.js': (define state-path "/tmp/afj-db95-auth-state-test.jsonl") | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/bootstrap-path.sh:6` | literal '.js':     candidate="$FREELANG_AFJ_ROOT/bootstrap.js" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/mariadb-afj-differential-runner.sh:15` | literal '.ts': maria_result="/tmp/afj-db95-differential-mariadb.tsv" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/mariadb-afj-differential-runner.sh:52` | literal 'node ': node "$bootstrap" run "$project/tests/tcp-daemon.fls" >"$afj_log" 2>&1 & | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/mariadb-afj-differential-runner.sh:59` | literal 'node ': timeout 30s node "$bootstrap" run "$client" >"$afj_result" 2>&1 | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/mariadb-afj-differential-runner.sh:61` | literal 'node ': node "$bootstrap" run "$project/src/engine.fls" >"$error_result" 2>&1 | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/mariadb-afj-differential-runner.sh:63` | literal 'node ': node "$bootstrap" run "$project/src/transaction-engine.fls" >"$isolation_result" 2>&1 | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/mariadb-recovery-reader.fls:5` | literal '.js': (let [expected (json-parse (file_read (str afj-tcp-wal ".expected.json")))] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/mariadb-remediation-regression.fls:128` | literal '.js': (file_write (str afj-tcp-wal ".expected.json") | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/native-release-gate.sh:101` | literal 'node ': if PATH="$safe_path" command -v node >/dev/null 2>&1 \\|\\| | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/native-release-gate.sh:119` | literal 'node ':     env_text="${env_text%% node *}" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/native-release-gate.sh:27` | literal '.js':   export AFJ_DB_AUTH_STATE="$case_dir/auth.jsonl" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/native-release-gate.sh:28` | literal '.js':   export AFJ_DB_AUDIT_LOG="$case_dir/audit.jsonl" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/native-release-gate.sh:34` | literal '.js':   if [[ "$rel_path" == "tests/tcp-long-soak.js" && -f "$repo_root/tests/tcp-long-soak.fls" ]]; then | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/native-release-gate.sh:38` | literal '.js':     *.js) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/production-native-tls.js:78` | literal '.js':   const audit = path.join(directory, "audit.jsonl"); | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/production-native-tls.js:79` | literal '.js':   const authState = path.join(directory, "auth-state.jsonl"); | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/production-tls-boundary.sh:14` | literal '.js':   AFJ_DB_AUDIT_LOG="$runtime_dir/production-tls-boundary.audit.jsonl" \ | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/production-tls-boundary.sh:15` | literal '.js':   AFJ_DB_AUTH_STATE="$runtime_dir/production-tls-boundary.state.jsonl" \ | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:100` | literal 'node ': run_case "lock timeout" env AFJ_DB_LOCK_WAIT_TIMEOUT_MS=1000 node "$bootstrap" run "$repo_root/tests/tcp-lock-timeout.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:101` | literal 'node ': run_case "prepared JSON adapter" node "$bootstrap" run "$repo_root/tests/tcp-prepared.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:102` | literal 'node ': run_case "request limits" node "$bootstrap" run "$repo_root/tests/request-limits.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:103` | literal 'node ': run_case "slow request timeout" env AFJ_DB_REQUEST_TIMEOUT_MS=100 node "$bootstrap" run "$repo_root/tests/tcp-slow-timeout.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:104` | literal 'node ': run_case "auth lockout" node "$bootstrap" run "$repo_root/tests/auth-lockout.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:105` | literal 'node ': run_case "auth lockout reset" env AFJ_DB_AUTH_LOCKOUT_MS=1000 node "$bootstrap" run "$repo_root/tests/auth-lockout-reset.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:106` | literal 'node ': run_case "auth state recovery" env AFJ_DB_AUTH_STATE=/tmp/afj-db95-auth-state-test.jsonl node "$bootstrap" run "$repo_root/tests/auth-state-recovery.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:107` | literal 'node ': run_case "session cleanup" node "$bootstrap" run "$repo_root/tests/session-cleanup.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:108` | literal 'node ': run_case "session TTL" env AFJ_DB_SESSION_TTL_MS=1000 node "$bootstrap" run "$repo_root/tests/session-ttl.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:109` | literal 'node ': run_case "connection rate limit" env AFJ_DB_RATE_MAX_REQUESTS=2 node "$bootstrap" run "$repo_root/tests/connection-rate-limit.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:110` | literal 'node ': run_case "peer IP rate limit" env AFJ_DB_RATE_MAX_REQUESTS=2 node "$bootstrap" run "$repo_root/tests/ip-rate-limit.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:111` | literal 'node ': run_case "runtime peer IP rate limit" node "$repo_root/tests/ip-rate-limit-runtime.js" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:112` | literal 'node ': run_case "fragmented TCP requests" node "$bootstrap" run "$repo_root/tests/tcp-fragmented-requests.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:113` | literal 'node ': run_case "SQL error mapping" node "$bootstrap" run "$repo_root/tests/error-mapping.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:115` | literal 'node ': run_case "native TLS listener" node "$repo_root/tests/native-tls-smoke.js" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:116` | literal 'node ': run_case "native TLS mTLS/reload" node "$repo_root/tests/native-tls-policy-smoke.js" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:117` | literal 'node ': run_case "production native TLS integration" node "$repo_root/tests/production-native-tls.js" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:118` | literal 'node ': run_case "TLS adapter handshake/reload" node "$repo_root/tests/tls-proxy-smoke.js" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:119` | literal 'node ': run_case "MariaDB wire packet unit" node "$repo_root/tests/mariadb-wire-unit.js" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:120` | literal 'node ': run_case "MariaDB mysql_native_password SHA-1" node "$bootstrap" run "$repo_root/tests/mariadb-sha1.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:121` | literal 'node ': run_case "MariaDB wire prepared" node "$repo_root/tests/mariadb-wire-prepared.js" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:122` | literal 'node ': run_case "MariaDB standard-client smoke" node "$repo_root/tests/mariadb-wire-smoke.js" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:123` | literal 'node ': run_case "native MariaDB wire" node "$repo_root/tests/native-mariadb-smoke.js" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:124` | literal 'node ': run_case "native MariaDB auth rejection" node "$repo_root/tests/native-mariadb-auth.js" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:125` | literal 'node ': run_case "native MariaDB decoder" node "$bootstrap" run "$repo_root/tests/native-mariadb-decoder.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:126` | literal 'node ': run_case "native MariaDB prepared" node "$repo_root/tests/native-mariadb-prepared.js" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:35` | literal '.js':      AFJ_DB_AUTH_STATE="$temp_dir/case-$case_number.auth.jsonl" \ | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:36` | literal '.js':      AFJ_DB_AUDIT_LOG="$temp_dir/case-$case_number.audit.jsonl" \ | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:56` | literal 'node ': run_case "syntax: tcp-server" node "$bootstrap" check "$repo_root/src/tcp-server.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:57` | literal 'node ': run_case "syntax: afj-db95" node "$bootstrap" check "$repo_root/src/afj-db95.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:58` | literal 'node ': run_case "raw binary boundary" node "$bootstrap" run "$repo_root/tests/binary-raw.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:59` | literal 'node ': run_case "MariaDB packet codec" node "$bootstrap" run "$repo_root/tests/mariadb-wire-codec.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:60` | literal 'node ': run_case "MariaDB charset contract" node "$bootstrap" run "$repo_root/tests/mariadb-charset-contract.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:61` | literal 'node ': run_case "COLLATE contract" node "$bootstrap" run "$repo_root/tests/collation-contract.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:62` | literal 'node ': run_case "page/WAL smoke" env AFJ_DB_PAGE_CACHE_PAGES=2 node "$bootstrap" run "$repo_root/src/afj-db95.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:63` | literal 'node ': run_case "metrics contract" node "$bootstrap" run "$repo_root/tests/metrics-contract.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:65` | literal 'node ': run_case "large corpus" node "$bootstrap" run "$repo_root/tests/large-corpus-soak.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:66` | literal 'node ': run_case "TCP catalog pages" env AFJ_DB_WAL=/tmp/afj-db95-catalog-page-gate.wal AFJ_DB_CATALOG_PAGES=/tmp/afj-db95-catalog-page-gate.pages AFJ_DB_PAGE_POOL_PAGES=2 node "$bootstrap" run "$repo_root/tests/tcp-catalog-pages.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:67` | literal 'node ': run_case "TCP lazy catalog eviction" env AFJ_DB_WAL=/tmp/afj-db95-lazy-catalog-gate.wal AFJ_DB_CATALOG_PAGES=/tmp/afj-db95-lazy-catalog-gate.pages AFJ_DB_LAZY_TABLES=true AFJ_DB_LAZY_TABLE_CACHE=1 node "$bootstrap" run "$repo_root/tests/tcp-lazy-catalog.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:68` | literal 'node ': run_case "persistent recursive B+Tree" node "$bootstrap" run "$repo_root/tests/persistent-btree.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:69` | literal 'node ': run_case "bounded B+Tree range" node "$bootstrap" run "$repo_root/tests/btree-range.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:70` | literal 'node ': run_case "compound unique index" node "$bootstrap" run "$repo_root/tests/compound-index.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:71` | literal 'node ': run_case "compound predicate range" node "$bootstrap" run "$repo_root/tests/compound-range.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:72` | literal 'node ': run_case "schema MVCC merge" node "$bootstrap" run "$repo_root/tests/tcp-schema-mvcc.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:73` | literal 'node ': run_case "composite type order" node "$bootstrap" run "$repo_root/tests/composite-type-order.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:74` | literal 'node ': run_case "composite NULL order" node "$bootstrap" run "$repo_root/tests/composite-null-order.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:75` | literal 'node ': run_case "schema-aware composite order" node "$bootstrap" run "$repo_root/tests/composite-schema-order.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:76` | literal 'node ': run_case "mixed declared composite types" node "$bootstrap" run "$repo_root/tests/composite-mixed-type-order.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:77` | literal 'node ': run_case "composite reopen rebuild" node "$bootstrap" run "$repo_root/tests/composite-reopen-rebuild.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:78` | literal 'node ': run_case "schema-aware composite range" node "$bootstrap" run "$repo_root/tests/composite-schema-range.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:79` | literal 'node ': run_case "TCP concurrency" node "$repo_root/tests/tcp-concurrency-smoke.js" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:80` | literal 'node ': run_case "TCP 5m performance envelope" env AFJ_SOAK_MS=300000 AFJ_SOAK_P95_LATENCY_MS=15000 AFJ_SOAK_MAX_LATENCY_MS=20000 node "$repo_root/tests/tcp-long-soak.js" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:81` | literal 'node ': run_case "backup smoke" node "$bootstrap" run "$repo_root/src/backup.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:82` | literal 'node ': run_case "backup restore soak" node "$bootstrap" run "$repo_root/tests/backup-soak.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:83` | literal 'node ': run_case "compatibility scorecard" node "$bootstrap" run "$repo_root/src/compatibility.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:84` | literal 'node ': run_case "AFJ error token mapping" node "$bootstrap" run "$repo_root/tests/error-mapping.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:85` | literal 'node ': run_case "MariaDB remediation regression" node "$bootstrap" run "$repo_root/tests/mariadb-remediation-regression.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:86` | literal 'node ': run_case "MariaDB unsupported contract" node "$bootstrap" run "$repo_root/tests/mariadb-unsupported-contract.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:87` | literal 'node ': run_case "TCP database session scope" node "$bootstrap" run "$repo_root/tests/tcp-database-scope.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:88` | literal 'node ': run_case "WAL format" node "$bootstrap" run "$repo_root/tests/wal-format.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:90` | literal 'node ': run_case "TCP transaction forced-kill recovery" node "$repo_root/tests/tcp-transaction-kill-recovery.js" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:91` | literal 'node ': run_case "transaction isolation" node "$bootstrap" run "$repo_root/tests/transaction-isolation.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:92` | literal 'node ': run_case "isolation levels" node "$bootstrap" run "$repo_root/tests/tcp-isolation-levels.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:93` | literal 'node ': run_case "MVCC conflicts" node "$bootstrap" run "$repo_root/tests/tcp-mvcc-conflicts.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:94` | literal 'node ': run_case "range predicate conflict" node "$bootstrap" run "$repo_root/tests/tcp-range-predicate.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:95` | literal 'node ': run_case "row-version MVCC core" node "$bootstrap" run "$repo_root/tests/mvcc-row-version.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:96` | literal 'node ': run_case "row-version WAL recovery" env AFJ_DB_WAL=/tmp/afj-db95-row-version-recovery.wal node "$bootstrap" run "$repo_root/tests/tcp-row-version-recovery.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:97` | literal 'node ': run_case "row-version catalog checkpoint" env AFJ_DB_WAL=/tmp/afj-db95-row-version-checkpoint.wal AFJ_DB_CATALOG_PAGES=/tmp/afj-db95-row-version-checkpoint.pages node "$bootstrap" run "$repo_root/tests/tcp-row-version-checkpoint.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:98` | literal 'node ': run_case "savepoint" node "$bootstrap" run "$repo_root/tests/tcp-savepoint.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/release-gate.sh:99` | literal 'node ': run_case "deadlock" node "$bootstrap" run "$repo_root/tests/tcp-deadlock.fls" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/runtime-path.js:13` | literal '.js':       "(예: AFJ_BOOTSTRAP=/path/to/freelang-afj/bootstrap.js)" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/runtime-path.js:9` | literal '.js':       path.join(process.env.FREELANG_AFJ_ROOT, "bootstrap.js")); | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/silent-fix-three-pass.sh:14` | literal 'node ':     node "$bootstrap" run tests/silent-batch-index.fls | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/silent-fix-three-pass.sh:16` | literal 'node ':     node "$bootstrap" run tests/silent-wal-recovery.fls | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/silent-fix-three-pass.sh:19` | literal 'node ':     node "$bootstrap" run tests/silent-tcp-wal-recovery.fls | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `AFJ_DB95/tests/tcp-long-soak.fls:1` | literal '.js': ;; FreeLang port of tests/tcp-long-soak.js. Keep the JS checks and limits 1:1. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `install.sh` | 파일 없음; 실행하지 않음 | 실행 | 설치 진입점을 저장소 문서에 명시 |
| `runtime/browser-target.h:234` | literal 'node ':  if(user.tag!=FL_VECTOR)bj_fail(&b,"program must be a node vector"); | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `runtime/gc.c:291` | literal 'node ':     PermNode* node = (PermNode*)malloc(sizeof(PermNode)); | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `runtime/http-form.h:332` | literal 'node ':          * collect an inode between its creation and acquisition of flock. */ | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `runtime/v11-http.h:39` | literal '.js':  static const char*types[][2]={{".png","image/png"},{".jpg","image/jpeg"},{".jpeg","image/jpeg"},{".gif","image/gif"},{".svg","image/svg+xml"},{".html","text/html; charset=utf-8"},{".htm","text/html; charset=utf-8"},{".css","text/css; charset=utf-8"},{".js","application/javascript; charset=utf-8"},{".mjs","application/javascript; charset=utf-8"},{".json","application/json; charset=utf-8"},{".ico","image/x-icon"},{".woff","font/woff"},{".woff2","font/woff2"},{".ttf","font/ttf"},{".txt","text/plain; charset=utf-8"},{".pdf","application/pdf"}}; | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/audit-l07-execve.sh:13` | literal 'npm':   printf "NODE=%d NPM=%d NPX=%d JS=%d PYTHON=%d\n",node,npm,npx,js,python | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/audit-l07-execve.sh:14` | literal 'npm':   if (node+npm+npx+js+python) exit 1 | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/audit-l07-execve.sh:4` | literal '.js': # Inspect the executed program, not regex arguments mentioning bootstrap.js. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/audit-l07-execve.sh:8` | literal 'npm':   if (p == "npm") npm++ | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/audit-l07-execve.sh:9` | literal 'npx':   if (p == "npx") npx++ | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l01-browser-target.sh:33` | literal '.js': "$compiler" --browser-js "$out/source/$entry" "$out/root/program.js" > "$out/generate.log" 2>&1 | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l01-browser-target.sh:35` | literal '.js': "$compiler" --browser-js "$out/source/$entry" "$out/again.js" > "$out/generate-again.log" 2>&1 | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l01-browser-target.sh:36` | literal '.js': cmp "$out/root/program.js" "$out/again.js" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l01-browser-target.sh:38` | literal '.js': sha256sum "$out/root/program.js" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l01-browser-target.sh:42` | literal '.js':  if "$compiler" --browser-js "$file" "$out/reject-$spec.js" > "$out/reject-$spec.actual" 2> "$out/reject-$spec.stderr";then rc=0;else rc=$?;fi | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l01-browser-target.sh:45` | literal '.js':  if [[ "$rc" == 1 && ! -e "$out/reject-$spec.js" && "$before" == "$(sha256sum "$file" \\| cut -d' ' -f1)" ]];then passed=$((passed+1));else failed=$((failed+1));fi | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l01-browser-target.sh:50` | literal '.js':  [[ "$spec" != same-source ]] \\|\\| { output="$out/alias.js";ln -s "$input" "$output"; } | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-blog-backup-integrity.sh:45` | literal '.js': manifest_hash=$(sha256sum "$backup/manifest.json" \\| cut -d' ' -f1) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-blog-backup-integrity.sh:50` | literal '.js':  manifest="$root/blog-backups/integrity-proof/manifest.json" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-blog-backup-integrity.sh:65` | literal '.js':  if [[ -n "$expression" ]];then jq "$expression" "$manifest" > "$root/changed-manifest.json";mv "$root/changed-manifest.json" "$manifest";fi | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-blog-backup-integrity.sh:79` | literal '.js': if [[ "$manifest_hash" == "$(sha256sum "$backup/manifest.json" \\| cut -d' ' -f1)" && "$original_hash" == "$(sha256sum "$backup/images/$image" \\| cut -d' ' -f1)" ]];then echo BACKUP_INTEGRITY_SOURCE_BACKUP_UNCHANGED=PASS;else echo BACKUP_INTEGRITY_SOURCE_BACKUP_UNCHANGED=FAIL;failed=$((failed+1));fi | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-blog-backup-restore-faults.sh:10` | literal '.js': [[ -f "$blog/app/blog-backup.fl" && ! -e "$blog/data/blog.wal" && -f "$archive/manifest.json" ]] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-blog-backup-restore-faults.sh:30` | literal '.js': manifest_hash=$(sha256sum "$archive/manifest.json" \\| cut -d' ' -f1) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-blog-backup-restore-faults.sh:47` | literal '.js': [[ "$manifest_hash" == "$(sha256sum "$archive/manifest.json" \\| cut -d' ' -f1)" && "$image_hash" == "$(sha256sum "$archive/images/$image" \\| cut -d' ' -f1)" ]] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-blog-image-upload-faults.sh:104` | literal '.js':   status=$(get /__l08_upload_images "$root/rows-$restart.json") | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-blog-image-upload-faults.sh:106` | literal '.js':   if [[ "$expected_rows" == 0 && "$status" == 200 ]] && jq -e 'length == 0' "$root/rows-$restart.json" > /dev/null 2> "$root/rows-parse.stderr";then | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-blog-image-upload-faults.sh:109` | literal '.js':   elif [[ "$expected_rows" == 1 && "$status" == 200 ]] && jq -e --argjson bytes "$image_size" 'length == 1 and .[0].mime == "image/gif" and .[0].size == $bytes and .[0].alt == "시험 한🙂"' "$root/rows-$restart.json" > /dev/null 2> "$root/rows-parse.stderr";then | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-blog-image-upload-faults.sh:110` | literal '.js':    rows=1;path=$(jq -r '.[0].path' "$root/rows-$restart.json") | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-blog-legacy-backup.sh:28` | literal '.js': manifest_hash=$(sha256sum "$source_backup/manifest.json" \\| cut -d' ' -f1) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-blog-legacy-backup.sh:46` | literal '.js': [[ "$manifest_hash" == "$(sha256sum "$source_backup/manifest.json" \\| cut -d' ' -f1)" ]] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-blog-store-http.sh:48` | literal '.js':  curl --noproxy '*' -fsS --max-time 10 "http://127.0.0.1:$port/rows" > "$out/$1.json" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-blog-store-http.sh:49` | literal '.js':  jq -e 'length == 32 and (map(.id)\\|sort) == ([range(1;33)\\|tostring]\\|sort) and all(.title == "한🙂")' "$out/$1.json" > /dev/null | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-blog-store-http.sh:54` | literal '.js': curl --noproxy '*' -fsS --max-time 5 "http://127.0.0.1:$port/stats" > "$out/stats.json" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-blog-store-http.sh:60` | literal '.js':  for field in workers peak active;do printf 'DB_HTTP_%s=%s\n' "${field^^}" "$(jq -r ".$field" "$out/stats.json")";done | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-blog-store-http.sh:61` | literal '.js':  printf 'DB_HTTP_LIVE_ROWS=%s\n' "$(jq length "$out/live.json")" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-blog-store-http.sh:74` | literal '.js':  printf 'DB_HTTP_RESTART_%s_ROWS=%s\n' "$iteration" "$(jq length "$out/restarted-$iteration.json")" >> "$out/result" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-http-atom.sh:54` | literal '.js': curl --noproxy '*' -fsS --max-time 5 "http://127.0.0.1:$port/stats" > "$out/stats.json" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-http-atom.sh:61` | literal '.js':  printf 'HTTP_ATOM_WORKERS=%s\n' "$(jq -r '.workers' "$out/stats.json")" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-http-atom.sh:62` | literal '.js':  printf 'HTTP_ATOM_PEAK=%s\n' "$(jq -r '.peak' "$out/stats.json")" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-http-atom.sh:63` | literal '.js':  printf 'HTTP_ATOM_ACTIVE=%s\n' "$(jq -r '.active' "$out/stats.json")" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-http-atom.sh:64` | literal '.js':  if jq -se 'map(.value)\\|sort == [range(1;1001)]' "$out/requests/"*.body > "$out/exact-values.json";then echo HTTP_ATOM_EXACT_VALUES=PASS;else echo HTTP_ATOM_EXACT_VALUES=FAIL;fi | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-http-atom.sh:79` | literal '.js':   curl --noproxy '*' -fsS --max-time 5 "http://127.0.0.1:$port/lifetime" > "$out/$label.json" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-http-atom.sh:80` | literal '.js':   live=$(jq -r '.live' "$out/$label.json") | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-http-atom.sh:97` | literal '.js':  printf 'HTTP_ATOM_CREATED=%s\n' "$(jq -r '.created' "$out/cleared.json")" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-l08-tcp-self.sh:60` | literal '.js':  jq -e 'group_by(.id) \\| all(.[]; (map(select(.event=="connect"))\\|length)==1 and (map(select(.event=="close"))\\|length)==1 and .[0].event=="connect" and .[-1].event=="close")' "$out/root/$label/events.json" > "$out/$label-event-audit.log" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-native-bootstrap.sh:103` | literal 'npm':   if rg -n 'execve\(.*"[^" ]*/(node\\|npm\\|npx\\|ts-node\\|tsx\\|deno\\|bun\\|cgc-bin\\|bootstrap\.js)(/\\|"\\|$)' "$log" >/dev/null; then | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `scripts/verify-native-bootstrap.sh:107` | literal '.js': if rg -n '/home/kim/kim/platform/freelang-afj\\|freelang-afj-native-parser\\|bootstrap\.js' "$TMP_DIR"/*.execve.log >/dev/null; then | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1041` | literal 'node ':     ;; ((complex-expr) args...) — op-node at args[0], actual args at [1..] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1494` | literal 'node ': (defn cgc-collect-vars [$node $acc] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1497` | literal 'node ':     [(and (= (get $node :kind) "literal") (= (get $node :type) "symbol")) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1498` | literal 'node ':      (let [[$n (c-name (get $node :value))]] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1500` | literal 'node ':     [(= (get $node :kind) "variable") | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1501` | literal 'node ':      (let [[$n (c-name (get $node :name))]] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1503` | literal 'node ':     [(= (get $node :kind) "sexpr") | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1505` | literal 'node ':      (let [[$op (get $node :op)] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1512` | literal 'node ':        (cgc-collect-vars-loop (get $node :args) 0 $acc2))] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1513` | literal 'node ':     [(and (= (get $node :kind) "block") (= (get $node :type) "Map")) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1514` | literal 'node ':      (cgc-collect-map-values (get (get $node :fields) "items") 1 $acc)] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1515` | literal 'node ':     [(= (get $node :kind) "block") | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1516` | literal 'node ':      (cgc-collect-vars-loop (get (get $node :fields) "items") 0 $acc)] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1517` | literal 'node ':     [(= (get $node :kind) "try") | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1518` | literal 'node ':      (cgc-collect-vars (get $node :finally) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1519` | literal 'node ':        (cgc-collect-vars (get $node :catch) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1520` | literal 'node ':          (cgc-collect-vars (get $node :body) $acc)))] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1521` | literal 'node ':     [(= (get $node :kind) "throw") | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1522` | literal 'node ':      (cgc-collect-vars (get $node :expr) $acc)] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1523` | literal 'node ':     [(= (get $node :kind) "and") | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1524` | literal 'node ':      (cgc-collect-vars-loop (get $node :args) 0 $acc)] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1525` | literal 'node ':     [(= (get $node :kind) "or") | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1526` | literal 'node ':      (cgc-collect-vars-loop (get $node :args) 0 $acc)] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1570` | literal 'node ':         [$body-node (make-sexpr "do" (slice $args 1 (length $args)) nil)] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1571` | literal 'node ':         [$all-vars (cgc-collect-vars $body-node [])] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1632` | literal 'node ':   (if (and (= (get $node :kind) "sexpr") (= (get $node "call-user") true)) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1633` | literal 'node ':     (let [[$op (get $node :op)] [$args (get $node :args)] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1636` | literal 'node ':                      (cgc (make-variable (substring $op 1 (length $op)) (get $node :line))))] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1735` | literal 'node ': (defn cgc-sequence-c [$node $code $last] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1760` | literal 'node ':   (if (not (= (get $node :kind) "sexpr")) false | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1761` | literal 'node ':   (let [[$op (get $node :op)] [$args (get $node :args)]] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1779` | literal 'node ': (defn cgc-with-recur-goto [$node $vars $label] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1781` | literal 'node ':   (let [[$k (get $node :kind)]] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1784` | literal 'node ':      (let [[$op (get $node :op)] [$args (get $node :args)]] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1947` | literal 'node ':   (and (= (get $node :kind) "literal") | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1948` | literal 'node ':        (= (get $node :type) "symbol") | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1949` | literal 'node ':        (= (get $node :value) "_"))) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1953` | literal 'node ':   (= (get $node :kind) "variable")) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1957` | literal 'node ':   (and (= (get $node :kind) "block") (= (get $node :type) "Array"))) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1961` | literal 'node ':   (or (and (= (get $node :kind) "literal") (= (get $node :type) "nil")) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1962` | literal 'node ':       (and (= (get $node :kind) "literal") (= (get $node :type) "symbol") | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1963` | literal 'node ':            (or (= (get $node :value) "nil") (= (get $node :value) "null"))))) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:1967` | literal 'node ':   (= (get $node :kind) "variable")) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2038` | literal 'node ':         [$vnode  (get $items 0)] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2039` | literal 'node ':         [$cnode  (get $items 1)] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2055` | literal 'node ':   (let [[$var-node (get $args 0)] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2089` | literal 'node ':         [$var-node (get $items 0)] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2090` | literal 'node ':         [$col-node (get $items 1)] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2411` | literal 'node ': (defn cgc-with-recur [$node $vars] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2413` | literal 'node ':   (let [[$k (get $node :kind)]] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2416` | literal 'node ':      (let [[$op (get $node :op)] [$args (get $node :args)]] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2502` | literal 'node ':   (if (= (get $key-node :kind) "keyword") | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2503` | literal 'node ':     (str "fl_str_val(\"" (c-esc (get $key-node :name)) "\")") | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2529` | literal '.js':     ;; bootstrap.js AST 경로: map-entries로 직접 접근 (json 라운드트립 금지 — 중첩 Map 손상) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2632` | literal 'node ':       (let [[$node (get $nodes $i)]] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2633` | literal 'node ':         (if (and (= (get $node :kind) "sexpr") (= (get $node :op) "load")) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2634` | literal 'node ':           (let [[$path-node (get (get $node :args) 0)] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2635` | literal 'node ':                 [$rel-path  (get $path-node :value)] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2662` | literal 'node ':   (let [[$k (get $node :kind)]] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2665` | literal 'node ':        (str "{\"type\":\"call\",\"fn\":" (ast-json-str (get $node :op)) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2666` | literal 'node ':             ",\"args\":[" (str-join "," (map ast-node-to-json (get $node :args))) "]" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2667` | literal 'node ':             ",\"line\":" (get $node :line) "}")] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2669` | literal 'node ':        (let [[$t (get $node :type)] [$v (get $node :value)]] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2678` | literal 'node ':        (str "{\"type\":\"var\",\"name\":" (ast-json-str (get $node :name)) "}")] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2681` | literal 'node ':             (str-join "," (map ast-node-to-json (get $node :items))) "]}")] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2685` | literal 'node ': (defn ast-tree-lines [$node $prefix $is-last] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2687` | literal 'node ':   (let [[$k   (get $node :kind)] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2692` | literal 'node ':        (let [[$head (str $prefix $conn (get $node :op))] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2693` | literal 'node ':              [$args (get $node :args)] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2707` | literal 'node ':        (let [[$v (get $node :value)] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2708` | literal 'node ':              [$t (get $node :type)]] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2711` | literal 'node ':        [(str $prefix $conn "$" (get $node :name))]] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2714` | literal 'node ':              [$items (get $node :items)] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2739` | literal 'node ':               (append-all $acc (ast-tree-lines $node "" true))) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2807` | literal '.js':     (throw "compiler: --browser-js requires input.fl and output.js") | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2811` | literal '.js':             (if (or (str-ends-with $output ".js") (str-ends-with $output ".mjs")) nil | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:2812` | literal '.js':               (throw "compiler: browser output must end in .js or .mjs"))] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:419` | literal 'node ': ;; node 생성자는 self/ast.fl 정의 재사용. 테스트 단독 실행 위해 여기에도 복제. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:451` | literal 'node ': ;; 2-tuple 반환용 map {:p p' :node n} | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:453` | literal 'node ':   {:p $p :node $node}) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:485` | literal 'node ':   (if (= (get $node :kind) "literal") (get $node :value) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:486` | literal 'node ':   (if (= (get $node :kind) "variable") (str "$" (get $node :name)) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:491` | literal 'node ':   (if (and (= (get $node :kind) "literal") | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:492` | literal 'node ':            (= (get $node :type) "symbol") | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:493` | literal 'node ':            (= (get $node :value) "%")) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:494` | literal 'node ':     (make-variable "__pct__" (get $node :line)) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:495` | literal 'node ':     (if (= (get $node :kind) "sexpr") | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:496` | literal 'node ':       (make-sexpr (get $node :op) (replace-pct-list (get $node :args) 0 []) (get $node :line)) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:576` | literal 'node ':     (let [[$op-node (get $args 0)] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:577` | literal 'node ':           [$op (if (= (get $op-node :kind) "literal") | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:578` | literal 'node ':                  (get $op-node :value) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:579` | literal 'node ':                  (if (= (get $op-node :kind) "variable") | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:580` | literal 'node ':                    (str "$" (get $op-node :name)) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:762` | literal 'node ':   (if (and (= (get $node :kind) "block") (= (get $node :type) "Array")) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:763` | literal 'node ':     (get (get $node :fields) "items") | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:932` | literal 'node ':       [$body-node (get $f :body)] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:950` | literal 'node ':     [(= (get $node :kind) "variable") (c-name (get $node :name))] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:951` | literal 'node ':     [(= (get $node :kind) "literal")  (c-name (str (get $node :value)))] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:959` | literal 'node ':         [$body-node    (get $n :body)] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:960` | literal 'node ':         [$catch-node   (get $n :catch)] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:961` | literal 'node ':         [$finally-node (get $n :finally)] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:963` | literal 'node ':         [$catch-args (if (null? $catch-node) (list) (get $catch-node :args))] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `self/cgc-main.fl:974` | literal 'node ':                       (let [[$fargs (get $finally-node :args)]] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l01/browser-cancel-server.fl:4` | literal '.js': (server_get "/" (fn [req] (server_html "<!doctype html><html lang='ko'><meta charset='utf-8'><title>FreeLang cancel 20</title><body><h1>FreeLang 비동기 취소</h1><pre id='proof'>WAIT</pre><script src='/program.js' defer></script></body></html>"))) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l01/browser-cancel-server.fl:5` | literal '.js': (server_get "/program.js" (fn [req] (server_header (server_text (get (file-read-result "program.js") "data")) "Content-Type" "application/javascript; charset=utf-8"))) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l01/browser-events-server.fl:2` | literal '.js':   (server_html "<!doctype html><html lang='ko'><meta charset='utf-8'><title>FreeLang 입력·이벤트 시험</title><body><h1>FreeLang 입력·이벤트</h1><label for='editor'>본문</label><textarea id='editor'></textarea><button id='commit' type='button'>확인</button><div id='literal'></div><p id='ready'>WAIT</p><pre id='proof'>WAIT</pre><script src='/program.js' defer></script></body></html>"))) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l01/browser-events-server.fl:3` | literal '.js': (server_get "/program.js" (fn [req] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l01/browser-events-server.fl:4` | literal '.js':   (let [result (file-read-result "program.js")] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l01/browser-http-server.fl:2` | literal '.js':   (server_html "<!doctype html><html lang='ko'><meta charset='utf-8'><title>FreeLang HTTP 20</title><body><h1>FreeLang 비동기 HTTP</h1><pre id='proof'>WAIT</pre><script src='/program.js' defer></script></body></html>"))) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l01/browser-http-server.fl:3` | literal '.js': (server_get "/program.js" (fn [req] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l01/browser-http-server.fl:4` | literal '.js':   (server_header (server_text (get (file-read-result "program.js") "data")) "Content-Type" "application/javascript; charset=utf-8"))) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l01/browser-server.fl:2` | literal '.js':   (server_html "<!doctype html><html lang='ko'><meta charset='utf-8'><title>FreeLang browser target</title><body><pre id='proof'>WAIT</pre><script src='/program.js' defer></script></body></html>"))) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l01/browser-server.fl:3` | literal '.js': (server_get "/program.js" (fn [req] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l01/browser-server.fl:4` | literal '.js':   (let [result (file-read-result "program.js")] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l01/editor-required-contract.md:7` | literal '.ts': HTML escape도 앱의 대체 API에 남기지 않고 기존 언어 html-escape를 browser target에 연결한다. v11 \\`b49cc9194a00649b8cf58e1d6e8ee38ffce770d4\\` src/eval-builtins.ts:3010~3013은 nil을 빈 문자열로 바꾸고 String 변환 뒤 &, <, >, quote, apostrophe 순서로 escape한다. apostrophe 정답은 \\`&#x27;\\`다. 기존 native L08 2차 unicode 정답을 보존하며 문자열/기존 entity·nil·0·false·array·map 7개를 먼저 추가해 총 37개로 고정한다. Atom 등 기존 browser str 미지원 타입은 전체 지원으로 주장하지 않는다. Native 코드 경로의 이전 byte API를 기준으로 정답을 바꾸지 않는다. HTML 문자열을 DOM에 실행하는 API는 추가하지 않는다. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-phase2/contract.md:22` | literal '.ts': 후속 전체 회귀에서 산술 타입 오류 정답 3줄의 누락을 확인했다. 기존 표의 ‘정정 2건’은 당시 이력이며 아래 3줄 추가 정정을 포함하지 않는다. v11 \\`b49cc9194a00649b8cf58e1d6e8ee38ffce770d4\\` src/eval-builtins.ts:2000~2008은 \\`[E_TYPE_MISMATCH]\\`로 시작하는 Error를 던지고, src/interpreter.ts:1991~2015는 \\`[+]\\` 또는 \\`[-]\\`를 추가한다. src/eval-pattern-match.ts:83~91의 catch는 대괄호 접두어를 **한 번만** 제거하므로 \\`[E_TYPE_MISMATCH]\\`는 남는다. 처음 정답은 evalBuiltin의 메시지에서 이 접두어를 제거하고 evalSExpr의 포장을 반영하지 않아 잘못됐다. 전체 실패 \\`L08_P2_PASS=9 FAIL=2\\` 원문과 기존 정답은 history에 보존한다. contracts의 string/nil 2줄, source-edge의 boolean 1줄에만 이 접두어를 복원한다. 힌트·나머지 문자열·프로그램·단정은 변경하지 않는다. runtime을 정답에 맞춰 고친 것이 아니며 기존 런타임 출력에 근거 없는 정답 변경도 아니다. Node 실행은 하지 않았다. [전체 실패 원문](../../docs/evidence/l08-platform/extern-current-full-before.txt). | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-phase2/contract.md:6` | literal '.ts': - [기본 연산 원문](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/src/eval-builtins.ts): 이름의 \\`_\\`→\\`-\\` 정규화 후 builtin이 먼저 선택된다. \\`=\\`는 flDeepEq, \\`!=\\`는 참조 비교다. str_to_num은 parseFloat이며 빈/비숫자는 nil, 숫자 접두사를 허용한다. html-escape의 apostrophe는 \\`&#x27;\\`다. str_split의 추가 인자는 결과 조각 수 기준이다. str_replace는 split/join 전역 리터럴 치환이므로 빈 패턴은 문자 사이만 치환한다. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-phase2/contract.md:7` | literal '.ts': - [특수 형식](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/src/eval-special-forms.ts): if/and는 JS truthiness, or는 nil/false만 거짓이며 단락 평가한다. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-phase2/contract.md:8` | literal '.ts': - [출력](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/src/interpreter.ts): toDisplayString, println 인자 사이 공백, str의 배열/일반 객체는 JSON 표현. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-phase2/contract.md:9` | literal '.js': - [문자열·배열 fallback 원문](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/bootstrap.js#L27165): builtin에 없는 str_slice/find/lines/URI·arr_*는 등록 모듈 계약을 따른다. 문자열 인덱스는 UTF-16 단위이며 이모지는 두 단위다. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/atom-lifetime-contract.md:3` | literal '.ts': v11 atom은 src/eval-builtins.ts에서 \\`{ value: ... }\\`로 만들고 FreeLang 사용자에게 명시적인 free를 요구하지 않는다. v11 기준 \\`b49cc9194a00649b8cf58e1d6e8ee38ffce770d4\\` 소스를 읽었으며 Node 실행으로 검증하지 않았다. C20은 요청/반복 할당 수명과 누수 검증을 요구한다. 참조가 사라진 임시 atom, self/two-atom/closure cycle은 수거돼야 하며 반환·캡처·컨테이너·외부 저장소로 도달하는 atom은 동일한 가변 참조로 유지돼야 한다. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/atom-snapshot-contract.md:3` | literal '.js': v11 \\`b49cc9194a00649b8cf58e1d6e8ee38ffce770d4\\` bootstrap.js의 atom/deref/reset!은 value field를 읽고 교체하며 이전 객체를 직접 free하지 않는다. 지역 변수·반환값이 참조하는 이전 map/vector/string은 reset 뒤에도 읽을 수 있다. source 기준으로 atom-snapshot.expected를 먼저 고정한다. Node를 실행하지 않는다. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/atom-update-contract.md:3` | literal '.ts': v11 \\`b49cc9194a00649b8cf58e1d6e8ee38ffce770d4\\`의 src/eval-builtins.ts:526~534,952~960 소스를 읽었다. callback을 현재 값·extra 인자로 한 번 호출하고, 반환한 값을 저장·반환한다. callback이 같은 atom에 reset!/swap!을 호출해도 바깥 호출은 원래 읽은 값으로 계산한 결과를 최종 저장한다. callback이 throw하면 바깥 저장은 하지 않지만 callback 안에서 이미 수행한 reset!은 취소하지 않는다. Node로 실행하지 않았다. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/auth-csrf.fl:4` | literal '.js': (define data (json_parse (file_read (str root "/csrf.json")))) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/auth-fixtures.c:22` | literal '.js':  FILE*f=file("password-fixtures.json","wb");fputs("[",f); | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/auth-fixtures.c:47` | literal '.js':  f=file("csrf.json","wb");fputs("{",f);token(f,"now","1800000000",key,0);token(f,"boundary","1799996400",key,1);token(f,"expired","1799996399",key,1);token(f,"future","1800000001",key,1);token(f,"padded"," +1800000000tail",key,1);token(f,"invalid","no-time",key,1);fputs("}",f);fclose(f); | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/auth-password.fl:3` | literal '.js': (define fixtures (json_parse (file_read (str root "/password-fixtures.json")))) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/auth-verify.fl:6` | literal '.js': (define fixtures (json_parse (file_read (str root "/password-fixtures.json")))) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/call-error-context-contract.md:3` | literal '.js': v11 \\`b49cc9194a00649b8cf58e1d6e8ee38ffce770d4\\`의 src/interpreter.ts:1991~2018와 bootstrap.js:40912~40931은 evalBuiltin 호출이 던진 Error의 message에 \\`[op]\\`를 붙이고 최초 \\`(at line ...)\\`만 보존한다. 이미 \\`[op]\\` 또는 \\`(op)\\`로 시작하면 같은 접두어를 다시 붙이지 않는다. 인자는 이 try 전에 평가하므로 인자 오류에는 아직 호출하지 않은 바깥 연산 문맥을 붙이지 않는다. evalTryBlock은 첫 대괄호 접두어와 위치를 제거한다. Node를 실행하지 않고 이 소스 계약으로 정답을 먼저 고정한다. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/compiler-parse-memory.fl:11` | literal 'node ':  (let [node (get nodes i) args (get node "args") value (first args)] | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/compiler-parse-memory.fl:12` | literal 'node ':   (if (and (= (get node "kind") "sexpr") (= (get node "op") "println") (= (get node "line") (+ i 1)) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/define-sequence-contract.md:9` | literal '.ts': v11 \\`b49cc9194a00649b8cf58e1d6e8ee38ffce770d4\\`의 src/eval-special-forms.ts:787~865는 define의 RHS를 먼저 평가하고 값을 저장한다. 같은 파일 :1249~1294의 do는 별도 scope를 push하지 않는다. src/eval-call-function.ts:54~76 propagateMutations는 변경된 캡처 이름을 capturedEnv와 savedStack으로 역전파하고 :530~549는 정상 함수 반환 전에 이를 호출한다. 따라서 캡처한 n=0에 +1·+2를 적용한 뒤 반환값과 외부 n이 모두 3인 원래 정답을 유지한다. Node로 실행하지 않았다. 함수 파라미터·기존 cell은 해당 바인딩을 재사용하고 같은 local 이름은 한 번만 선언해야 한다. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/feed-contract.md:3` | literal '.ts': 기준 v11 \\`b49cc9194a00649b8cf58e1d6e8ee38ffce770d4\\`, \\`src/stdlib-feed.ts\\`의 rss_feed와 sitemap_xml을 텍스트로 조사했다. Node 실행은 금지한다. source XML 줄 순서·개행·5개 escape, truthy optional 필드, 입력 item 순서 유지, 기본 language en, URL 끝 slash 한 개 처리, 숫자 priority만 출력하는 계약으로 정답을 구현 전에 고정한다. 절대 URL을 별도 재해석하거나 item을 정렬하지 않는다. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/http-client-contract.md:19` | literal '.ts': 시험 입력 정정 이력: 기준 v11 lexer.ts:159~176은 \\`\u\\` 문자열 escape를 해석하지 않으며 fx lexer도 동일하다. 최초 fixture의 직접 \\`\u0000\\`은 실제 NUL을 만들지 않았다. v11 소스 계약대로 JSON parser로 NUL을 생성하도록 입력을 고쳤고, 출력 정답·의미는 바꾸지 않았다. 최초 RESPONSE_UTF8_NUL assertion 실패는 실제 NUL과 문자 u0000 비교였음을 생성 C에서 확인했다. 또한 underscore file_read는 C10 텍스트 replacement API여서 FF가 이미 U+FFFD가 된 상태였다. wire의 FF 시험은 공존하기로 한 L07 file-read 바이너리 API로 바꿔 수신 단계의 replacement를 검사한다. 최초 fixture의 원문 실패는 보존하며 해당 출력으로 NUL·잘못된 UTF-8 통과를 주장하지 않는다. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/http-client-contract.md:3` | literal '.ts': 기준 v11 \\`b49cc9194a00649b8cf58e1d6e8ee38ffce770d4\\`, \\`src/stdlib-http.ts\\`의 nodeHttpRequest·http_get·http_request·http_request_timeout이다. TS/내장 worker JS는 텍스트로만 읽으며 실행하지 않는다. blog1의 HTTP 통합 시험은 http_request의 method/url/headers/body 순서를 사용한다. 현재 native http_req의 method/url/body/headers와 구분해 공개 연결하며 앱에서 인자를 뒤집지 않는다. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/http-platform-contract.md:3` | literal '.ts': 기준 v11 b49cc9194a00649b8cf58e1d6e8ee38ffce770d4의 stdlib-http-server.ts와 Node request.headers 계약을 조사했다. 시험은 loopback·bind(0)·소유한 서버 과정에서만 수행한다. C 함수 직접 호출을 통과 증거로 사용하지 않는다. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/late-call-contract.md:3` | literal '.ts': v11 \\`b49cc9194a00649b8cf58e1d6e8ee38ffce770d4\\`의 src/eval-special-forms.ts defn 분기는 body를 평가하지 않고 context.functions에 함수 정의를 등록한다. src/interpreter.ts:1932~1992는 실제 호출 시 인자를 평가한 뒤 evalBuiltin으로, src/eval-builtins.ts:7413~7414는 callUser로 전달한다. src/eval-call-function.ts:256~261은 이름을 찾지 못할 때 FunctionNotFoundError를 던진다. src/errors.ts:126~139의 고정 message 시작은 \\`Function not found: functionName\\`이며 후보에 따른 추가 hint는 변할 수 있다. Node를 실행하지 않고 소스로 정답을 고정한다. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/sort-context-contract.md:3` | literal '.ts': v11 \\`b49cc9194a00649b8cf58e1d6e8ee38ffce770d4\\`의 src/eval-builtins.ts:3974~4010 소스를 읽었다. sort-by는 호출별 local keyFn·sbExtract를 사용하며 입력 배열 복사본을 정렬한다. 순수 숫자 key의 오름차순·음수 key의 내림차순과 중첩 key 호출에서 외부 callback 유지가 정답이다. Node는 실행하지 않는다. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/tail-call-context-contract.md:3` | literal '.ts': v11 \\`b49cc9194a00649b8cf58e1d6e8ee38ffce770d4\\` src/eval-special-forms.ts:1216~1238은 tcoMode에서 선택한 if branch가 사용자 함수 호출이면 evalSExpr로 다시 감싸지 않고 TailCall을 반환한다. src/eval-call-function.ts:403~467은 일반 함수 body를 tcoMode=true로 평가하며 TailCall의 다음 함수를 반복 실행한다. 따라서 if branch의 호출 이름은 오류 접두어에 추가되지 않는다. 이와 달리 일반 함수 본문의 직접 sexpr 호출은 evalSExpr 문맥을 유지한다. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/tail-call-context-contract.md:5` | literal '.ts': let RHS는 eval-special-forms.ts:2238~2243의 evValue로, 호출 인자는 src/interpreter.ts:1932~1941에서 tcoMode=false로 평가한다. 함수 body의 let 뒤 if와 값 위치의 let/인자 안 if를 혼동하지 않는다. 정답은 이 소스에서 고정하며 Node를 실행하지 않는다. 상호 꼬리 호출과 깊이 5000의 오류 전달을 포함하여 접두어만 빼는 우회로 완료 판정하지 않는다. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/tail-call-context-contract.md:9` | literal '.ts': 추가 정답은 같은 소스의 while body가 tcoMode=false인 점(eval-special-forms.ts:1339~1347)과 함수 값이 caller mode를 상속하는 점(eval-call-function.ts:482~485/757~762)도 고정한다. 직접 recur와 다른 named tail branch가 함께 있는 함수, 값 위치로 실행되는 while body를 구분한다. tail token을 만들고 실행하지 않아 부작용이 사라지는 것도 실패다. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/tail-nonlast-contract.md:5` | literal '.ts': v11 eval-special-forms.ts:1248~1298의 do는 각 표현식을 순서대로 평가하며 if의 주석 계약은 ‘꼬리 위치’ 사용자 함수 호출이다. 다만 해당 v11 소스의 do loop에는 non-last tcoMode guard가 명시되어 있지 않다. v11 전체 실행에서 이 조합이 어떻게 처리되는지는 Node 금지로 실행 확인하지 않았다. source의 이 부분과 의도한 꼬리 위치 규칙을 혼동해 완전 호환이라고 보고하지 않는다. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/tcp-contract.md:3` | literal '.js': 기준 v11 \\`b49cc9194a00649b8cf58e1d6e8ee38ffce770d4\\` bootstrap.js:16436~16640의 raw TCP/stop/write/drop와 error drain을 텍스트로 조사했다. Node/내장 worker JS는 실행하지 않는다. 직접 raw 분기는 connect/data/close 3인자 callback, conn_N ID, binary 문자열, write ok/error/not-found, drop ok/not-found, stop ok/not-running을 제공한다. 별도 worker 내부 accept/listening 프로토콜을 이 직접 분기의 정답으로 섞지 않는다. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/tcp-echo.fl:29` | literal '.js':               (file_write (str root "/errors.json") (json_str all-errors)) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/tcp-echo.fl:34` | literal '.js':    (do (swap! pending dissoc id) (swap! modes dissoc id) (file_write (str root "/events.json") (json_str (deref events)))) nil))) | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/l08-platform/timer-contract.md:3` | literal '.js': 기준 v11 \\`b49cc9194a00649b8cf58e1d6e8ee38ffce770d4\\`의 bootstrap.js set-timeout 분기(17541), src/async-runtime.ts의 FreeLangPromise, src/stdlib-async.ts의 await_result를 텍스트로 조사했다. Node 실행은 하지 않는다. TCP·TLS는 같은 C19의 남은 작업이며 이 타이머 시험으로 완료 판정하지 않는다. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/native-bootstrap/process-exit-contract.md:7` | literal '.ts': The oracle pins the output markers and status values in process-exit.expected.tsv. | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/native-bootstrap/process-exit.sh:26` | literal '.ts': done < "$root/tests/native-bootstrap/process-exit.expected.tsv" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/native-bootstrap/process-exit.sh:31` | literal '.ts': done < "$root/tests/native-bootstrap/process-exit.expected.tsv" | 글자 | 해당 호출을 native FreeLang/C 경로로 대체 검토 |
| `tests/run-tests.sh` | node/npm/npx 차단 PATH에서 rc=0 | 실행 | 실패가 있으면 로그 원인 확인 |
| `tests/run-tests.sh` | node/npm/npx/python execve 없음 (rc=0) | 추적 | 해당 없음 |

NODE_FREE=290
