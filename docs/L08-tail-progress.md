# C18 꼬리 호출·C02 함수 실행과 실제 저장소 진행

## 확인한 것

v11 소스의 if TailCall·일반 함수 trampoline·let/인자/while 값 위치 계약으로 `.fl`·정답을 먼저 커밋했다. 최초 `TAIL_CONTEXT_BEFORE_EXIT=1`, 추가 recur 형제 branch의 `TAIL_EXTRA_BEFORE_EXIT=1`, while 부작용 누락의 `TAIL_WHILE_BEFORE_EXIT=1`을 보존했다. [계약](../tests/l08-platform/tail-call-context-contract.md), [최초 실패](evidence/l08-platform/tail-call-context-before.txt), [recur 실패](evidence/l08-platform/tail-call-extra-before.txt), [while 실패](evidence/l08-platform/tail-while-before.txt). 정답을 구현에 맞춰 바꾸지 않았다.

런타임은 내부 tail 요청을 반복 실행한다. 호출마다 C stack을 더 쌓거나 오류 문자열에서 이름을 지우지 않는다. 일반 defn의 실행 모드와 lambda의 caller 모드를 구분하며 값 위치는 원래 모드를 복원한다. catch 경계 밖으로 나가는 실행 모드·borrow pin·콜스택 문맥을 함께 정리한다. 직접 recur와 다른 함수의 tail branch가 함께 있는 경우도 처리한다. 각 반복은 private arena를 재사용하고 다음 인자·반환값·반환 closure를 소유한 heap 값으로 보존한 뒤 이전 반복을 정리한다. caller arena는 유지한다.

초기 candidate 빌드는 loop 매크로 괄호와 identity 바인딩 누락으로 `TAIL_PROBE_BUILD_EXIT=1`이었다. [원문](evidence/l08-platform/tail-call-first-build-before.txt). v11 src/eval-builtins.ts:2773~2774와 bootstrap.js:17440의 identity는 첫 인자를 그대로 반환한다. 기존 C identity는 있었지만 이름 변환 때문에 연결되지 않았다. v11 native dispatcher에 이를 연결했으며 시험을 다른 함수로 바꾸지 않았다.

```bash
CPATH=<가림> LIBRARY_PATH=<가림> L08_TAIL_OUT=<가림> L08_TAIL_NATIVE_COMPILER=<가림> L08_TAIL_SANITIZER_LIBDIR=<가림> strace -f -e trace=execve -o <가림> bash scripts/verify-l08-tail-self.sh
bash scripts/audit-l07-execve.sh <가림>
```

최소 tail 9개 출력과 추가 recur/lambda 4개 출력은 정상 native 실행에서 통과했다. [9개 출력](evidence/l08-platform/tail-call-initial-after.txt), [4개 출력](evidence/l08-platform/tail-call-extra-after.txt). 깊이 5000 상호 호출·오류 전달·map 인자 수명·반환 closure, let RHS와 body·인자·loop 값 위치를 확인했다.

기존 호출 문맥·compiler blockers·try/finally·atom을 포함한 ASan/UBSan driver는 처음 `L08_TAIL_PASS=5 FAIL=0`, 보강 후 `L08_TAIL_PASS=6 FAIL=0`, `TAIL_FINAL_SANITIZER_EXIT=0`이다. [초기](evidence/l08-platform/tail-call-initial-sanitized.txt), [보강 원문](evidence/l08-platform/tail-call-final-sanitized.txt). 그 뒤 while 수정의 별도 ASan/UBSan 실행은 `TAIL_WHILE_SANITIZER_EXIT=0`이다. 최신 compiler가 만든 기존 6개 C 파일이 보강 ASan 실행의 입력과 byte 단위로 같음을 확인했다(`TAIL_EXISTING_GENERATED_C_EQ=6`). [while·동일성 원문](evidence/l08-platform/tail-while-after.txt). 각각 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다. 최신 driver는 while까지 포함한 7묶음이며, 위 결과를 7묶음 driver 자체의 종료 결과라고 바꾸지 않는다. 전체 회귀 driver에서 다시 실행한다.

## 실제 blog 저장소 복사본

앱과 vendor는 변경하지 않았다. source-only 복사본의 원래 stage1-write/verify를 native로 실행했다. 최초 writer 성공 뒤 driver가 load된 db95 자체 시험 stdout까지 fixture marker 정답에 diff하여 실패했다. [원문](evidence/l08-platform/blog-store-stdout-before.txt). 원래 blog 시험 runner·load 소스에 따라 prelude와 marker를 구분하도록 driver만 정정했다. 기대 marker 파일·WAL 거래는 유지했다. [정정 근거와 규칙](../tests/l08-platform/blog-store-stdout-contract.md).

```bash
CPATH=<가림> LIBRARY_PATH=<가림> L08_BLOG_STORE_OUT=<가림> L08_BLOG_SOURCE_ROOT=<가림> L08_BLOG_NATIVE_COMPILER=<가림> strace -f -e trace=execve -o <가림> bash scripts/verify-l08-blog-store-native.sh
bash scripts/audit-l07-execve.sh <가림>
```

`writer_NATIVE_EXIT=0`과 별도 reader 과정 10개가 각각 종료 `0`이다. 글 생성·읽기·수정, 재시작 복구와 draft 비공개, 각 reader의 같은 db95 prelude, 재생 전후 WAL 바이트 동일성을 확인했다. `DB95_NATIVE_STORE_PASS=11 FAIL=0`, `BLOG_STORE_TAIL_RESTART_EXIT=0`, `DB95_NATIVE_WAL_READ_ONLY=PASS`, `DB95_NATIVE_WAL_SHA256=bdbd45ed8c835322f4c504293fa47dcb9c491aa32654283b7bca5484214aa7da`, `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다. [원문](evidence/l08-platform/blog-store-tail-restart.txt). 운영 WAL·설정·이미지 대신 빈 시험 root와 합성 글을 사용했다. HTTP 시험이 아니며 운영 데이터 재생 성공이라고 확대하지 않는다.

## 미확인

- 실제 저장소의 첫 ASan/UBSan 재실행은 `writer_NATIVE_EXIT=1`, `BLOG_STORE_TAIL_SANITIZER_EXIT=1`이다. 아래 zero-extra swap 보완 뒤 새 시험 root에서 다시 실행 중이며, 일반 실행 성공으로 대체하지 않는다.
- 최신 전체 회귀·L06/L07·native bootstrap 결과는 아직 없다. 타이머·TCP·snapshot까지만 포함한 이전 고정 소스 전체 실행과 구분한다.
- `detect_leaks=0`이며 누수 검사는 미실행이다. native 반복 상한 profile은 첫 global 진입 2000000·함수 값 1000000이며 해당 상한 경계 시험은 미실행이다. atom 영구 수명·RC cycle·swap! 원자성·DB 동시 거래와 전체 메모리 안정성은 남아 있다.
- 원래 blog fixture 8개의 별도 C 구문 조사에서는 6개 통과·2개 실패다. review/AI fixture가 load하지 않은 관리자·이미지 함수에 대한 호출을 정의만 해도 C 미선언 오류가 난다. 실행 시 이름을 찾는 언어 계약 보완이 다음 작업이다. [조사 요약과 선택 진단](evidence/l08-platform/blog-fixture-syntax-before.txt). C 구문 통과는 링크·실행 통과가 아니다.
- auth_verify_password의 빈 scrypt key 정책은 사용자 결정 대기다. 전체 앱 실행·로그인·브라우저 J안·댓글·다중 작성자·실제 화면·운영 데이터 호환·백업/복구·성능은 여전히 미완료다. 서버 23·25 접속·운영 데이터 수정·합치기·배포는 하지 않았다.

## 실제 저장소 sanitizer에서 발견한 zero-extra swap

실제 writer는 collection.c의 swap!에서 길이 0의 memcpy에 NULL source를 넘기는 UB로 멈췄다. [실패 원문](evidence/l08-platform/blog-store-tail-sanitizer-before.txt). v11 src/eval-builtins.ts:526~534는 extraArgs가 없어도 함수에 현재 값 하나를 전달하므로 이 경우도 지원해야 한다. 최소 `.fl`·정답을 먼저 커밋하고 같은 `SWAP_ZERO_BEFORE_EXIT=1`을 보존했다. [최소 실패](evidence/l08-platform/swap-zero-before.txt).

복사는 extra 길이가 있을 때만 실행하도록 공유 런타임 한 곳을 고쳤다. extra가 없는 반환·저장, callback 오류 전달·기존 값 유지, extra 2개, nil 반환·저장의 7개 출력이 ASan/UBSan에서 통과했다. `SWAP_ZERO_FIXED_EXIT=0`, `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`. [수정 후 원문](evidence/l08-platform/swap-zero-after.txt). swap! 논리적 동시성은 이 수정 범위가 아니다. 최신 tail 자체 driver는 이 시험을 추가한 8묶음이며, 전체 회귀에서 검증한다.

## 실제 저장소 ASan/UBSan 재실행과 전체 회귀 실패

zero-extra guard 구현은 `7fbe7698478d13ff5d6cfa8bf16c69e0aabb92d4`다. 실제 writer 1개와 별도 reader 10개가 ASan/UBSan에서 모두 종료 `0`, `DB95_NATIVE_STORE_PASS=11 FAIL=0`, `BLOG_STORE_SWAP_GUARD_SANITIZER_EXIT=0`이다. 재시작 복구·draft 비공개·WAL 바이트 동일성과 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`을 확인했다. [원문](evidence/l08-platform/blog-store-swap-sanitized-after.txt). 누수 검사는 미실행이다.

이 실행의 합성 WAL SHA는 `5a35fe5060906f60d73055dd2d9d3d03494bf16ebab007b194fc59d4719b77f1`이다. 일반 실행과 각각 `6`개 기록이며 두 새 writer가 생성한 uuid_short 거래 ID만 달랐다. 비교용 stream에서 tx field를 제외하면 나머지는 byte 단위로 같다. 원본 WAL·거래·본문은 변경하지 않았으며 각 실행의 재시작 검증은 원본 전체를 비교했다. [읽기 전용 비교 원문](evidence/l08-platform/blog-store-run-comparison.txt). 운영 데이터 해시 비교가 아니다.

같은 소스의 전체 driver에서 자체 tail은 `L08_TAIL_PASS=8 FAIL=0`, `TAIL_SELF_EXIT=0`이지만 이후 TCP에서 실패했다. `TAIL_PLATFORM_FULL_EXIT=1`이다. [전체 실패](evidence/l08-platform/tail-full-before.txt). raw 큰 slow-reader echo가 실패하고 TLS 회복도 실패했다. 따라서 이 최신 소스의 L06/L07 전체 회귀·native bootstrap은 실행하지 못했다. non-last if에서 만든 tail 요청이 다음 식에 덮여 부작용을 잃는 최소 재현·정답을 추가했다. [후속 계약](../tests/l08-platform/tail-nonlast-contract.md). 기존 TCP·tail 정답은 유지하며 emitter의 tail 위치 구분을 다음에 고쳐야 한다. 전체 언어 완료·배포 가능으로 판정하지 않는다.
