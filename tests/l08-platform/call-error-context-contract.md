# C18 연산 호출 오류 문맥 추가 재현

v11 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4`의 src/interpreter.ts:1991~2018와 bootstrap.js:40912~40931은 evalBuiltin 호출이 던진 Error의 message에 `[op]`를 붙이고 최초 `(at line ...)`만 보존한다. 이미 `[op]` 또는 `(op)`로 시작하면 같은 접두어를 다시 붙이지 않는다. 인자는 이 try 전에 평가하므로 인자 오류에는 아직 호출하지 않은 바깥 연산 문맥을 붙이지 않는다. evalTryBlock은 첫 대괄호 접두어와 위치를 제거한다. Node를 실행하지 않고 이 소스 계약으로 정답을 먼저 고정한다.

따라서 leaf에서 throw한 boom을 outer→leaf로 호출한 뒤 바깥에서 catch하면 `[outer] [leaf] boom`에서 `[outer]`만 제거되어 `[leaf] boom`이다. 직접 leaf를 catch하면 boom이다. 실제 db95 engine 자체 시험도 이 계약을 기대한다. vendor 기대값·오류 문자열을 지워 통과시키지 않는다.

기존 fx는 fl_v11_throw/catch 자체 포장은 구현했지만 호출 연산 문맥은 빠져 있었다. 새 최소 시험은 직접·중첩·기존 접두어 중복·소괄호 접두어·인자 평가·내부 catch·catch 후 재호출을 확인한다. 함수 이름을 C symbol로 바꾼 이름이 아니라 원래 연산 이름으로 보존해야 한다. 실제 저장소 전체 fixture의 성공은 이 최소 시험과 별도로 확인한다.
