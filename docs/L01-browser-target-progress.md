# L01 J안: 최초 생성물의 실제 브라우저 검증

## 최신 제품 연결과 자체 검증

후속 `fa37289276bf9035e27c9497e94fa3a9387c1c8e`는 string?·typed focus·언어 공통 html-escape를 FreeLang 지원 소스에 제공한다. 정답을 먼저 고정했고 미지원 생성 실패와 focus 최초 표본 실패를 보존했다. 같은 생성물의 실제 입력은 정상이며 focus 표본은 자신의 page 활성화 전 `0`, 후 `1`이었다. C는 Element.focus를 대신 호출하지 않으며 실제 Input.insertText와 :focus로 검사한다. 소스 계약은 [editor-required-contract](../tests/l01/editor-required-contract.md)다.

최신 실제 Chrome 결과는 의미/HTML `37 PASS / 0 FAIL`, 입력·이벤트/focus `26 PASS / 0 FAIL`, DOM/URL `31 PASS / 0 FAIL`이다. 각 실행의 native 의미 정답·생성물 재현성·compiler 거부 `10/0`·종료 `0`을 확인했다. 원문은 l01-shared-html-semantics.txt, l01-shared-html-events.txt, l01-shared-html-dom.txt다. 기존 browser str의 atom 등 미지원 타입을 html-escape 전체 지원으로 주장하지 않는다.

같은 .fl 제품 모델은 native/browser 각각 `21 PASS / 0 FAIL`이며 실제 블로그 새 작성·사진·포인터/키보드·undo/redo·저장·기존 시각 편집기 재열기는 세 크기 합계 `48 PASS / 0 FAIL`이다. 독립 reader `3/0`에서 합성 3글·1이미지의 정확한 본문·상태와 WAL 불변을 확인했고 공개 draft 경로는 각각 HTTP `404`다. 기존 편집기 회귀는 `27/0`이다. [제품 소스·실패·원문·화면·미확인](https://github.com/kimjindol2025/blog1/blob/feat/blog-platform-completion/docs/block-editor-progress.md)에 구분했다. 새 화면은 생성 JS, 기존 화면 재열기/회귀는 기존 JS 무수정 실행이며 실제 비밀번호 로그인은 미실행이다.

초기 고정 `188ea729681f4a652769835fdb82f48986a14fd8`의 전체 native 회귀는 `L01_NATIVE_FULL_EXIT=0`, `L08_PLATFORM_CURRENT_GATES=PASS`다. L08 3차 11/0·2차 11/0·1차 17/0, L06 HTTP/TLS 각각 110/0, L07 221/0, 변경하지 않은 bootstrap 0이다. stage2·stage3 SHA-256은 모두 `6f7c143734a8492fb6ae9ab77036f619c41915d0181a6f263e95985cf90ad423`다. [전체 원문](evidence/l08-platform/l01-native-full.txt)을 보존했다. 이후 string?/focus 고정 `f6fc692759fbb3a44603518f777c5f1a3b470c32`의 새 전체 회귀는 진행 중이며 HTML 후속 변경의 위 자체 결과와 구분한다. 앞선 통과를 최신 코드 전체 통과로 대신하지 않는다.

모든 도구 execve 감사는 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다. Chrome 내부에서 생성 JS/기존 JS를 실행한 것과 별도 도구 미호출을 구분한다. JS 주입/Runtime.evaluate는 0이다. L01 전체·제품 1단계·전체 PRD를 완료 처리하지 않았다.

## 확인한 것

사용자가 결정한 J안에 따라 FreeLang에서 브라우저 JS를 생성했다. 제품 JS를 손으로 작성하거나 생성물에 패치하지 않았다. 구현 대상은 `fa05146fa670080bb39f2ce38a7250f3c2600f1e`이며 runtime/self 구현은 증거 보존 커밋 `188ea729681f4a652769835fdb82f48986a14fd8`까지 동일하다.

지원 로직은 self/browser-core.fl, 공통 in-memory AST printer는 runtime/browser-target.h, CLI는 self/cgc-main.fl이다. 고정 stage0는 변경하지 않았다. 지원 소스의 재현 가능한 C byte 배열은 refresh-browser-core.sh로 생성·대조한다. Node·외부 JS compiler·Python·fl-build.sh를 사용하지 않았다.

먼저 실패 정답을 고정했다. 옛 compiler가 알 수 없는 option 뒤 입력 파일을 출력으로 오인한 문제는 자신의 복사본에서 재현했다. 입력 해시 보존을 포함한 CLI 시험은 수정 전 `1 PASS / 3 FAIL`, 수정 후 `4 PASS / 0 FAIL`이다. 고정 seed에서 지원하지 않는 함수 이름을 사용한 빌드 실패와 parser registry 부재의 생성 실패도 숨기지 않고 원문을 남겼다. 안정된 API 이름과 명시적 parser callback으로 수정했으며 원래 seed나 정답은 바꾸지 않았다.

실행 명령은 의존 include/library와 검증한 compiler를 지정한 뒤 `strace -f -e trace=execve` 아래 `bash scripts/verify-l01-browser-target.sh`다. 자신의 loopback HTTP server·빈 포트·새 Chrome profile만 사용한다. C CDP 도구는 DOM과 Page를 통해 결과를 읽고 화면을 저장하며 Runtime.evaluate 또는 주입 JS를 사용하지 않는다.

```text
L01_PARSER_BUILD_EXIT=0
L01_PARSER_GENERATE_EXIT=0
L01_GENERATION_REPRODUCIBLE=PASS
L01_COMPILER_REJECT_PASS=10 FAIL=0
L01_NATIVE_ORACLE=PASS
L01_BROWSER_RUNTIME_EXIT=0
L01_BROWSER_CODE=COMPILER_GENERATED
L01_BROWSER_INJECTED_JS=0
L01_BROWSER_PROOF_EXIT=0
L01_BROWSER_SEMANTICS_PASS=29 FAIL=0
L01_TARGET_SELF=PASS
L01_OWN_browser_EXIT=0
L01_OWN_server_EXIT=0
L01_SELF_FIRST_EXIT=0
```

생성물 SHA-256은 `e87e83f0bd426757c57dca29d9403d166e2e60917e203d307ea24b87193f9d7a`다. 같은 입력의 두 생성물을 byte 비교했다. 한글·이모지 UTF-16 길이/부분 문자열, map 기본값·nil·0, 함수 재정의·클로저·swap 단일 호출, 10000회 loop/recur, 음의 0, __proto__ key와 try/finally 등을 native 동일 정답과 비교했다. 정답과 fixture는 tests/l01, 전체 원문은 [l01-browser-first-pass.txt](evidence/l08-platform/l01-browser-first-pass.txt), 768×1024 화면은 [l01-generated-proof-768.png](evidence/l08-platform/l01-generated-proof-768.png)다. parser 보완 뒤 빌드 원문은 l01-parser-callback-after.txt다.

도구 execve 감사는 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다. 이는 별도 JS 도구 미호출이며 Chrome 내부에서는 compiler가 생성한 JS를 실행했다. 기존 블로그 편집기의 JS는 별도 화면 시험에서 무수정 실행했으며 이 생성물 증거와 구분한다.

## J2 입력·이벤트 후속 구현

정답·실제 HTTP/Chrome fixture를 먼저 고정한 커밋은 `84e38182522500bdd2eb30c661b74fb943da6504`다. 첫 실행은 API 미지원으로 `L01_EVENTS_BEFORE_EXIT=1`이었다. 구현 `57d722c005d5eba068e9a6da13e9750d8628cc8b`는 텍스트/입력값 읽기·쓰기, 이벤트 등록/독립 해제, target/key·기본 동작 취소를 FreeLang 지원 소스로 제공한다. host C printer는 고정 DOM 타입 판정·속성·메서드 호출만 생성한다. token record는 private WeakMap에 보관한다. 새 외부 라이브러리를 사용하지 않았다.

고정 seed에서 compiler 빌드 `L01_EVENTS_COMPILER_BUILD_EXIT=0`, 실제 브라우저 `L01_BROWSER_EVENT_PASS=22 FAIL=0`, `L01_EVENTS_FIRST_EXIT=0`이다. 한글·이모지·HTML처럼 보이는 입력 원문과 실제 입력 1회, setter의 input 이벤트 0, 실제 Enter 키 기본 동작 취소, 동일 callback 독립 등록·해제·중복 해제, 인자/타입 거부와 HTML 자식 부재를 검증했다. C CDP로 DOM.focus·Input.insertText·Input.dispatchKeyEvent·포인터 클릭을 수행했다. 앱 로그인이나 운영 계정은 사용하지 않았다.

같은 compiler의 기존 의미 회귀도 실제 Chrome `L01_BROWSER_SEMANTICS_PASS=29 FAIL=0`, `L01_EVENTS_SEMANTICS_EXIT=0`이다. compiler 거부 `10 PASS / 0 FAIL`, 생성물 두 번 byte 일치도 두 시나리오 각각 확인했다. 원문은 l01-events-before.txt, l01-events-compiler-build.txt, l01-events-first-pass.txt, l01-events-semantics.txt이며 [입력·이벤트 화면](evidence/l08-platform/l01-events-proof-768.png)을 보관했다. 감사는 각각 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다. Chrome 내부에서 생성 JS를 실행한 사실과 구분한다.

첫 22개 뒤 서로 다른 Element/token이 구조적 equality로 같아지는 문제를 검토했다. 수정 전 `3fe9861c4a5b39bea0194754cd0aae113ffb448b`에서 두 정답을 추가했다. 실제 24개 중 Element/token identity 두 개가 FAIL이며 `L01_IDENTITY_BEFORE_EXIT=1`이다. [실패 원문](evidence/l08-platform/l01-identity-before.txt)을 먼저 보존했다. 수정 `49ff5c915561e7de36b5c4c27b3261c7991c5a47`는 일반 map/vector 구조 비교를 유지하고 host object/token만 참조로 구분한다. 고정 seed에서 다시 빌드한 수정본의 실제 Chrome 결과는 `L01_BROWSER_EVENT_PASS=24 FAIL=0`, `L01_IDENTITY_EVENTS_EXIT=0`이다. 기존 의미 회귀도 `L01_BROWSER_SEMANTICS_PASS=29 FAIL=0`, `L01_IDENTITY_SEMANTICS_EXIT=0`이다. 두 실행 모두 compiler 거부 10/0·재현성 PASS·브라우저 예외 0·종료 0이며 원문은 l01-identity-events.txt와 l01-identity-semantics.txt다. 이 고정 소스의 변경하지 않은 bootstrap도 `L01_IDENTITY_BOOTSTRAP_EXIT=0`, `STAGE2_EQ_STAGE3=PASS`다. stage2·stage3 SHA-256은 모두 `6f7c143734a8492fb6ae9ab77036f619c41915d0181a6f263e95985cf90ad423`, ARRAY_LOOP=PASS·RESIDUAL_PROCESSES=0이다. 원문은 l01-identity-bootstrap.txt다. 완료와 cleanup 뒤 정리된 임시 실행물을 가리킨 추가 식 진단은 시작 실패 `L01_BOOTSTRAP_FORMS_EXIT=127`로 보존했으며 본 bootstrap 실패로 집계하지 않는다. 통과 뒤 불필요한 추가 식 진단은 반복하지 않았다.

## J3 블록·사진 DOM 구성 후속 구현

고정 정답·계약 `d46accb4eb81a6f6b0833798fc74595770bcb6d9` 뒤 API 미지원의 `L01_DOM_BEFORE_EXIT=1`을 먼저 보존했다. 구현 `b63710cd6fffee38995e5f19a795dbcde22342fe`는 FreeLang 지원 소스에서 허용 tag/attribute와 URL 경계를 검사한다. 문단/사진 요소 생성, 이동/삽입/삭제/비우기, 자식 vector snapshot, 속성·hidden을 지원한다. innerHTML·eval API는 없다. private C host는 표준 DOM primitive와 URL parser 호출을 출력한다.

같은 고정 source에서 fixed stage0로 새 compiler를 빌드한 뒤 실제 Chrome을 세 번 실행했다.

```text
L01_DOM_COMPILER_BUILD_EXIT=0
L01_BROWSER_DOM_PASS=31 FAIL=0
L01_DOM_DOM_EXIT=0
L01_BROWSER_EVENT_PASS=24 FAIL=0
L01_DOM_EVENTS_EXIT=0
L01_BROWSER_SEMANTICS_PASS=29 FAIL=0
L01_DOM_SEMANTICS_EXIT=0
```

각 시나리오에서 compiler 거부 `10 PASS / 0 FAIL`, 생성물 두 번 byte 일치, 브라우저 예외/주입 JS `0`, 자신의 server/browser 종료 `0`이다. DOM 시험은 기존 요소의 동일성/순서, 자식 snapshot의 독립성, cycle/다른 부모의 before 오류, script/iframe/on*/style·위험한 URL·외부 이미지 origin 거부와 기존 속성 보존을 확인했다. 최종 DOM 구조와 literal 문자열의 HTML 미실행은 C CDP에서 별도로 검사했다. 이 DOM fixture의 이미지 검사는 URL 속성 계약이며 이미지 HTTP 200·decoding/처리 시험이 아니다.

DOM 생성 JS SHA-256은 `8c352d4255359d13b77c178064dc00a98f651318034c3790c61d6999d38aaba0`이다. 원문은 l01-dom-before.txt, l01-dom-compiler-build.txt, l01-dom-dom.txt, l01-dom-events.txt, l01-dom-semantics.txt다. [768×1024 언어 시험 화면](evidence/l08-platform/l01-dom-proof-768.png)은 제품 편집기 화면 인수와 구분한다. 감사는 각각 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이며 Chrome은 생성 JS를 실행했다.

처음 parser callback 구현부터 J2·identity·J3까지 고정 seed가 만든 compiler.c는 모두 SHA-256 `8a3b18e7347840bb9f0afbe31736abed9b5f2c629ede40e7c13b4aecd4cbd650`이다. 지원 소스/host backend는 바뀌었고 실행물 hash는 다르다. 이 C 소스 일치를 자기 컴파일 stage2=stage3 결과로 확대하지 않는다.

## 미확인

L01 전체와 PRD는 미완료다. 위 입력·이벤트와 제한된 DOM 구성을 제외한 선택 영역·fetch/취소·타이머·undo/redo·직렬화·자동저장·제품 편집기 전환은 아직 인수하지 않았다. 실제 IME composition·붙여넣기·모바일 가상 키보드와 전체 DOM/브라우저 호환은 이 작은 자체 시험으로 대신하지 않는다. 지원하지 않는 전체 표준 라이브러리를 호환으로 주장하지 않는다. 기존 --ast 숫자 debug JSON 오류도 미수정이다.

이전 runtime `cdad51e78deee1c3cde014fd106566db77159afc`와 초기 L01 고정 `188ea729681f4a652769835fdb82f48986a14fd8`의 전체 회귀·변경하지 않은 bootstrap은 각각 종료 `0`이다. 현재 새 전체 실행은 `f6fc692759fbb3a44603518f777c5f1a3b470c32`이며 그 이후 코드의 전체 결과로 확대하지 않는다. generation-only는 browser 미실행을 명시하고 위 별도 Chrome 검증과 구분한다.

서버 23·25·운영 자료·기존 backup에는 접근하거나 변경하지 않았다. 합치기·배포·강제 푸시는 하지 않았다. 실제 경로는 `<가림>`이다.
