# 기존 native JIT 함수 연결 회귀

원래 jit-test.fl·gc-stress-test.fl·tests/run-tests.sh와 기대값은 변경하지 않는다. define 문법을 고친 뒤 실제 기존 회귀는 `0 PASS / 10 FAIL / 10 총`이었다. `jit-test-add1`, `jit-test-mul2`, `jit-test-add-xy`, `jit-stats`의 C 구현은 runtime/jit.c:381~425, 선언은 runtime/runtime.h:255~258에 있지만 source의 hyphen 이름이 native metadata에 없어 사용자 함수 조회로 잘못 처리됐다. Node로 우회하지 않는다.

self/cgc-main.fl에 이 기존 네 source API의 명시적 C dispatch를 연결하고 metadata를 재생성한다. 모든 미정의 이름을 native라고 처리하거나 사용자 함수 lookup·extern 검증을 없애지 않는다. 수정 뒤 원래 runner의 10개 시험이 그대로 통과해야 한다. 별도 native bootstrap과 전체 L08 회귀를 대체하지 않는다.
