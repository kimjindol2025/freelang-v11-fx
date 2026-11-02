# J2 입력·이벤트 계약과 고정 정답

제품 편집기의 입력/자동저장 기반을 위한 언어 자체 시험이다. 새 구현 전에 fixture·정답을 커밋한다. 기존 native/v11 API를 덮어쓰지 않는 browser 전용 API다.

| API | 계약 |
|---|---|
| browser-text(element) | 요소의 textContent 문자열. HTML 실행 없음 |
| browser-value(element), browser-set-value!(element,string) | input/textarea만 허용. Unicode 원문 보존. setter는 nil, 자체 input 이벤트를 만들지 않음 |
| browser-on!(element,event,function) | 정확히 3인자, 별도 listener를 등록하고 해제용 token 반환. 같은 함수 중복 등록도 독립적 |
| browser-off!(token) | 자신의 token만 허용. 중복 해제는 nil이며 이후 callback 없음 |
| browser-event-target(event) | 실제 Event만 허용. target이 Element면 해당 요소, 아니면 nil |
| browser-event-key(event) | 실제 Event만 허용. KeyboardEvent의 key 문자열, 다른 Event는 nil |
| browser-prevent-default!(event) | 실제 Event만 허용. preventDefault 호출 후 nil |

event 허용 목록은 input, change, click, keydown, beforeinput, compositionstart, compositionend, paste, focus, blur다. DOM 이벤트 wrapper는 FreeLang으로 작성하며 host는 타입 판정·표준 DOM 메서드/속성 primitive만 제공한다. 잘못된 인자 수/타입/token/이벤트 이름은 throw, side effect 없음이다. token은 private WeakMap에 연결하며 사용자가 내부 listener record에 접근하지 못한다. 브라우저 callback 예외를 숨기지 않는다.

실제 loopback HTTP와 Chrome에서 C CDP의 DOM.focus·Input.insertText·Input.dispatchKeyEvent·포인터 click을 사용한다. Runtime.evaluate·합성 JS dispatchEvent·Node·수작업 JS 없이 최초 22개 고정 정답을 비교한다. 한글·이모지·HTML처럼 보이는 문자열 무손실, 텍스트의 자식 HTML 부재, setter 이벤트 0, 실제 입력 1, target/key, Enter 기본 동작 취소, 중복 callback 독립 등록·해제·중복 해제와 경계 거부를 포함한다. 종료 코드는 0, 브라우저 예외 0이 정답이다. native 29개 의미 시험과 compiler 거부 10개는 회귀로 유지한다. 전체 편집기·자동저장·L01 완료를 뜻하지 않는다.

첫 22개 통과 뒤 DOM/token을 plain map/vector와 구분하지 않는 equality 위험을 검토했다. 새 정답 2개를 수정 전에 추가해 총 24개로 고정한다. 같은 Element/token만 동일하며 서로 다른 Element/token은 같지 않아야 한다. plain map/vector의 기존 구조적 equality는 유지한다. 사용자에게 노출한 host object는 identity로 비교하고 private listener record는 여전히 접근할 수 없어야 한다. 이 추가 정답의 실패를 먼저 남기고 수정한다.
