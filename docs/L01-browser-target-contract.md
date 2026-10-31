# L01 J안: 브라우저 출력의 첫 구현 계약

사용자가 확정한 J안과 PRD v0.6 완성 지시를 따른다. FreeLang compiler가 .fl AST/IR에서 browser JS를 생성한다. 생성기/지원 코드는 .fl·C, 제품·언어 지원 로직은 .fl이다. JS 파일·문자열·고정 템플릿에 사람이 작성한 제품 JS를 숨기거나 생성 결과를 손으로 보정하지 않는다. 원래 native target·고정 stage0·기대값을 보존한다.

## CLI와 경계

제안 CLI는 `compiler --browser-js input.fl output.js`다. 입력 load 확장과 기존 parser를 재사용하며 별도 JS parser나 Node를 도입하지 않는다. 출력은 생성물이며 compiler/source/runtime/options와 SHA-256을 기록한다. 지원하지 않는 IR·잘못된 인자·알 수 없는 host intrinsic은 명확하게 실패한다. native C·raw JS·eval로 우회하지 않는다. 실패를 정상 생성물로 표시하지 않는다.

브라우저 host API의 첫 계약은 browser-by-id(존재하지 않으면 nil)와 browser-set-text!(요소,문자열)다. textContent를 사용하며 문자열을 HTML로 실행하지 않는다. DOM node 없는 경우·잘못된 타입은 실패한다. 후속 작업에 DOM 생성/속성·이벤트·선택 영역·타이머·fetch/취소·편집 이력을 추가하고 각각 .fl 자체 시험을 둔다. 두 API만으로 L01 전체 완료를 판정하지 않는다.

## 정답 먼저 고정

tests/l01/browser-semantics.fl의 같은 함수가 native와 browser에서 같은 28개 정답을 내야 한다. 최초 24개에 native 소스 근거로 숫자 타입 거부·빈 뺄셈 거부·음의 0·배열 append 네 항목을 추가했다. arithmetic/소수·숫자 map key·get 기본값과 저장된 nil/false/0·UTF-16 글자 단위·escape·조건/논리·클로저·함수 값/재정의·atom/swap·인자 평가 순서·10000회 loop/recur·배열 callback·__proto__ key·try/finally를 포함한다. 정답은 기존 v11 호환 계약과 native 자체 시험에 맞추며 구현 결과에 맞춰 낮추지 않는다. 불일치가 있으면 원문 근거와 정정 이력을 먼저 남긴다.

native wrapper와 browser wrapper만 입출력이 다르다. browser는 .fl에서 proof 요소의 text를 설정하고 실제 Chrome에서 DOM 결과를 고정 정답과 비교한다. C 기반 CDP는 DOM/CSS/Input/Page만 사용하며 주입 JS/Runtime.evaluate·Node·Python은 사용하지 않는다. Native toolchain의 JS 미호출과 브라우저에서 compiler 생성 JS 실행을 별도 보고한다.

순서: fixture/정답 커밋 → native oracle 검증·원래 compiler의 browser target 미지원 실패 보존 → 구현 → 생성물 재현성·실제 Chrome·오류/경계 자체 시험 → native 회귀/변경하지 않은 bootstrap → 제품 .fl frontend 적용과 화면 인수.

## 아직 전체 완료가 아닌 항목

이 첫 fixture 외 언어 의미·동적 타입/오류·async·선택 영역·입력/붙여넣기·undo/redo·저장/재접속·브라우저 호환/출력 크기·성능과 모든 제품 흐름은 별도 증거가 필요하다. 기존 편집기 JS의 무수정 실행은 보존할 수 있지만 J안의 증거로 바꾸지 않는다. 프레임워크·추가 라이브러리 설치·합치기·배포·운영 자료 변경은 이 계약으로 승인되지 않는다.

## 구현 중인 지원 코드의 경계

self/browser-core.fl은 FreeLang으로 작성한 browser 지원 로직의 초안이다. 아직 출력 backend/CLI에 연결하지 않았으며 browser에서 실행·검증하지 않았다. delimiter와 기존 parser로 소스를 확인했지만 이것을 target 통과로 집계하지 않는다. jcore-/j-host- 이름은 compiler 내부용으로 설계하며 실제 backend에서 사용자 모듈 접근을 거부해야 한다. 일부 타입·API는 명시적 미지원 오류이며 전체 언어 호환으로 보고하지 않는다.

native/debug --ast의 숫자 출력에 fl_int(...) C 표현이 들어가 JSON이 되지 않는 기존 문제도 확인했다. browser target은 이 텍스트를 JSON으로 재해석하는 우회를 쓰지 않고 기존 in-memory AST를 직접 재사용한다. debug export 문제를 고쳤다고 보고하지 않는다.
