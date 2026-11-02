# C20 atom snapshot 최소 정답

v11 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4` bootstrap.js의 atom/deref/reset!은 value field를 읽고 교체하며 이전 객체를 직접 free하지 않는다. 지역 변수·반환값이 참조하는 이전 map/vector/string은 reset 뒤에도 읽을 수 있다. source 기준으로 atom-snapshot.expected를 먼저 고정한다. Node를 실행하지 않는다.

실제 blog1 stage1-write.fl은 수정하지 않은 채 native에서 139로 종료됐으며 ASan은 fl_atom_reset의 이전 root 해제 이후 fl_eq의 freed key 접근을 확인했다. 예상 출력을 구현에 맞추거나 WAL 거래를 삭제하지 않는다. 언어 runtime에서 읽은 값의 수명을 보존하고, 함수 반환·catch/longjmp·요청 arena 종료에서 보호를 해제하는 방향으로 고친다. 동시 read/reset도 freed root를 읽지 않도록 해야 한다. 이것만으로 swap!의 원자성·SQL 동시 거래·누수 전체를 완료 판정하지 않는다.
