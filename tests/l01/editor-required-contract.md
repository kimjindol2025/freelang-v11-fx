# 제품 블록 편집기에 필요한 기존 의미와 focus

blog1 web/block-model.fl은 문자열만 받는 계약을 검사한다. 기존 native/v11 string?의 문자열 true·숫자/nil/vector/map/atom false를 같은 .fl 정답으로 browser target에도 연결한다. J_STRING_PREDICATE를 기존 29개에 추가해 30개로 고정한다. JS coercion이나 앱 쪽 우회로 타입 검사를 바꾸지 않는다.

browser-focus!(element)는 정확히 한 Element를 받아 focus 후 nil을 반환한다. 잘못된 인자는 focus 이전 throw다. 블록 이동/삭제/undo 뒤 키보드 작업을 이어가기 위해 필요하다. 기존 입력·이벤트 fixture가 생성 FreeLang 코드로 textarea를 focus하고 C CDP는 focus를 대신 수행하지 않는다. C는 :focus와 실제 Input.insertText로 목적 요소를 독립 검사한다. 기존 24개에 잘못된 대상 거부·실제 focus 두 개를 추가해 26개로 고정한다. 실제 .fl 자체 시험 이후에만 언어 부분 통과로 판정하며 블로그 시험으로 대신하지 않는다.

HTML escape도 앱의 대체 API에 남기지 않고 기존 언어 html-escape를 browser target에 연결한다. v11 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4` src/eval-builtins.ts:3010~3013은 nil을 빈 문자열로 바꾸고 String 변환 뒤 &, <, >, quote, apostrophe 순서로 escape한다. apostrophe 정답은 `&#x27;`다. 기존 native L08 2차 unicode 정답을 보존하며 문자열/기존 entity·nil·0·false·array·map 7개를 먼저 추가해 총 37개로 고정한다. Atom 등 기존 browser str 미지원 타입은 전체 지원으로 주장하지 않는다. Native 코드 경로의 이전 byte API를 기준으로 정답을 바꾸지 않는다. HTML 문자열을 DOM에 실행하는 API는 추가하지 않는다.
