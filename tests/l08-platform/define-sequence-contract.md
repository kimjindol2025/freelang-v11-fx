# define의 값 위치 생성 C 회귀

전체 고정 소스 `166ef374bb8417f7cd2227f23e3b98de47fe0f84` 회귀는 L07 `221`건과 runtime 시험 뒤 기존 jit-test.fl의 `when` 안 재정의가 `FL_VALUE(assignment;);`로 생성되어 GCC 문법 오류로 종료했다. 따라서 native bootstrap은 미실행이다. jit-test.fl이나 기존 기대값을 고쳐 감추지 않는다.

기존 define의 전역 재정의 계약은 유지한다. 복합 do·when 안에서도 C 표현식으로 생성해 연속 재정의 뒤 값 `3`·재귀 종료 `10`을 확인한다. 반환값 자체의 언어 계약을 이 시험에서 새로 정하지 않는다. 기존 jit-test·gc-stress 전체 회귀를 추가로 실행해야 한다. Node 실행 금지.

문법 수정 `b27af4fd4637455b3cc647e61fdda88259443970` 뒤 GCC는 통과했지만 첫 `(+ n 1)`이 nil을 읽었다. resolver가 같은 이름의 local cell을 두 번 미리 만들어 외부 n을 가렸기 때문이다. 추가 실패를 보존한다. 정답은 바꾸지 않는다.

v11 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4`의 src/eval-special-forms.ts:787~865는 define의 RHS를 먼저 평가하고 값을 저장한다. 같은 파일 :1249~1294의 do는 별도 scope를 push하지 않는다. src/eval-call-function.ts:54~76 propagateMutations는 변경된 캡처 이름을 capturedEnv와 savedStack으로 역전파하고 :530~549는 정상 함수 반환 전에 이를 호출한다. 따라서 캡처한 n=0에 +1·+2를 적용한 뒤 반환값과 외부 n이 모두 3인 원래 정답을 유지한다. Node로 실행하지 않았다. 함수 파라미터·기존 cell은 해당 바인딩을 재사용하고 같은 local 이름은 한 번만 선언해야 한다.
