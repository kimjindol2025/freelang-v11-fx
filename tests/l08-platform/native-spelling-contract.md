# 기존 native C API의 source 이름

fx 자체 compiler의 cgc-context-call은 to-string을 호출한다. C 구현 to_string과 runtime/runtime.h 선언이 있지만 source의 hyphen 이름이 native metadata에서 빠져 있어 stage2 compiler가 가장 작은 `(println 1)` 입력도 `Function not found: to-string`으로 실패했다. stage1의 동일 입력은 성공했다. bootstrap의 큰 자기 입력은 아직 실제 실행 중이며 이 작은 probe 결과로 큰 과정의 종료를 주장하지 않는다.

원래 native compiler가 c-name으로 hyphen을 underscore로 변환해 연결하던 FLValue C API의 source 이름도 선언된 native namespace에 포함해야 한다. 임의 미정의 함수의 lookup·extern 검증을 없애지 않는다. 선언 이름 자체를 유지하고 FLValue 함수 선언에서 생성한 source 별칭만 추가한다. 사용자 정확한 이름 정의·v11 builtins·기존 함수 별칭 회귀도 확인해야 한다.

기존 to_string(42)의 문자열 `42`를 최소 기대값으로 먼저 고정한다. 기존 JIT 네 함수도 같은 원인의 source 이름이며 명시적 중복 dispatch 대신 공통 metadata에서 처리할 수 있다. 실제 stage2의 작은 입력과 unchanged verify-native-bootstrap.sh의 stage2=stage3를 모두 다시 확인해야 완료다.
