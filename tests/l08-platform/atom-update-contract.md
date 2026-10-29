# C20 atom 原子的 갱신 시험 계약

v11 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4`의 src/eval-builtins.ts:526~534,952~960 소스를 읽었다. callback을 현재 값·extra 인자로 한 번 호출하고, 반환한 값을 저장·반환한다. callback이 같은 atom에 reset!/swap!을 호출해도 바깥 호출은 원래 읽은 값으로 계산한 결과를 최종 저장한다. callback이 throw하면 바깥 저장은 하지 않지만 callback 안에서 이미 수행한 reset!은 취소하지 않는다. Node로 실행하지 않았다.

동시성 기대값은 L08 C20 지시서의 유실 갱신 방지에서 고정한다. 실제 native 8-thread 각 100회 `+1`은 800이며, callback 호출 수도 800이어야 한다. retry로 사용자 callback을 여러 번 실행하거나 worker를 1개로 낮추지 않는다. 숫자와 한글·이모지를 포함한 map을 각각 갱신한다. 오류 뒤 다른 스레드에서 같은 atom을 갱신해 잠금 해제를 확인한다.

서로 다른 atom은 독립적으로 진행해야 한다. C 시험 setup이 이전 64개 잠금 버킷에 충돌하는 두 atom을 만들고, FreeLang 두 실제 작업에서 한 atom의 callback이 대기하는 동안 다른 atom을 갱신하도록 한다. 전역 잠금이나 버킷 잠금을 callback 전체에 걸어 독립 작업을 멈추는 해법을 거부한다. C helper는 barrier·스레드 수/콜백 횟수·짧은 pause·조건변수만 제공하고 실제 갱신과 단정은 `.fl`에서 수행한다.

callback이 여러 atom을 순환 대기하게 만드는 임의의 사용자 lock 의존성까지 자동 해소한다고 주장하지 않는다. 이 시험은 atom 원자성의 부분 증거다. HTTP 요청 수명·동시 정렬·DB 거래·WAL 일관성·메모리 누수는 별도 검증하며 C20 전체 완료를 이 시험만으로 선언하지 않는다.
