# V-10 회귀 확인 및 병합 준비

RUN-20261103-15. 수정 가지 기준 `fix/raw-tcp-review` `42e0bebe3f454da3104c48622d1c4c8ab0fa4fd1`; 코2 기준 `b8fdb29b94eec109db245172237c2d5a55e4c001`.

## 기준 가지 회귀

`42e0bebe3f454da3104c48622d1c4c8ab0fa4fd1`에서 `tests/native-bootstrap/raw-tcp-contract-probes.sh`, `raw-peer.sh`, `raw-peer-ports.sh`를 실행했다.

```text
RAW_TCP_REVIEW_SUMMARY=6/6
RAW_SLOW_WRITE_RESULT=error
RAW_SLOW_WRITE_ELAPSED_MS=31025
RAW_PEER=PASS
TCP_FIXED_PORT=10/10
TCP_RANDOM_PORT=10/10
```

계약 회귀 6/6은 빈/null host, float yield, chunk, binary length, 큰 응답, 30초 무수신 종료를 포함한다. raw TCP 필수 확인은 peer 1건과 고정·임의 포트 각 10회다.

## 최신 코2 원격 및 병합 시뮬레이션

Forgejo와 GitHub의 `feat/db95-native-peer` 원격 HEAD는 모두 아래 값이었다. 기준 `b8fdb29b94eec109db245172237c2d5a55e4c001`보다 앞서지 않았다.

```text
FORGEJO_CODE2_HEAD=b8fdb29b94eec109db245172237c2d5a55e4c001
GITHUB_CODE2_HEAD=b8fdb29b94eec109db245172237c2d5a55e4c001
TEMP_CLONE_HEAD=b8fdb29b94eec109db245172237c2d5a55e4c001
MERGE_SOURCE=42e0bebe3f454da3104c48622d1c4c8ab0fa4fd1
MERGE_RC=0
```

새 임시 복제본의 `v10-merge-simulation` 가지에서 `--no-commit --no-ff` 병합을 시험했다. Git 원문은 `Automatic merge went well; stopped before committing as requested`이며 충돌 파일은 없었다. 임시 병합 결과는 커밋하거나 푸시하지 않았다.

## 임시 병합본 회귀

병합 작업 트리에서 같은 회귀·raw TCP 필수 확인과 `raw-tcp-slow-client-v9.sh`를 실행했다.

```text
RAW_TCP_REVIEW_SUMMARY=6/6
RAW_SLOW_WRITE_RESULT=error
RAW_SLOW_WRITE_ELAPSED_MS=31035
RAW_PEER=PASS
TCP_FIXED_PORT=10/10
TCP_RANDOM_PORT=10/10
```

| 느린 수신자 | 수신 바이트 | 경과 | 서버 반환 | 다른 연결 응답 |
|---|---:|---:|---|---:|
| 조금 느림 | 16777216 | 1962ms | `V9_WRITE_RESULT=ok` | 2ms |
| 많이 느림 | 16777216 | 6708ms | `V9_WRITE_RESULT=ok` | 3ms |
| 읽지 않음 | 4227072 (연결 종료 뒤 읽힌 응답) | 31226ms | `V9_WRITE_RESULT=error` | 3ms |

세 경우 모두 `V9_READY_RESULT=ok`였다. 무수신 client만 `timeout 40`으로 실행했다.

## 71건 gate 상태

저장소에서 이름이나 결과가 71건으로 고정된 별도 gate runner와 코2 기준 `34`건 결과를 찾지 못했다. 가장 넓은 기존 `scripts/verify-l08-platform-all.sh`를 임시 병합본에서 실행했으나 compiler 준비 단계에서 실패했다. 실제 첫 오류는 다음과 같다.

```text
<가림>/runtime/http_client.c:16:10: fatal error: curl/curl.h: No such file or directory
PLATFORM_SELF_RC=1
PLATFORM_GATE_RC=1
```

compiler가 빌드되지 않아 이 gate의 assertion 통과 수는 `0`건으로 확정된 것이 아니라 **미실행**이다. 따라서 71건 gate 통과 수와 기준 34건 대비 변화는 판정하지 않았다. 별도 71건 runner가 확인되면 그 gate를 실행해야 한다.

## 언어 성과 및 범위

임시 병합 결과에서도 raw TCP 언어 동작은 유지됐다. `raw-tcp-deferred-boundaries.fl` 회귀 6/6과 세 속도 `raw-tcp-slow-client-v9.sh`에서 느린 연결 진행·다른 연결 양보·30초 무수신 종료를 확인했다. 언어 compiler 결함은 확인되지 않았다.

V-10에서는 임시 복제본만 병합 시험에 사용했다. 코2 작업 폴더·가지에는 쓰지 않았고 실제 병합 및 푸시는 하지 않았다. 지정된 gate runner 부재와 native compiler의 curl 헤더 의존성 때문에 전체 병합 준비 판정은 보류다.

V10_SUMMARY 3/4

## V-10 보충: afj-db95 native 71건 gate

보충 RUN-20261103-15b에서 `afj-db95`를 원격 Forgejo에서 새 임시 복제본으로 받았다. Forgejo와 GitHub `native-run` 최신 원격 HEAD는 모두 `85424a5d3dacb829f4de921a7767cccec558eae0`였고, 기준 `f67661bae549204ada8250848f9897eb07ec7a1b`보다 앞선 커밋이다. 임시 복제본은 이 최신 HEAD에서 실행했다. 복제본은 커밋·푸시하지 않았다.

`docs/DB95-10-regress-row.md`의 gate는 `tests/native-release-gate.sh`이며, 원문 기준 결과는 다음과 같다.

```text
NATIVE_GATE_PASS=34 FAIL=37 TIMEOUT=0 SKIP=0 TOTAL=71
```

같은 runner에 임시 병합 FX의 Stage2 compiler를 넘긴 실행 명령(비밀 없는 빌드 경로는 보고서 규칙에 따라 가림):

```sh
FX_ROOT=<가림> FX_NATIVE_STAGE0=<가림> NATIVE_GATE_LOG_DIR=<가림> CPATH=<가림> LIBRARY_PATH=<가림> bash tests/native-release-gate.sh
```

`FX_NATIVE_STAGE0`에는 Stage0 실행 파일이 아니라 임시 병합본에서 생성한 Stage2 compiler를 지정했다. Stage2 생성 C 소스의 SHA-256은 원문 그대로 다음과 같다.

```text
STAGE2_SOURCE_SHA256=6a0d4e384a18834d4f0cb03c10460aade5bf05f999f5db981dd5dc275d313e7d
STAGE2_COMPILER=<가림>
```

없는 curl 헤더를 설치하지 않았다. 이미 존재하는 curl include 파일 경로와 `libcurl.so.4`를 가리키는 임시 `LIBRARY_PATH` 항목만 실행 환경에 사용했다. 최초 runner 호출은 `FX_ROOT`를 AFJ 복제본으로 잘못 지정해 `NATIVE_FX_CHECKOUT_MISSING`에서 종료됐고 사례 실행 전의 준비 오류였다. 올바른 FX 병합본을 지정한 유효 실행을 다시 시작했다.

세 사례가 서로 다른 시험 이름에서 같은 120초 compiler timeout으로 실패해, 3회 반복 실패 중단 규칙을 적용했다. 따라서 전체 gate summary는 생성되지 않았고 71건 완료 결과나 기준 34건과의 총계 차이를 주장하지 않는다. 중단 시 기록된 항목은 다음과 같다.

```text
NATIVE_CASE=1 status=FAIL exit=124 label=syntax: tcp-server
NATIVE_CASE=2 status=PASS exit=0 label=syntax: afj-db95
NATIVE_CASE=3 status=FAIL exit=1 label=raw binary boundary
NATIVE_CASE=4 status=FAIL exit=1 label=MariaDB packet codec
NATIVE_CASE=5 status=FAIL exit=1 label=MariaDB charset contract
NATIVE_CASE=6 status=FAIL exit=124 label=COLLATE contract
NATIVE_CASE=7 status=FAIL exit=1 label=page/WAL smoke
NATIVE_CASE=8 status=FAIL exit=124 label=metrics contract
NATIVE_CASE=9 status=FAIL exit=127 label=local canary rollback
```

오류 원문:

```text
Uncaught error: [buf-to-binary] Function not found: buf-to-binary (at line 2, col 0)
Uncaught error: [buf-str] Function not found: buf-str (at line 116, col 0)
Uncaught error: [buf-str] Function not found: buf-str (at line 116, col 0)
Uncaught error (line 455): [oversized-wal-smoke-test] oversized WAL was accepted (at line 486, col 0)
<가림> line 11: node: command not found
```

세 timeout 원문은 case 1 `syntax: tcp-server`, case 6 `COLLATE contract`, case 8 `metrics contract`이며 각각 `exit=124`다. 나머지 62개 사례는 이 중단 실행에서 확인하지 않았다. node는 호출되지 않았으며, case 9의 격리된 shell에서 실행기가 `node`를 찾지 못했다. DB95 JS runner와 시험 코드는 수정하지 않았다.

이 보충은 FX 다른 gate의 `curl/curl.h` 오류와 구분된다. FX 병합 시뮬레이션과 afj-db95 임시 복제본 모두 커밋·푸시하지 않았다. 아래 요약은 전체 gate 미완료 상태를 반영한다.

V10_SUMMARY 3/4
