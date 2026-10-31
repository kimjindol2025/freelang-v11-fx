# L01 J안: 최초 생성물의 실제 브라우저 검증

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

## 미확인

L01 전체와 PRD는 미완료다. 이벤트·입력·선택 영역·fetch/취소·타이머·undo/redo·직렬화·자동저장·제품 편집기 전환은 아직 인수하지 않았다. 지원하지 않는 전체 표준 라이브러리를 호환으로 주장하지 않는다. 기존 --ast 숫자 debug JSON 오류도 미수정이다.

이전 runtime `cdad51e78deee1c3cde014fd106566db77159afc`의 전체 회귀와 변경하지 않은 bootstrap은 종료 `0`으로 확인했다. 이후 L01 compiler 변경과 generation-only 관문을 포함한 고정 `188ea729681f4a652769835fdb82f48986a14fd8`의 새 전체 실행은 진행 중이며 앞선 통과로 대신하지 않는다. generation-only는 browser 미실행을 명시한다.

서버 23·25·운영 자료·기존 backup에는 접근하거나 변경하지 않았다. 합치기·배포·강제 푸시는 하지 않았다. 실제 경로는 `<가림>`이다.
