# C18 연산 호출 오류 문맥 부분 보완

## 확인한 것

`.fl`·정답·v11 소스 계약을 먼저 커밋하고 `CALL_CONTEXT_BEFORE_EXIT=1`을 보존했다. 기존 fx에서 outer→leaf의 오류를 catch하면 boom만 남아 원래 기대값 `[leaf] boom`과 달랐다. [계약](../tests/l08-platform/call-error-context-contract.md), [수정 전 원문](evidence/l08-platform/call-error-context-before.txt).

바인딩 변환 때 원래 호출 이름을 보존하고, 일반 사용자 함수와 지원되는 v11 builtin 호출에 오류 문맥을 기록한다. 인자는 문맥 진입 전에 평가하며, catch 경계 밖으로 나가는 문맥만 붙인다. 중복 `[op]`·`(op)`와 첫 위치 정보를 보존한다. 추가 setjmp frame을 함수마다 만들지 않으며 thread-local 호출 문맥은 오류 전달 때 해제한다. unwind 때 atom 보호 참조와 콜스택 깊이도 복원한다. builtin마다 closure를 새로 할당하지 않는다.

```bash
CPATH=<가림> LIBRARY_PATH=<가림> L08_CALL_CONTEXT_OUT=<가림> L08_CALL_CONTEXT_COMPILER=<가림> L08_CALL_CONTEXT_SANITIZER_LIBDIR=<가림> strace -f -e trace=execve -o <가림> bash scripts/verify-l08-call-context.sh
bash scripts/audit-l07-execve.sh <가림>
```

호출 문맥 7개 출력과 기존 compiler-blockers·try-scope·atom-snapshot이 정상 native 실행에서 통과했다. 동일 4묶음의 ASan/UBSan 결과는 `L08_CALL_CONTEXT_PASS=4 FAIL=0`, `CALL_CONTEXT_SANITIZER_EXIT=0`, `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다. [원문](evidence/l08-platform/call-error-context-after.txt). `detect_leaks=0`이며 누수 검사는 미실행이다.

## 미확인 / 실제 저장소의 다음 차이

실제 저장소는 여전히 `writer_NATIVE_EXIT=1`, `BLOG_NATIVE_STORE_CONTEXT_EXIT=1`로 schema constraint 시험에서 멈췄다. [원문](evidence/l08-platform/blog-store-after-context.txt). 실제 writer의 ASan/UBSan 재실행도 같은 논리 실패로 `STORE_CONTEXT_SANITIZER_EXIT=1`이다. 해당 지점까지 메모리 진단은 없었지만 writer 성공·전체 메모리 안전·복구 성공으로 판정하지 않는다. 이 ASan writer 실행에는 별도 strace를 붙이지 않았으며 앞의 자체 driver 및 일반 저장소 실행의 strace와 구분한다. [ASan writer 출력](evidence/l08-platform/blog-store-after-context-sanitized.txt).

진단용 generated C 복사본에 값 출력만 추가했다. 앱·vendor 원본과 원래 시험은 변경하지 않았다. 생성·삽입 결과는 정상이지만 오류 값에는 `[execute-insert]` 접두어가 추가로 있었다. [진단](evidence/l08-platform/blog-store-constraint-diagnostic.txt). v11은 execute-ast의 if branch를 TailCall로 반환하여 이 중간 문맥을 생략한다. 꼬리 호출 처리·상호 호출 깊이·값 위치 구분을 다음 최소 재현부터 보완해야 한다. 단순히 문자열에서 접두어를 지우지 않는다. 전체 회귀·native bootstrap·실제 저장소 writer/reader 10회는 아직 통과하지 않았다.

초기 재시험 호출에는 잘못 지정한 compiler 경로 때문에 `BLOG_NATIVE_STORE_CONTEXT_EXIT=127`도 있었다. 해당 호출은 코드 실행 증거가 아니며 실제 compiler 존재를 확인한 새 시험 영역에서 다시 실행했다. [호출 실패 원문](evidence/l08-platform/blog-store-context-invocation-before.txt).

전체 블로그 로그인은 auth_verify_password 정책 결정 대기이며 아직 실행하지 못했다. PRD의 편집·발행·미디어·계정·댓글·화면·운영 데이터 호환·복구·성능은 별도다. 서버 23·25 접속·운영 데이터 수정·합치기·배포는 하지 않았다.
