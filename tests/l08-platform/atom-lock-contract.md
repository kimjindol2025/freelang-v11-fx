# 명시적 복합 작업 잠금 계약

`with-atom-lock guard callback`은 fx에 추가하는 언어 기능이다. v11에 존재한다고 보고하지 않는다. 실제 DB 동시 쓰기에서 성공 32건 중 live 20건·재생 32건의 불일치를 재현한 뒤 필요성을 확인했다. atom 값을 변경하는 swap!을 잠금 우회로 쓰지 않는다.

- 실제 atom과 인자 없는 함수만 받는다. vector·nil·함수 아닌 값을 거부한다.
- guard와 callback 식은 왼쪽부터 각각 한 번 평가한다. 잠금 획득 뒤 callback을 한 번 실행하고 값을 반환한다. guard 값은 자동으로 바꾸지 않는다.
- 같은 guard의 재진입과 callback 내부 reset!/swap!을 허용한다. 예외를 전달하고 반드시 잠금을 해제한다. callback의 이미 일어난 부수 효과는 취소하지 않는다.
- 다른 guard는 서로의 callback 완료를 기다리지 않는다. 전역 callback 잠금 금지.
- 8개의 실제 native worker가 각각 100번 읽기·멈춤·쓰기를 수행해 정확히 800이 되어야 한다. callback 계측도 800이어야 한다.
- arena 소유권을 유지하며 실행 중 guard가 수거되지 않도록 보호한다. 일반 LSan과 전체 DB 장애 원자성은 이 자체 시험으로 완료 판정하지 않는다.

실제 DB 경계의 읽기·commit 전체에 이 기능을 적용한 뒤 별도 store-concurrent 32/live·32/replay 정답과 HTTP 통합을 확인해야 한다. 본 시험 통과로 DB 전체 완료를 대신하지 않는다.
