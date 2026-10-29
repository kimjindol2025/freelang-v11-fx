# define의 값 위치 생성 C 회귀

전체 고정 소스 `166ef374bb8417f7cd2227f23e3b98de47fe0f84` 회귀는 L07 `221`건과 runtime 시험 뒤 기존 jit-test.fl의 `when` 안 재정의가 `FL_VALUE(assignment;);`로 생성되어 GCC 문법 오류로 종료했다. 따라서 native bootstrap은 미실행이다. jit-test.fl이나 기존 기대값을 고쳐 감추지 않는다.

기존 define의 전역 재정의 계약은 유지한다. 복합 do·when 안에서도 C 표현식으로 생성해 연속 재정의 뒤 값 `3`·재귀 종료 `10`을 확인한다. 반환값 자체의 언어 계약을 이 시험에서 새로 정하지 않는다. 기존 jit-test·gc-stress 전체 회귀를 추가로 실행해야 한다. Node 실행 금지.
