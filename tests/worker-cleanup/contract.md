# E07 worker 종료 정답 — 구현 전 고정

cancelled: 기존 fl_schedule_shutdown으로 취소한 예약이 worker 종료 뒤에도
cancelled로 남고 callback은 호출되지 않는다. pending: 아직 울리지 않은
worker 예약은 worker 종료 때 취소한다. 다른 owner인 main 예약은 유지한다.
active: callback 안에서 pthread_exit로 worker가 종료돼도 실행 중 예약과
그 worker의 대기 예약 모두 취소 상태가 되고, dispatch 잠금과 소유 참조는
해제된다. main의 새 예약은 실행된다. capture 문자열은 한글·이모지를 보존한다.

세 .fl 시험은 실제 set-timeout/await-result를 호출한다. C 보조 코드는
pthread 시작·종료, arena 사용, 독립 관측만 제공한다. worker가 정상 반환하는
경우에도 명시적 arena 파괴 호출을 넣지 않는다. 요청 사이 cache 재사용은
기존 계약대로 유지한다. cancelled 모드의 명시적 전역 shutdown은 이미
취소된 상태 재현 전용이며 cleanup 구현이 전역 shutdown을 쓰는 우회는
pending/active 모드의 main 예약 유지 정답으로 거부한다.

모든 .expected와 종료 0, LSAN 누수 0 byte/0 allocation은 구현 전에 고정한다.
기존 262464 byte(s)/8 allocation(s) 사례도 같은 ASAN ELF로 수정 전 1회만
재현한다. 정답·문턱·suppressions 변경은 금지한다. LSAN은 ptrace 밖,
빌드 감사는 strace로 기록한다. 새 실행 RUN-20261101-02에서 before는
증거만 보존하고, 수정 후 같은 점검의 세 번째 실패면 즉시 중단한다.

추가 정답: 명시적 arena 이전의 임시 문자열·closure도 thread 종료 때
회수하되 `fl_arena_active`의 의미와 요청 reset 사이 startup 값 수명은
유지한다. `unscoped.c/.expected`는 그 호환을 검증한다. 기존 serialization
사례에서 관측한 `800 byte(s)/20 allocation(s)` 실패를 추가 수정의 기준으로
보존하며 기존 시험을 고쳐 누수를 없애지 않는다.
