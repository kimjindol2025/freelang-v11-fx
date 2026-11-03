# V-9 느린 수신자 전송 제한

RUN-20261103-12b. 기준 `fix/raw-tcp-review` `71e0830eea52fa84091000cd325854f970cacca3`. 변경은 이 가지에만 적용했다.

## 변경

`runtime/raw_tcp.c`에 이름 붙은 `RAW_SEND_IDLE_TIMEOUT_MS` 상수 하나로 30000ms를 지정했다. `fl_tcp_write`는 `EAGAIN`/`EWOULDBLOCK`이면 남은 유휴 제한 시간 동안 `poll`하며, 매번 성공적으로 전송한 바이트가 있으면 30초 시계를 다시 시작한다. 대기 중 listener와 다른 연결의 입력을 처리하고, 송신 연결이 다시 쓸 수 있으면 전송을 이어간다. 30초 동안 진행이 없거나 다른 송신 오류가 나면 해당 연결을 닫고 `error`를 반환한다. `raw_emit`은 중첩 이벤트 호출 중 바깥 콜백의 arena 값을 보존하도록 `fl_arena_scope_enter/leave`를 사용한다.

수정 전 동작은 같은 `raw-tcp-deferred-boundaries.fl` 시험을 기준 커밋의 `runtime/raw_tcp.c`로 빌드해 재현했다. 기존 V8 느린 수신자 probe 원문은 `RAW_SLOW_WRITE_RESULT=error`, `RAW_SLOW_WRITE_ELAPSED_MS=35`였다. V9 전용 재현에서도 각 경우 `V9_WRITE_RESULT=error`가 즉시 나왔고, 조금 느린/많이 느린 클라이언트는 각각 4,096,000바이트에서 종료됐다. 두 번째 연결은 각각 2ms에 응답했다. 무수신 재현의 client는 `timeout 40`으로 실행했다.

## V9 수신 속도 시험

시험: `tests/native-bootstrap/raw-tcp-slow-client-v9.sh`가 `.fl` handler인 `raw-tcp-deferred-boundaries.fl`을 빌드하고 localhost 연결로 16,777,216바이트 응답을 요청했다. 두 정상 속도에서는 읽는 동안 두 번째 연결을 열어 `READY` 응답 시간을 쟀다. 무수신 경우에만 client에 `timeout 40`을 적용하고, 첫 `READY`를 읽은 뒤 응답은 읽지 않았다. 마지막 `BYTES_AFTER_CLOSE`는 연결 종료 뒤 커널에 남아 client가 읽은 응답 바이트 수다.

| 속도 | 수신 바이트 | 전송/종료 시간 | 서버 반환 | 다른 연결 READY |
|---|---:|---:|---|---:|
| 조금 느림 (`0.002`초/64KiB) | 16777216 | 1946ms | `V9_WRITE_RESULT=ok` | 2ms |
| 많이 느림 (`0.02`초/64KiB) | 16777216 | 6720ms | `V9_WRITE_RESULT=ok` | 3ms |
| 아예 읽지 않음 | 4227072 (연결 종료 후 읽힌 응답) | 31242ms client 측정 | `V9_WRITE_RESULT=error` | 3ms |

원문 출력에는 세 경우 모두 `V9_READY_RESULT=ok`가 있었다. 수정 후 계약 probe의 무수신 항목도 `RAW_SLOW_WRITE_RESULT=error`, `RAW_SLOW_WRITE_ELAPSED_MS=31031`로 30초 제한 후 종료됐다. 서버가 느린 송신자를 기다리는 동안 다른 연결의 `READY`가 지연되지 않았다.

## 회귀 확인

| 확인 | 결과 |
|---|---|
| V-8 raw TCP 계약 probe 및 v11 회귀 확인 | `RAW_TCP_REVIEW_SUMMARY=6/6` |
| peer 응답 | `RAW_PEER=PASS` |
| 고정·임의 포트 | `TCP_FIXED_PORT=10/10`, `TCP_RANDOM_PORT=10/10` |

위 시험은 localhost에서 실행했다. 무거운 시험 전 `ps`를 확인했고 `flock -o /tmp/fl-heavy.lock` 안에서 실행했다. 수정 전 재현과 수정 후 느린 송신 시험도 같은 lock으로 실행했다.

## Native bootstrap

strace 없이 native bootstrap을 실행했다. Stage2/3/4의 원문 SHA-256과 비교 결과를 아래에 기록한다.

```text
STAGE2_SHA256=6a0d4e384a18834d4f0cb03c10460aade5bf05f999f5db981dd5dc275d313e7d
STAGE3_SHA256=6a0d4e384a18834d4f0cb03c10460aade5bf05f999f5db981dd5dc275d313e7d
STAGE4_SHA256=6a0d4e384a18834d4f0cb03c10460aade5bf05f999f5db981dd5dc275d313e7d
CMP_2_3=0
CMP_3_4=0
STRACE=0
NATIVE_BOOTSTRAP=PASS
```

V-8 기록과 동일한 고정점이므로 `FIXPOINT_LOG.md`에는 변경 사유 항목을 추가하지 않았다.

언어 성과: 느린 송신자의 진행 대기·다중 연결 양보는 `runtime/raw_tcp.c` 실행 환경 변경이며, 시험은 `raw-tcp-deferred-boundaries.fl`이다. 이 확인에서 compiler 언어 결함은 발견하지 않았다.

V9_SUMMARY 5/5
