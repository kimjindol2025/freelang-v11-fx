# V-18 실패 증거 재수집

- RUN-ID: `RUN-20261104-19`
- 작업 가지: `fix/raw-tcp-review`
- FX snapshot: `b8fdb29b94eec109db245172237c2d5a55e4c001`
- AFJ DB95 snapshot: `b60320b86de044179f284d2e7aa537a111e6da0f`
- 기준 Stage2 실행 파일 SHA-256: `9e08f102a579aed6ad77edac06997637e9f7b05dbe7197de1002107a6e905864`
- Stage2 생성 소스 SHA-256: `6a0d4e384a18834d4f0cb03c10460aade5bf05f999f5db981dd5dc275d313e7d`
- 시험 전·후 해시는 다음 값과 각각 일치했다. Stage0는 실행하거나 사용하지 않았다.

```text
9e08f102a579aed6ad77edac06997637e9f7b05dbe7197de1002107a6e905864  <가림>/stage2-compiler
6a0d4e384a18834d4f0cb03c10460aade5bf05f999f5db981dd5dc275d313e7d  <가림>/stage2.c
```

## 실행 및 기록

지정 컴파일러와 Stage2 소스 해시 확인 후 gate를 한 번 실행했다. 시작 직전 `ps`에서 다른 Stage2 compiler, native gate, 또는 고부하 GCC/cc1 실행은 확인되지 않았다. 실행 형태는 다음과 같고 모든 실제 경로와 환경값은 가렸다.

```text
TMPDIR=<가림> FX_ROOT=<가림> FX_NATIVE_STAGE0=<가림> CPATH=<가림> LIBRARY_PATH=<가림> NATIVE_GATE_LOG_DIR=<가림> flock -o /tmp/fl-heavy.lock bash tests/native-release-gate.sh
```

`FX_NATIVE_STAGE0`에는 변수 이름과 달리 지정된 Stage2 실행 파일을 전달했다. gate 전체는 `flock -o /tmp/fl-heavy.lock` 아래에서 수행했고, `TMPDIR` 및 산출물은 `<가림>` 작업 공간으로 지정했다. 전체 gate 종료 코드는 `1`이다.

실행 요약 원문:

```text
NATIVE_GATE_PASS=4 FAIL=67 TIMEOUT=39 SKIP=0 TOTAL=71
```

gate runner가 보존한 로그에는 71개 case 각각의 출력, 종료 코드, 경과시간, 단계, 실행 프로세스 그룹 정리 결과가 있다. case 결과 행 71개, `output.log` 71개이며, 각 그룹의 종료 뒤 잔류 프로세스 수는 71/71 모두 0이다. 보고서에는 전체 경로를 싣지 않고 가렸다.

원래 DB95 snapshot과 비교해 직접 `/tmp`를 지정한 임시 시험 입력의 출력 경로만 `<가림>` 작업 공간으로 옮겼다. 기대값·제한시간·시험 동작은 바꾸지 않았다. case별 출력 보존과 프로세스 그룹 기록을 위해 task-local native gate runner를 계측했다. 원격 저장소나 제품 소스에는 변경을 하지 않았다. 일반 제한시간은 120초로 유지했다.

## 71개 case 원문 결과

오류 칸은 해당 case 로그에서 처음 확인되는 오류 진단 줄이다. timeout 로그에 진단문이 없으면 그 사실을 표시했다. 경로는 `<가림>`으로 처리했다. 모듈은 root 시험 입력에서 전이적으로 `(load ...)`된 고유 파일 수와 각 파일의 `wc -l` 합이다. root 입력 파일은 합계에서 제외했다. `.js`와 `.sh` 입력은 로드 모듈 0개로 기록된다.

| # | Case | 결과/RC | 시간(ms) | 실패 단계 | 모듈 수/줄 합 | 첫 오류 줄 또는 로그 사실 |
|---:|---|---|---:|---|---:|---|
| 1 | syntax: tcp-server | FAIL (124) | 120005 | FreeLang→C | 4/2584 | 시간초과; 로그에 오류 문구 없음 |
| 2 | syntax: afj-db95 | PASS (0) | 7710 | GCC syntax | 0/0 | 실패 진단문 없음 |
| 3 | raw binary boundary | FAIL (1) | 11604 | execution | 0/0 | Uncaught error: [buf-to-binary] Function not found: buf-to-binary (at line 2, col 0) |
| 4 | MariaDB packet codec | FAIL (1) | 26572 | execution | 2/597 | Uncaught error: [buf-str] Function not found: buf-str (at line 116, col 0) |
| 5 | MariaDB charset contract | FAIL (1) | 26417 | execution | 2/597 | Uncaught error: [buf-str] Function not found: buf-str (at line 116, col 0) |
| 6 | COLLATE contract | FAIL (124) | 120006 | FreeLang→C | 2/2237 | 시간초과; 로그에 오류 문구 없음 |
| 7 | page/WAL smoke | FAIL (1) | 22493 | execution | 0/0 | Uncaught error (line 455): [oversized-wal-smoke-test] oversized WAL was accepted (at line 486, col 0) |
| 8 | metrics contract | FAIL (124) | 120005 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 9 | local canary rollback | FAIL (127) | 99 | execution | 0/0 | <가림>: line 11: node: command not found |
| 10 | large corpus | FAIL (124) | 120005 | FreeLang→C | 2/2237 | 시간초과; 로그에 오류 문구 없음 |
| 11 | TCP catalog pages | FAIL (124) | 120005 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 12 | TCP lazy catalog eviction | FAIL (124) | 120006 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 13 | persistent recursive B+Tree | FAIL (1) | 25789 | execution | 2/664 | Uncaught error (line 455): [oversized-wal-smoke-test] oversized WAL was accepted (at line 486, col 0) |
| 14 | bounded B+Tree range | FAIL (124) | 120005 | FreeLang→C | 2/2237 | 시간초과; 로그에 오류 문구 없음 |
| 15 | compound unique index | FAIL (124) | 120005 | FreeLang→C | 2/2237 | 시간초과; 로그에 오류 문구 없음 |
| 16 | compound predicate range | FAIL (124) | 120006 | FreeLang→C | 2/2237 | 시간초과; 로그에 오류 문구 없음 |
| 17 | schema MVCC merge | FAIL (124) | 120006 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 18 | composite type order | FAIL (124) | 120006 | FreeLang→C | 2/2237 | 시간초과; 로그에 오류 문구 없음 |
| 19 | composite NULL order | FAIL (124) | 120005 | FreeLang→C | 2/2237 | 시간초과; 로그에 오류 문구 없음 |
| 20 | schema-aware composite order | FAIL (124) | 120005 | GCC link | 2/2237 | 시간초과; 로그에 오류 문구 없음 |
| 21 | mixed declared composite types | FAIL (124) | 120005 | FreeLang→C | 2/2237 | 시간초과; 로그에 오류 문구 없음 |
| 22 | composite reopen rebuild | FAIL (124) | 120005 | FreeLang→C | 2/2237 | 시간초과; 로그에 오류 문구 없음 |
| 23 | schema-aware composite range | FAIL (124) | 120006 | GCC link | 2/2237 | 시간초과; 로그에 오류 문구 없음 |
| 24 | TCP concurrency | FAIL (1) | 732 | FreeLang→C / JS source not ported | 0/0 | [IR-ERR] literal bad type=unknown \| kind=literal line=138 |
| 25 | TCP 5m performance envelope | FAIL (1) | 849 | FreeLang→C / JS source not ported | 0/0 | [CGC-ERR] unsupported literal type=unknown line=11 |
| 26 | backup smoke | FAIL (1) | 21252 | execution | 0/0 | Uncaught error (line 210): [backup-smoke-test] [backup-bundle] [backup-manifest] [file_mtime] Function not found: file_mtime (at line 18, col 0) |
| 27 | backup restore soak | FAIL (1) | 21542 | execution | 1/406 | Uncaught error (line 219): [backup-smoke-test] [backup-bundle] [backup-manifest] [file_mtime] Function not found: file_mtime (at line 18, col 0) |
| 28 | compatibility scorecard | FAIL (1) | 11959 | comparison | 0/0 | NATIVE_PASS_MARKER_MISSING |
| 29 | AFJ error token mapping | FAIL (124) | 120005 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 30 | MariaDB remediation regression | FAIL (124) | 120006 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 31 | MariaDB unsupported contract | FAIL (124) | 120005 | GCC link | 2/2237 | 시간초과; 로그에 오류 문구 없음 |
| 32 | TCP database session scope | FAIL (124) | 120005 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 33 | WAL format | FAIL (124) | 120006 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 34 | forced-kill WAL recovery | FAIL (1) | 35 | execution | 0/0 | <가림>: line 24: node: command not found |
| 35 | TCP transaction forced-kill recovery | FAIL (1) | 813 | FreeLang→C / JS source not ported | 0/0 | [CGC-ERR] unsupported literal type=unknown line=12 |
| 36 | transaction isolation | FAIL (124) | 120005 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 37 | isolation levels | FAIL (124) | 120006 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 38 | MVCC conflicts | FAIL (124) | 120005 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 39 | range predicate conflict | FAIL (124) | 120006 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 40 | row-version MVCC core | PASS (0) | 13141 | comparison | 1/114 | 실패 진단문 없음 |
| 41 | row-version WAL recovery | FAIL (124) | 120006 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 42 | row-version catalog checkpoint | FAIL (124) | 120006 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 43 | savepoint | FAIL (124) | 120005 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 44 | deadlock | FAIL (124) | 120005 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 45 | lock timeout | FAIL (124) | 120006 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 46 | prepared JSON adapter | FAIL (124) | 120005 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 47 | request limits | FAIL (124) | 120005 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 48 | slow request timeout | FAIL (124) | 120005 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 49 | auth lockout | PASS (0) | 15874 | comparison | 1/280 | 실패 진단문 없음 |
| 50 | auth lockout reset | FAIL (1) | 15510 | execution | 1/280 | Uncaught error (line 176): [sleep_ms] Function not found: sleep_ms (at line 18, col 0) |
| 51 | auth state recovery | PASS (0) | 15647 | comparison | 1/280 | 실패 진단문 없음 |
| 52 | session cleanup | FAIL (124) | 120005 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 53 | session TTL | FAIL (124) | 120005 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 54 | connection rate limit | FAIL (124) | 120005 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 55 | peer IP rate limit | FAIL (124) | 120005 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 56 | runtime peer IP rate limit | FAIL (1) | 494 | FreeLang→C / JS source not ported | 0/0 | [CGC-ERR] unsupported literal type=unknown line=12 |
| 57 | fragmented TCP requests | FAIL (124) | 120005 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 58 | SQL error mapping | FAIL (124) | 120005 | FreeLang→C | 5/4628 | 시간초과; 로그에 오류 문구 없음 |
| 59 | production TLS boundary | FAIL (1) | 39 | execution | 0/0 | production TLS boundary returned an unexpected error |
| 60 | native TLS listener | FAIL (1) | 475 | FreeLang→C / JS source not ported | 0/0 | [CGC-ERR] unsupported literal type=unknown line=9 |
| 61 | native TLS mTLS/reload | FAIL (1) | 595 | FreeLang→C / JS source not ported | 0/0 | [CGC-ERR] unsupported literal type=unknown line=9 |
| 62 | production native TLS integration | FAIL (1) | 603 | FreeLang→C / JS source not ported | 0/0 | [CGC-ERR] unsupported literal type=unknown line=9 |
| 63 | TLS adapter handshake/reload | FAIL (1) | 1209 | FreeLang→C / JS source not ported | 0/0 | [CGC-ERR] unsupported literal type=unknown line=8 |
| 64 | MariaDB wire packet unit | FAIL (1) | 276 | FreeLang→C / JS source not ported | 0/0 | [CGC-ERR] unsupported literal type=unknown line=9 |
| 65 | MariaDB mysql_native_password SHA-1 | FAIL (1) | 25160 | execution | 2/597 | Uncaught error: [buf-str] Function not found: buf-str (at line 116, col 0) |
| 66 | MariaDB wire prepared | FAIL (1) | 968 | FreeLang→C / JS source not ported | 0/0 | [IR-ERR] literal bad type=unknown \| kind=literal line=122 |
| 67 | MariaDB standard-client smoke | FAIL (1) | 862 | FreeLang→C / JS source not ported | 0/0 | [CGC-ERR] unsupported literal type=unknown line=15 |
| 68 | native MariaDB wire | FAIL (1) | 462 | FreeLang→C / JS source not ported | 0/0 | [CGC-ERR] unsupported literal type=unknown line=9 |
| 69 | native MariaDB auth rejection | FAIL (1) | 737 | FreeLang→C / JS source not ported | 0/0 | [CGC-ERR] unsupported literal type=unknown line=11 |
| 70 | native MariaDB decoder | FAIL (1) | 25555 | execution | 2/597 | Uncaught error: [buf-str] Function not found: buf-str (at line 116, col 0) |
| 71 | native MariaDB prepared | FAIL (1) | 2207 | FreeLang→C / JS source not ported | 0/0 | [IR-ERR] literal bad type=unknown \| kind=literal line=83 |

## 모듈 크기와 timeout

`exit=124`인 timeout 39건은 전부 하나 이상의 모듈을 불러왔다. 고유 로드 모듈의 줄 합계는 최소 `2237`, 최대 `4628`, 총 `149756`, 평균 `3839.90`줄이었다. 분포는 `2237`줄 12건, `2584`줄 1건, `4628`줄 26건이다. 단계는 `FreeLang→C` 36건, `GCC link` 3건이다.

timeout이 아닌 실패 28건은 로드 모듈 줄 합계 총 `3738`, 평균 `133.50`줄이었다. 이 중 21건은 0줄, 7건은 1개 이상의 모듈을 로드했고 최댓값은 `664`줄이다. 이 실행에서는 timeout이 2237줄 이상인 비교적 큰 공통 모듈 그래프에 집중됐다. 따라서 모듈 크기와 timeout은 강하게 함께 움직였지만, 이 관찰만으로 크기가 timeout의 유일한 원인이라고 단정하지 않는다. 측정은 컴파일 환경 한 곳의 단일 gate 실행이다.

## V-17 미분류 8건 재분류

| Case | 이번 로그의 첫 진단/관찰 | 분류 |
|---:|---|---|
| 26 | `Uncaught error (line 210): [backup-smoke-test] [backup-bundle] [backup-manifest] [file_mtime] Function not found: file_mtime (at line 18, col 0)` | 런타임 함수 미구현/미등록 |
| 27 | `Uncaught error (line 219): [backup-smoke-test] [backup-bundle] [backup-manifest] [file_mtime] Function not found: file_mtime (at line 18, col 0)` | 런타임 함수 미구현/미등록 |
| 28 | `NATIVE_PASS_MARKER_MISSING` | 출력 비교 계약 불일치: 로그에 `PASS` marker 없음 |
| 34 | `<가림>/tests/fault-injection-restart.sh: line 24: node: command not found` | 시험 환경 의존성: Node 실행 파일 없음 |
| 50 | `Uncaught error (line 176): [sleep_ms] Function not found: sleep_ms (at line 18, col 0)` | 런타임 함수 미구현/미등록 |
| 59 | `production TLS boundary returned an unexpected error`; 다음 로그 줄은 `env: ‘node’: No such file or directory` | TLS shell 시험의 Node 의존성/실행 환경 |
| 65 | `Uncaught error: [buf-str] Function not found: buf-str (at line 116, col 0)` | 런타임 함수 미구현/미등록 |
| 70 | `Uncaught error: [buf-str] Function not found: buf-str (at line 116, col 0)` | 런타임 함수 미구현/미등록 |

## 실패 묶음 및 판정

67건의 실패는 빈칸 없이 다음과 같이 분류된다.

| 원인 묶음 | 건수 | 근거 |
|---|---:|---|
| 컴파일 timeout (`FreeLang→C`) | 36 | 120초 종료 코드 `124`, phase 로그 |
| 컴파일 timeout (`GCC link`) | 3 | 120초 종료 코드 `124`, phase 로그 |
| native gate에서 JS 입력 미지원 | 14 | `.js` branch의 `[IR-ERR]`/`[CGC-ERR]`, runner의 `NATIVE_JS_SOURCE_NOT_PORTED` |
| 런타임 함수 없음 | 8 | `buf-to-binary` 1, `buf-str` 4, `file_mtime` 2, `sleep_ms` 1 |
| oversized WAL 거부 기대 불일치 | 2 | `[oversized-wal-smoke-test] oversized WAL was accepted` |
| 실행 환경에 Node 없음 | 3 | case 9·34의 `node: command not found`, case 59의 `env: ‘node’: No such file or directory` |
| 성공 marker 비교 실패 | 1 | `NATIVE_PASS_MARKER_MISSING` |
| **실패 합계** | **67** | PASS 4건을 제외한 전체 실패 수 |

판정은 **(가) 시간 초과가 대부분 모듈 크기 때문**으로 한다. 39개 timeout 모두 모듈 줄 합계가 2237줄 이상이고, `FreeLang→C`에서 36건이 끝났다. 나머지 28건도 별도의 gate/환경/런타임 실패 묶음으로 전부 확인됐으므로, 67건 전체를 하나의 원인으로 해석하지 않는다. 언어 런타임 결함 후보로는 `buf-to-binary`, `buf-str`, `file_mtime`, `sleep_ms` 부재와 oversized-WAL 기대 불일치가 관찰됐다. JS 미지원·Node 부재·PASS marker 항목은 시험 도구/환경 계약 문제다.

언어 성과: 이번 기준 컴파일러는 71건 중 `4`건을 통과했다. 실패 원인별 수는 위 표와 같아 빈칸이 없다.

## 푸시 검증

보고서 파일만 `fix/raw-tcp-review` 가지에 추가했다. 민감정보 diff 검사에서 자격정보 URL·토큰·비밀값은 발견되지 않았고, 보고서 내 경로는 `<가림>`으로 처리했다. Forgejo를 먼저, GitHub를 다음으로 push했고, 마지막 확인에서 두 원격 branch HEAD가 일치했다.

| 항목 | 결과 |
|---|---|
| gate 실행 | 1회, 71/71 case 기록 |
| 컴파일러/Stage2 해시 | 시작·종료 모두 지정값 일치 |
| timeout / 기타 실패 / 통과 | 39 / 28 / 4 |
| V-17 미분류 | 8건 모두 재분류 |
| 문서 커밋 및 Forgejo→GitHub | 완료, 두 원격 branch HEAD 일치 확인 |

V18_SUMMARY=4/4
V18_SUMMARY=4/4
