# C01 실행 시 호출 이름 조회 최소 정답

v11 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4`의 src/eval-special-forms.ts defn 분기는 body를 평가하지 않고 context.functions에 함수 정의를 등록한다. src/interpreter.ts:1932~1992는 실제 호출 시 인자를 평가한 뒤 evalBuiltin으로, src/eval-builtins.ts:7413~7414는 callUser로 전달한다. src/eval-call-function.ts:256~261은 이름을 찾지 못할 때 FunctionNotFoundError를 던진다. src/errors.ts:126~139의 고정 message 시작은 `Function not found: functionName`이며 후보에 따른 추가 hint는 변할 수 있다. Node를 실행하지 않고 소스로 정답을 고정한다.

따라서 호출하지 않은 함수의 body가 아직 없는 helper를 참조해도 정의는 성공해야 한다. 실제로 호출하면 오류를 catch할 수 있어야 한다. 이 최소 시험은 정의·중첩 호출 실패·직접 호출 실패·catch 뒤 계속 실행을 확인한다. hint 전체를 고정하거나 C 컴파일러의 implicit declaration 실패를 언어의 호출 오류로 인정하지 않는다.

원래 blog1 review/AI fixture는 일부 관리자·이미지 모듈을 load하지 않은 상태에서 해당 helper를 참조하는 route 함수를 정의한다. 현재 native compiler는 이 body를 바로 미선언 C 함수 호출로 만들어 구문 검사부터 실패한다. 부족한 기능은 언어의 실행 시 조회로 보완하며, 앱 fixture에 필요 없는 모듈을 추가 load하거나 함수 stub을 넣어 통과시키지 않는다. 알려진 native C symbol·v11 builtin·lexical binding·이미 아는 user function의 구분도 유지해야 한다.

이 기능이 전체 앱의 C 컴파일을 허용하더라도 실제 auth_verify_password 구현·로그인 검증을 대체하지 않는다. 빈 scrypt key 정책 결정은 별개이며, 미구현 호출은 실제 실행 시험에서 실패로 보고한다.
