# C19 타이머 구현 전 고정 계약

기준 v11 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4`의 bootstrap.js set-timeout 분기(17541), src/async-runtime.ts의 FreeLangPromise, src/stdlib-async.ts의 await_result를 텍스트로 조사했다. Node 실행은 하지 않는다. TCP·TLS는 같은 C19의 남은 작업이며 이 타이머 시험으로 완료 판정하지 않는다.

- set-timeout은 callback/delay를 평가한 뒤 즉시 pending promise를 반환하고 callback은 나중에 한 번 실행한다. Node timer의 범위에 따라 Number delay를 밀리초로 변환하며 NaN/음수/0/2147483647 초과는 1 ms, 양의 소수는 버린다. 순서는 due time·등록 순서다.
- await_result는 pending이면 source의 고정 오류, resolved면 여러 번 같은 값, rejected면 저장한 오류를 throw한다. callback 아닌 값은 타이머 실행 시 rejected가 된다. 작업 실패를 성공 nil로 바꾸지 않는다.
- CLI가 정상 반환해도 등록한 타이머는 실행된다. 명시적 process-exit은 기다리지 않고 취소하며 종료 코드는 그대로다. source에 독립 clear-timeout 계약이 확인되지 않아 취소 public API를 임의로 만들지 않는다. HTTP 종료 시에도 pending 작업을 취소해 종료를 막지 않는다.
- native는 HTTP listener의 poll에서 visitor 없이 timer를 진행하고, fl-yield에서 협력적으로 진행할 수 있도록 설계한다. fl-yield의 반환 개수는 TCP I/O 이벤트 수이며 timer 수를 더하지 않는다. TCP 구현 전에는 I/O가 없으므로 0이다. Node의 worker 내부를 재현했다고 주장하지 않는다.
- 요청 종료 뒤 실행될 callback은 캡처한 local 문자열·함수·atom의 수명을 보존하며 atom identity를 복제하지 않는다. promise/result는 사용 중인 참조가 보존하며 완료된 작업을 전역 표에 영구 누적하지 않는다. callback을 실행할 때 호출자의 요청 아레나를 초기화하지 않는다.
- CLI 기대 출력은 timer.expected로 먼저 고정했다. HTTP는 POST 뒤 churn 32회로 request arena를 재사용하고, HTTP 조회 없이 fired.txt가 생성되는지 확인한다. 원문은 captured-한😀, result는 해당 값 1개이며 이미 실행한 작업을 재실행하지 않는다. 4 workers, 소유 loopback port 0만 사용한다.

기존 GC에서 closure는 malloc이지만 환경 값은 arena pointer를 그대로 보관하고, fl_heap_copy(FN)는 RC만 올린다. timer를 연결하기 전에 이를 숨기지 않고 escape 시 환경 승격·arena 종료 정리·atom identity를 언어 런타임에서 보완해야 한다. 앱 함수를 삭제하거나 방문 기반 scheduler로 우회하지 않는다. `.fl` 시험·원문·종료 코드·strace와 이전 전체 회귀 및 bootstrap을 별도 증거로 제출한다.
