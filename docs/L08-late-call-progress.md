# C01 실행 시 함수 이름 조회와 전체 앱 native 실행

## 확인한 것

사용하지 않은 함수 body의 미정의 호출을 C 미선언 함수로 출력하는 실패를 `.fl`·정답·[소스 계약](../tests/l08-platform/late-call-contract.md)부터 고정했다. `LATE_CALL_BEFORE_C_SYNTAX_EXIT=1`이다. [원문](evidence/l08-platform/late-call-before.txt).

native compiler의 binding pass는 lexical binding·v11 builtin·알려진 C symbol을 우선 보존하고 그 밖의 호출은 실행 시 named function registry에서 찾도록 표시한다. 실제 호출 때도 없으면 Function not found 오류를 catch할 수 있다. 함수 body를 없애거나 callback stub을 넣지 않았다. 같은 최소 4개 출력이 native와 ASan/UBSan에서 통과했다. `LATE_CALL_SANITIZER_EXIT=0`, `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`. [원문](evidence/l08-platform/late-call-after.txt).

원래 blog fixture 8개는 보완 후 모두 C 구문 검사 종료 `0`이다. [요약](evidence/l08-platform/blog-fixture-syntax-after.txt). 이것만으로 8개 fixture 링크·실행을 모두 통과했다고 주장하지 않는다.

## 원래 전체 blog와 편집기 HTTP

앱·vendor를 변경하지 않은 source-only 복사본에서 전체 main.fl, 원래 b1-editor-seed.fl·editor-http.fl을 native C로 변환하고 링크했다. 세 과정 모두 `NATIVE_BUILD_EXIT=0`이다. 앱은 test-only bind wrapper로 loopback의 port 0에만 bind한다. 남의 과정·포트는 끄지 않는다. 합성 CSRF·session 값은 Bash 안에서 생성하며 execve 인자·보고서에 출력하지 않는다. 운영 설정·데이터는 사용하지 않는다.

첫 실행은 driver가 TMPDIR을 빠뜨려 `BLOG_NATIVE_EDITOR_HTTP_EXIT=1`, `FL_FILE_BASE_AND_TMPDIR_REQUIRED_FOR_IMAGES`로 실패했다. [원문](evidence/l08-platform/blog-editor-config-before.txt). 원래 image-admin.fl의 계약대로 시험 root의 tmp를 설정하고 새 root에서 다시 실행했다. 앱의 경계 검사를 우회하지 않았다.

```bash
env -i PATH=<가림> LANG=C.UTF-8 TZ=UTC CPATH=<가림> LIBRARY_PATH=<가림> L08_BLOG_EDITOR_OUT=<가림> L08_BLOG_SOURCE_ROOT=<가림> L08_BLOG_NATIVE_COMPILER=<가림> strace -f -e trace=execve -o <가림> bash scripts/verify-l08-blog-editor-native.sh
bash scripts/audit-l07-execve.sh <가림>
```

수정 후 `BLOG_NATIVE_EDITOR_CONFIGURED_EXIT=0`, `NATIVE_BLOG_EDITOR_PASS=3 FAIL=0`, `NATIVE_BLOG_RESTARTS=2`, `NATIVE_BLOG_WAL_RESTART_UNCHANGED=PASS`다. 원래 editor-http의 고정 조건대로 매번 홈·로그인 **화면**·기존 editor.js·합성 세션의 글쓰기 화면이 모두 HTTP `200`이고 편집기 요소·기존 script 문자열이 유지됐다. 원래 HTTP fixture 자체를 수정하지 않았다. 각 server도 정상 종료 `0`이다. `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`. [결과 원문](evidence/l08-platform/blog-editor-native-after.txt).

`AUTHENTICATION_METHOD=SYNTHETIC_PRESEEDED_SESSION`, `PASSWORD_LOGIN=UNEXECUTED`, `EXISTING_EDITOR_JS=SERVED_NOT_EXECUTED`다. 기존 내장 JS는 그대로 제공했으며 브라우저에서 실행하거나 새 JS를 작성하지 않았다. 이 시험은 화면 HTML·script 제공과 재시작을 확인했으며 브라우저 조작·화면 캡처·글 편집 저장 성공은 별도다.

## 미확인

- auth_verify_password는 여전히 미구현·빈 scrypt key 정책 결정 대기다. 이번 언어 기능은 미정의 호출을 실행 시 오류로 처리하도록 해 전체 C 컴파일을 허용한 것이다. 실제 비밀번호 로그인에서 이 미구현 API를 호출하면 실패하며, 이를 구현 완료나 로그인 성공으로 집계하지 않는다.
- 함수 미발견 hint 문구 전체·모든 동적 이름 변경과 callback 수명·동시성은 미확인이다. 최소 정답은 v11의 안정된 message 시작과 정의/호출 구분이며 전체 C01 완료가 아니다.
- tail·swap까지 포함한 고정 `7fbe7698478d13ff5d6cfa8bf16c69e0aabb92d4` 전체 회귀는 `TAIL_PLATFORM_FULL_EXIT=1`로 TCP에서 멈췄다. non-last tail 부작용 누락의 최소 재현을 먼저 고정했으며 수정·전체 재검증이 필요하다. 이전 `a1c69c2e85ae700edea1be03e14a8ced7eb155b6` 전체 통과를 현재 코드에 적용하지 않는다.
- 댓글·다중 작성자·브라우저 J안·미디어 처리·예약/편집/운영 기능·실제 화면·운영 데이터 호환·백업/복구·성능은 남아 있다. 서버 23·25 접속·운영 데이터 수정·합치기·배포는 하지 않았다.

이후 non-last emitter를 보완한 소스에서 이름 조회를 포함한 ASan/UBSan 11묶음, 기존 TCP/TLS 8묶음, 전체 앱 editor HTTP 3회·재시작 2회를 재확인했다. 각각 종료 `0`이다. [후속 언어·TCP·앱 원문](L08-tail-progress.md#non-last-위치-보완과-tcp-회복-재확인). 이전 TCP 실패 기록은 보존했으며 최신 전체 회귀·bootstrap 통과로 대체하지 않는다.
