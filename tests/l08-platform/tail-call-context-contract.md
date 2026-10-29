# C18 꼬리 호출 문맥과 C02 함수 호출 후속 정답

v11 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4` src/eval-special-forms.ts:1216~1238은 tcoMode에서 선택한 if branch가 사용자 함수 호출이면 evalSExpr로 다시 감싸지 않고 TailCall을 반환한다. src/eval-call-function.ts:403~467은 일반 함수 body를 tcoMode=true로 평가하며 TailCall의 다음 함수를 반복 실행한다. 따라서 if branch의 호출 이름은 오류 접두어에 추가되지 않는다. 이와 달리 일반 함수 본문의 직접 sexpr 호출은 evalSExpr 문맥을 유지한다.

let RHS는 eval-special-forms.ts:2238~2243의 evValue로, 호출 인자는 src/interpreter.ts:1932~1941에서 tcoMode=false로 평가한다. 함수 body의 let 뒤 if와 값 위치의 let/인자 안 if를 혼동하지 않는다. 정답은 이 소스에서 고정하며 Node를 실행하지 않는다. 상호 꼬리 호출과 깊이 5000의 오류 전달을 포함하여 접두어만 빼는 우회로 완료 판정하지 않는다.

db95 execute-ast는 if branch에서 execute-insert를 호출한다. native 일반 호출 문맥 보완 뒤 실제 값은 `[execute-insert] [engine-validate-row] PRIMARY_KEY_VIOLATION`이지만 원래 기대값은 `[engine-validate-row] PRIMARY_KEY_VIOLATION`이다. 이 추가 차이를 별도 언어 과제로 재현한다. vendor/앱/기존 기대값은 변경하지 않는다.
