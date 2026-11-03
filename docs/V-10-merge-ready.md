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
