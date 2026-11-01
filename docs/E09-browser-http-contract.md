# E09 브라우저 비동기 HTTP 고정 계약

RUN-20261101-04. 구현 전에 20개 정답과 fixture를 커밋한다.
`browser-http!`는 URL 문자열, 옵션 map, callback 함수 세 인자를 받는다.
옵션은 method(GET 또는 POST), body(문자열; GET은 빈 문자열),
timeout(정수 1..2147483647 ms)을 모두 명시한다. URL은 상대 URL 또는
HTTP(S)다. 잘못된 인자·URL은 요청 전에 동기 오류로 드러난다.

요청 시작은 nil을 반환하고 callback은 비동기로 정확히 한 번 호출한다.
결과 map은 ok, status, body, error를 가진다. 2xx는 ok=true/error=nil,
4xx/5xx는 ok=false/error="http"이며 status·body를 보존한다.
연결 또는 본문 수신 실패는 status=0/body=""/error="network",
시간초과는 status=0/body=""/error="timeout"이다. 시간초과는 본문 수신까지
포함하고 실제 요청을 abort한다. callback 예외는 브라우저에 드러나며
HTTP 실패로 재호출하거나 조용히 소비하지 않는다.

`browser-json-parse`는 문자열 한 인자를 받고 JSON 값을 반환한다.
JSON null과 파싱 오류를 구분한다. 잘못된 JSON·타입·인자 수는 오류다.
검증·상태·완료 중복 방지·타이머 정리는 .fl로 구현한다. C는 표준 host
primitive 연결만 추가한다. 생성 JS를 손으로 수정하지 않는다.

20개 고정 정답은 browser-http.expected 순서다. 인자 오류 8개, 성공·비동기·
JSON object/array/null·404·500·연결 실패·시간초과·POST·빈 204 11개,
시간초과 뒤 중복 완료 없음과 JSON 오류 노출 1개다. 연결 실패는 브라우저가
차단하는 loopback 포트를 사용하며 외부 서비스나 서버 23·25를 사용하지 않는다.
실제 Chrome의 DOM 원문을 정답과 비교하고 화면을 저장한다.
기존 문턱값·seed·시험은 변경하지 않는다. 수정 전 실패는 증거이며,
수정 후 같은 점검의 세 번째 실패에는 중단한다.
