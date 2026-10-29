# C20 실제 blog 저장소 동시 쓰기 최소 계약

source-only 승인 복사본의 원래 app/store.fl·vendor 구현을 그대로 load한다. 8 실제 native worker가 각각 서로 다른 글 4건을 commit한다. 모든 worker 성공 8, live rows/고유 id 각각 32, 새 과정의 원래 복구 뒤 rows/고유 id 각각 32가 정답이다. reader가 WAL을 변경하지 않아야 한다.

첫 barrier는 commit 밖에서 8개 실제 worker의 동시 시작을 확인한다. C helper는 해당 catalog의 deref 직후 0.5 ms pause만 추가해 stale snapshot 경합을 드러낸다. 읽기 안에 barrier를 놓지 않아 올바른 transaction 직렬화를 deadlock으로 오판하지 않는다. 실제 commit·단정·복구는 `.fl`과 원래 blog/db95 코드에서 수행한다. worker 1개로 낮추거나 WAL 거래를 제거하지 않는다.

atom의 개별 swap! 원자성만으로 DB의 read/validate/catalog/index/WAL 전체 구간이 보호된다고 판정하지 않는다. 부족하면 명시적인 언어 동기화 primitive를 자체 시험과 함께 추가하고 저장소 경계에서 사용한다. 임의 DB 함수명을 C runtime에 하드코딩하거나 오류를 무시하는 우회는 하지 않는다. 원래 app/vendor를 몰래 수정하지 않는다.

시험은 별도 새 WAL과 정해진 source symlink만 사용한다. 운영 자료·설정·계정·다른 process/server는 사용하지 않는다. writer가 실패해도 reader를 실행해 live 상태와 재생 상태를 비교하며, 두 종료 코드와 WAL 무변경을 따로 기록한다. 이 최소 32건을 PRD 1000글/이미지·HTTP 부하·모든 실패 원자성 완료로 확대하지 않는다.
