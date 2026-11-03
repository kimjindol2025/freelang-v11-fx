# V-8 raw_tcp 검토 지적 정리

RUN-20261103-11. 기준 `freelang-v11-fx` HEAD `b8fdb29b94eec109db245172237c2d5a55e4c001`. 이 커밋에서 새로 만든 `fix/raw-tcp-review` 가지에서만 변경했다. `db-stability`와 코2 가지는 수정·병합하지 않았다.

## 항목별 수정 전·후

| 항목 | 수정 전 `.fl` 증거 | 변경 및 수정 후 |
|---|---|---|
| 빈/null host | `raw-tcp-deferred-boundaries.fl`의 empty/null probe 모두 `error`를 반환했다. 이번 환경에서는 동작 실패가 재현되지 않았다. 기존 구현이 빈값을 `getaddrinfo`로 넘기므로 런타임 라이브러리에 의존했다. | `runtime/raw_tcp.c`에서 문자열 여부, null 객체, 길이 0을 `getaddrinfo` 전에 거부하도록 명시했다. 같은 두 `.fl` probe가 계속 `error`를 반환했다. |
| 1ms 미만 float yield | 100회 `0.5` 호출 총 `RAW_FLOAT_YIELD_ELAPSED_MS=5`; timeout 인자가 정수만 처리돼 사실상 0ms였다. | 양수 float ms를 올림해 최소 1ms로 poll한다(상한 60000ms 유지). 수정 후 `RAW_FLOAT_YIELD_ELAPSED_MS=116`, `RAW_FLOAT_YIELD_DONE=1`. |
| 4096 chunk·binary 길이·큰 응답 | 8192 입력은 `RAW_DATA_LEN=4096` 두 번으로 총 8192바이트였고 chunk 유실은 재현되지 않았다. NUL 데이터의 길이는 `RAW_DATA_LEN=1`, echo도 `1`바이트였다. 8192바이트 응답은 수정 전에도 전부 전달됐다. | 수신 길이를 `fl_str_val_n`에 전달하고 문자열 `length`가 저장된 byte length를 사용하도록 했다. 재시험에서 chunk는 두 이벤트 합계 8192, NUL 포함 echo는 `RAW_DATA_LEN=5`, `RAW_BINARY_WRITE_RESULT=ok`, `RAW_BINARY_ECHO_BYTES=5`; 큰 응답은 `RAW_LARGE_RESPONSE_BYTES=8192`로 끝까지 전달됐다. |
| 느린 클라이언트 2초 정지 | 느린 수신자에게 16MiB 전송 시 결과가 `2379ms` 안에 나오지 않았고, 대기 후 `error`가 기록됐다. 기존 2초 `SO_SNDTIMEO`가 부분 send마다 다시 걸릴 수 있었다. | `SO_SNDTIMEO`를 제거하고 `MSG_DONTWAIT` 송신을 사용했다. 느린 수신자 probe는 `RAW_SLOW_WRITE_RESULT=error`, 마지막 확인 `RAW_SLOW_WRITE_ELAPSED_MS=36`으로 즉시 실패를 반환했다. 빠른 수신자 8192바이트 응답과 binary echo는 완전 전송을 확인했다. |

문자열 생성에 추가한 `fl_str_val_n`은 길이와 trailing NUL을 별도로 저장한다. raw TCP 이벤트 payload 길이는 `recv` 반환값을 그대로 사용한다. 변경 파일은 `runtime/raw_tcp.c`, `runtime/core.c`, `runtime/collection.c`, `runtime/runtime.h`와 해당 `.fl`/shell probe다.

수정 위치: host 검사 `runtime/raw_tcp.c:94`, 비차단 write `runtime/raw_tcp.c:123`, float timeout `runtime/raw_tcp.c:150`, 수신 payload 길이 `runtime/raw_tcp.c:207-211`, 길이 문자열 생성 `runtime/core.c:31`, 문자열 `length` `runtime/collection.c:415-418`.

## 회귀·필수 확인

수정 후 `tests/native-bootstrap/raw-tcp-contract-probes.sh` 결과는 `RAW_TCP_REVIEW_SUMMARY=6/6`이었다. 여섯 항목은 host 입력, float yield, 8192-byte chunk, binary echo, 큰 응답, 느린 수신자다. 추가 출력 `RAW_NAME_DISPATCH=REJECTED`도 통과했다.

필수 raw_tcp 확인 3건:

- contract probe: `RAW_TCP_REVIEW_SUMMARY=6/6`, exit `0`.
- peer 주소 응답: `RAW_PEER=PASS`, `RAW_PEER_RC=0`.
- 고정·임의 포트: `TCP_FIXED_PORT=10/10`, `TCP_RANDOM_PORT=10/10`, `RAW_PEER_PORTS_RC=0`.

chunk 경계 probe 재실행은 8192 입력을 두 번의 `4096` 이벤트로 전달했고 합계 8192로 통과했다. 원래 4096 경계 자체에서 데이터 유실은 없었다. NUL 포함 payload는 수정 전 1바이트에서 수정 후 5바이트 길이 echo로 바뀌었다.

첫 harness 시도는 server stdout line buffering이 없어 준비 완료를 제때 읽지 못했다. line-buffered harness로 고친 뒤 기준 구현 재현과 수정 후 검증을 완료했으며 이 초기 harness 오류는 제품 실패로 세지 않았다. 기준 구현에서 수정 전 회귀 집계는 `RAW_TCP_REVIEW_SUMMARY=3/6`(host, chunk, 큰 응답 통과; float, binary, slow 미통과)였다. 빈/null host의 수정 전 반환은 이미 `error`였으므로 실패 재현은 아니며, 명시적 입력 검사를 추가했다.

## Native bootstrap 및 고정점

strace 없이 Stage0부터 Stage4까지 native compiler로 실행했다. 결과는 `STAGE0_C_BUILD=PASS`, `STAGE0_TO_STAGE1=PASS`, `STAGE1_C_BUILD=PASS`, `STAGE1_TO_STAGE2=PASS`, `STAGE2_C_BUILD=PASS`, `STAGE2_TO_STAGE3=PASS`, `CMP_2_3=0`, `STAGE3_C_BUILD=PASS`, `STAGE3_TO_STAGE4=PASS`, `CMP_3_4=0`, `NATIVE_BOOTSTRAP=PASS`다.

```text
STAGE2_SHA256=6a0d4e384a18834d4f0cb03c10460aade5bf05f999f5db981dd5dc275d313e7d
STAGE3_SHA256=6a0d4e384a18834d4f0cb03c10460aade5bf05f999f5db981dd5dc275d313e7d
STAGE4_SHA256=6a0d4e384a18834d4f0cb03c10460aade5bf05f999f5db981dd5dc275d313e7d
```

직전 FIXPOINT_LOG의 2026-09-27 기준 이후 compiler source 변경이 축적됐다. 이번 시작 커밋 `b8fdb29b94eec109db245172237c2d5a55e4c001`에는 normalized `map-has-key-p` native dispatch 수정이 있고, V-8 변경은 `self/cgc-main.fl`을 건드리지 않았다. 이유와 원문 해시는 `FIXPOINT_LOG.md`에 기록했다. 시스템에 curl 개발 헤더가 없어 bootstrap의 HTTP 부분만 임시 stub으로 링크했으며 bootstrap 실행은 전부 native였고 strace·node는 사용하지 않았다.

언어 성과: 수정된 실행환경 결함은 raw TCP runtime의 float timeout 처리·payload byte length·느린 송신자 blocking이며, `raw-tcp-deferred-boundaries.fl` 및 `raw-peer.fl` 검증에서 언어 compiler 결함은 확인되지 않았다.

## 원격 확인

작업 복제본의 `origin`은 Forgejo, 추가한 `github`은 GitHub다. 두 원격의 기준 `feat/db95-native-peer`는 모두 `b8fdb29b94eec109db245172237c2d5a55e4c001`였고, `fix/raw-tcp-review`는 양쪽 모두 사전 확인 시 존재하지 않았다. 새 작업 가지를 두 원격에 순서대로 push하며 본줄기에는 합치지 않는다.

V8_SUMMARY 6/6
