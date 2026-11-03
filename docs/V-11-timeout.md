# V-11 시간 초과 3건 원인 가리기

RUN-20261103-16 / 담당 코1

## 시험 기준과 환경

- FX 기준본: `b8fdb29b94eec109db245172237c2d5a55e4c001`
- FX 병합본: 임시 병합 `7a1c278a4fe8268730f3351de3f6a234e8963731` + 기준본 `b8fdb29b94eec109db245172237c2d5a55e4c001` (커밋·푸시하지 않음)
- db95 임시 복제본: `85424a5d3dacb829f4de921a7767cccec558eae0`
- 두 FX 트리에서 각각 생성한 Stage2 소스 SHA-256: `6a0d4e384a18834d4f0cb03c10460aade5bf05f999f5db981dd5dc275d313e7d`
- 각 사례 제한 시간: `120`초. 변경하지 않았다.
- gate 실행: V-10 보충과 같은 `FX_ROOT=<가림> FX_NATIVE_STAGE0=<가림> NATIVE_GATE_LOG_DIR=<가림> CPATH=<가림> LIBRARY_PATH=<가림> bash tests/native-release-gate.sh`
- 무거운 실행 전 `ps`를 확인하고 `flock -o /tmp/fl-heavy.lock` 아래 수행했다. 실행 중 CPU 사용 프로세스(`grok-bot`, `node` 등)는 관찰했으나 코2의 작업인지 ps 자료만으로 식별할 수 없었다. 각 비교 실행의 ps 스냅샷을 보존했다.

## 대상 3건 비교

아래 시간은 실행 로그의 `ELAPSED_MS` 원문이다. 두 빌드 모두 세 시험이 제한 시간에 도달했다.

| 시험 | 기준본 | 병합본 |
|---|---|---|
| `syntax: tcp-server` (`src/tcp-server.fls`) | `TIMEOUT`, RC `124`, `ELAPSED_MS=120589` | `TIMEOUT`, RC `124`, `ELAPSED_MS=120549` |
| `COLLATE contract` (`tests/collation-contract.fls`) | `TIMEOUT`, RC `124`, `ELAPSED_MS=120610` | `TIMEOUT`, RC `124`, `ELAPSED_MS=120512` |
| `metrics contract` (`tests/metrics-contract.fls`) | `TIMEOUT`, RC `124`, `ELAPSED_MS=120600` | `TIMEOUT`, RC `124`, `ELAPSED_MS=120541` |

세 사례가 기준본에서도 모두 시간 초과했으므로 V-11 판정 규칙에 따라 raw_tcp 병합 변경과 무관한 결과로 기록한다. 이 조건은 병합본만 시간 초과할 때의 멈춤 조건을 충족하지 않았다.

## 71건 gate 결과

기준 비교 3건이 모두 시간 초과하여 병합본 Stage2 컴파일러로 전체 gate를 한 번 실행했다. 결과 원문:

```text
NATIVE_GATE_PASS=4 FAIL=67 TIMEOUT=39 SKIP=0 TOTAL=71
```

코2 기준 `NATIVE_GATE_PASS=34 FAIL=37 TIMEOUT=0 SKIP=0 TOTAL=71`보다 PASS가 `30`건 적다. 아래 실패 시험명과 각 사례 로그의 첫 진단을 기록한다. timeout 사례는 120초 안에 진단 줄이 나오지 않은 경우 `시간 초과 (exit=124)`로 표기했다.

| 번호 | 시험 | 첫 진단 |
|---:|---|---|
| 1 | syntax: tcp-server | 시간 초과 (exit=124) |
| 3 | raw binary boundary | Uncaught error: [buf-to-binary] Function not found: buf-to-binary (at line 2, col 0) |
| 4 | MariaDB packet codec | Uncaught error: [buf-str] Function not found: buf-str (at line 116, col 0) |
| 5 | MariaDB charset contract | Uncaught error: [buf-str] Function not found: buf-str (at line 116, col 0) |
| 6 | COLLATE contract | 시간 초과 (exit=124) |
| 7 | page/WAL smoke | Uncaught error (line 455): [oversized-wal-smoke-test] oversized WAL was accepted (at line 486, col 0) |
| 8 | metrics contract | 시간 초과 (exit=124) |
| 9 | local canary rollback | <가림> line 11: node: command not found |
| 10 | large corpus | 시간 초과 (exit=124) |
| 11 | TCP catalog pages | 시간 초과 (exit=124) |
| 12 | TCP lazy catalog eviction | 시간 초과 (exit=124) |
| 13 | persistent recursive B+Tree | Uncaught error (line 455): [oversized-wal-smoke-test] oversized WAL was accepted (at line 486, col 0) |
| 14 | bounded B+Tree range | 시간 초과 (exit=124) |
| 15 | compound unique index | 시간 초과 (exit=124) |
| 16 | compound predicate range | 시간 초과 (exit=124) |
| 17 | schema MVCC merge | 시간 초과 (exit=124) |
| 18 | composite type order | 시간 초과 (exit=124) |
| 19 | composite NULL order | 시간 초과 (exit=124) |
| 20 | schema-aware composite order | 시간 초과 (exit=124) |
| 21 | mixed declared composite types | 시간 초과 (exit=124) |
| 22 | composite reopen rebuild | 시간 초과 (exit=124) |
| 23 | schema-aware composite range | 시간 초과 (exit=124) |
| 24 | TCP concurrency | [IR-ERR] literal bad type=unknown | kind=literal line=138 |
| 25 | TCP 5m performance envelope | [CGC-ERR] unsupported literal type=unknown line=11 |
| 26 | backup smoke | Uncaught error (line 210): [backup-smoke-test] [backup-bundle] [backup-manifest] [file_mtime] Function not found: file_mtime (at line 18, col 0) |
| 27 | backup restore soak | Uncaught error (line 219): [backup-smoke-test] [backup-bundle] [backup-manifest] [file_mtime] Function not found: file_mtime (at line 18, col 0) |
| 28 | compatibility scorecard | afj-db95 compatibility baseline=88% |
| 29 | AFJ error token mapping | 시간 초과 (exit=124) |
| 30 | MariaDB remediation regression | 시간 초과 (exit=124) |
| 31 | MariaDB unsupported contract | 시간 초과 (exit=124) |
| 32 | TCP database session scope | 시간 초과 (exit=124) |
| 33 | WAL format | 시간 초과 (exit=124) |
| 34 | forced-kill WAL recovery | <가림> line 24: node: command not found |
| 35 | TCP transaction forced-kill recovery | [CGC-ERR] unsupported literal type=unknown line=12 |
| 36 | transaction isolation | 시간 초과 (exit=124) |
| 37 | isolation levels | 시간 초과 (exit=124) |
| 38 | MVCC conflicts | 시간 초과 (exit=124) |
| 39 | range predicate conflict | 시간 초과 (exit=124) |
| 41 | row-version WAL recovery | 시간 초과 (exit=124) |
| 42 | row-version catalog checkpoint | 시간 초과 (exit=124) |
| 43 | savepoint | 시간 초과 (exit=124) |
| 44 | deadlock | 시간 초과 (exit=124) |
| 45 | lock timeout | 시간 초과 (exit=124) |
| 46 | prepared JSON adapter | 시간 초과 (exit=124) |
| 47 | request limits | 시간 초과 (exit=124) |
| 48 | slow request timeout | 시간 초과 (exit=124) |
| 50 | auth lockout reset | Uncaught error (line 176): [sleep_ms] Function not found: sleep_ms (at line 18, col 0) |
| 52 | session cleanup | 시간 초과 (exit=124) |
| 53 | session TTL | 시간 초과 (exit=124) |
| 54 | connection rate limit | 시간 초과 (exit=124) |
| 55 | peer IP rate limit | 시간 초과 (exit=124) |
| 56 | runtime peer IP rate limit | [CGC-ERR] unsupported literal type=unknown line=12 |
| 57 | fragmented TCP requests | 시간 초과 (exit=124) |
| 58 | SQL error mapping | 시간 초과 (exit=124) |
| 59 | production TLS boundary | production TLS boundary returned an unexpected error |
| 60 | native TLS listener | [CGC-ERR] unsupported literal type=unknown line=9 |
| 61 | native TLS mTLS/reload | [CGC-ERR] unsupported literal type=unknown line=9 |
| 62 | production native TLS integration | [CGC-ERR] unsupported literal type=unknown line=9 |
| 63 | TLS adapter handshake/reload | [CGC-ERR] unsupported literal type=unknown line=8 |
| 64 | MariaDB wire packet unit | [CGC-ERR] unsupported literal type=unknown line=9 |
| 65 | MariaDB mysql_native_password SHA-1 | Uncaught error: [buf-str] Function not found: buf-str (at line 116, col 0) |
| 66 | MariaDB wire prepared | [IR-ERR] literal bad type=unknown | kind=literal line=122 |
| 67 | MariaDB standard-client smoke | [CGC-ERR] unsupported literal type=unknown line=15 |
| 68 | native MariaDB wire | [CGC-ERR] unsupported literal type=unknown line=9 |
| 69 | native MariaDB auth rejection | [CGC-ERR] unsupported literal type=unknown line=11 |
| 70 | native MariaDB decoder | Uncaught error: [buf-str] Function not found: buf-str (at line 116, col 0) |
| 71 | native MariaDB prepared | [IR-ERR] literal bad type=unknown | kind=literal line=83 |

통과한 네 사례는 `NATIVE_CASE=2 status=PASS exit=0 label=syntax: afj-db95`, `NATIVE_CASE=40 status=PASS exit=0 label=row-version MVCC core`, `NATIVE_CASE=49 status=PASS exit=0 label=auth lockout`, `NATIVE_CASE=51 status=PASS exit=0 label=auth state recovery`다. node를 직접 실행하지 않았다. gate 사례 중 shell 검사에서 `node: command not found`가 발생한 항목은 해당 실패 그대로 집계했다.

## 판정

세 대상 시간 초과는 기준본과 병합본에 공통으로 나타났으므로 raw_tcp 변경이 원인이라는 증거는 없다. 전체 gate는 기준보다 PASS가 30건 적고 39건이 timeout이다. 이 결과는 gate 전체를 통과로 판정하지 않는다. db95 임시 복제본, FX 임시 병합본, 시험 코드 및 기대값은 수정하거나 커밋하지 않았다.

V11_SUMMARY 4/4
