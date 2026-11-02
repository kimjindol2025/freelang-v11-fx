# C20 atom 소유권·순환 수명 계약

v11 atom은 src/eval-builtins.ts에서 `{ value: ... }`로 만들고 FreeLang 사용자에게 명시적인 free를 요구하지 않는다. v11 기준 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4` 소스를 읽었으며 Node 실행으로 검증하지 않았다. C20은 요청/반복 할당 수명과 누수 검증을 요구한다. 참조가 사라진 임시 atom, self/two-atom/closure cycle은 수거돼야 하며 반환·캡처·컨테이너·외부 저장소로 도달하는 atom은 동일한 가변 참조로 유지돼야 한다.

시험은 실제 `.fl`의 atom을 생성하며 native link wrapper로 fl_atom_new의 반환 pointer와 실제 free를 관찰한다. 계측을 시작한 뒤 1000개 임시 atom, 반환값, 캡처 closure, 컨테이너의 같은 atom 별칭, 오류, 8 실제 thread ×250 scope, 순환 참조를 검사한다. wrapper는 수거를 대신하지 않으며 counter나 기대값을 낮추지 않는다. C helper의 arena scope는 callback의 결과/예외를 소유한 heap 값으로 복사한 뒤 이전 arena로 복원한다. 수정 전 고정 runtime에도 같은 시험을 적용한다.

C API는 기존 fn/promise의 arena 소유권처럼 활성 arena 안의 생성값은 해당 arena가 소유하고, C 호출자가 arena 밖에 보관하면 fl_heap_copy/fl_heap_release로 명시적인 강한 참조를 보유하는 방향이다. FreeLang 소스에는 이 관리를 요구하지 않는다. 기존 snapshot-memory C fixture의 shared root는 이 규칙에 맞게 소유권만 명시할 수 있으며 4-reader×1000회·이전 snapshot·예외·출력 정답은 그대로 유지한다. 변경 이유와 전체 회귀를 보고한다.

모든 native 객체·future/promise cycle·프로세스 전역 수명·모든 OOM 경로·전체 누수 해결은 이 atom fixture만으로 완료 판정하지 않는다. 운영 자료·원본 백업은 시험에 쓰지 않는다.
