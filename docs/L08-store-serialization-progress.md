# L08 저장소 동시성·WAL 오류 경계 진행

## 확인한 것

### 언어 잠금과 동시 저장

새 `with-atom-lock guard callback` 언어 기능은 실제 atom의 재진입 잠금 아래 callback을 한 번 실행한다. guard 값은 자동 변경하지 않는다. 인자를 왼쪽부터 한 번 평가하고 오류 시 잠금을 해제하며 실행 중 guard의 소유권을 유지한다. 다른 guard의 callback은 독립적으로 진행한다. DB 함수명을 C runtime에 하드코딩하지 않았다.

시험·정답 선행 commit `e361ce26373f8a2b7dfc0bb5f8df07adcf3446f8`, 기능 부재 실패 `de35cc8976eeff0be9562efd5eeaaf37ef5300d8`, 최초 구현 `3d8fa3c23cb4ea2869602a95421085fa31f93e45`, 기존 closure API 이름 수정 `3820bc7ad6b2dc7e67ac076002f34e6026a2ab49`이다. 최초 구현은 잘못된 C 호출 이름 `fl_call` 때문에 빌드 실패했으며 그 원문도 보존했다. 최종 일반·ASan/UBSan은 17개 단정 모두 통과하고 `L08_ATOM_LOCK_PASS=1 FAIL=0`, 종료 `0`이다. 실제 8-worker×100 복합 읽기·멈춤·쓰기 결과와 callback 계측은 각각 `800`이다. [계약](../tests/l08-platform/atom-lock-contract.md), [수정 전](evidence/l08-platform/atom-lock-before.txt), [최초 빌드 실패](evidence/l08-platform/atom-lock-first-build-failure.txt), [일반](evidence/l08-platform/atom-lock-after.txt), [sanitizer](evidence/l08-platform/atom-lock-sanitized.txt).

blog1 `c8923b68d3c523241350f1d64e37a781d8f20eda`는 FreeLang app/store.fl에서 실제 execute-ast 읽기와 commit 전체에 같은 guard를 적용했다. 원래 함수 값은 보존하고 apply로 호출하여 내부 helper 이름의 추가 오류 문맥을 만들지 않는다. vendor·WAL 형식·정답·worker 수를 변경하지 않았다. v11의 새 API가 아니라 승인된 fx 이전을 위한 native 언어 기능이므로 이 앱 변경을 v11에서 실행할 수 있다고 보고하지 않는다.

동일한 실제 8-worker×4 거래 시험은 수정 전 live `20`·재생 `32`, 수정 후 live `32`·재생 `32`, 고유 ID `32`, `DB_CONCURRENT_SELF_EXIT=0`이다. reader 전후 WAL은 동일하다. [수정 전·계약](L08-arena-cache-progress.md), [수정 후 원문](evidence/l08-platform/store-concurrent-after.txt).

실제 loopback HTTP의 별도 source-only 앱 저장소 시험도 ASan/UBSan에서 통과했다. worker `4`, handler peak `4`, 동시 curl `10`, 정상 저장 `32`건 모두 HTTP `200`, live 정확한 ID 집합·한글/이모지 제목 보존, 중복 저장 `8`건 모두 `409`, 거부 후 WAL 무변경, 재시작 `2`회 뒤 정확한 `32`행·WAL 무변경이다. `DB_HTTP_VERIFIED_EXIT=0`. 최초 driver는 정상 `[http]` 시작 로그까지 stderr 오류로 취급하여 요청 전 실패했다. 알려진 시작·종료 로그만 정확한 패턴으로 허용하고 다른 stderr·sanitizer 출력을 거부하도록 고쳤다. [계약](../tests/l08-platform/store-http-contract.md), [최초 driver 실패](evidence/l08-platform/store-http-first-harness-failure.txt), [수정 후 원문](evidence/l08-platform/store-http-after.txt). 이는 관리자 인증 또는 완성된 제품 API 시험이 아니다.

### 전체 회귀에서 발견한 기존 compiler 결함

고정 `166ef374bb8417f7cd2227f23e3b98de47fe0f84` 전체는 L07 `221`개·runtime 통과 뒤 기존 jit-test.fl의 생성 C 문법 오류로 종료 `1`이었다. bootstrap은 그 실행에서 미실행이다. [원문](evidence/l08-platform/managed-full-legacy-failure.txt).

최소 정답은 `b0fd8ca5c2dd755250c1c9545bc212f95b8a3ab9`, 문법 수정은 `b27af4fd4637455b3cc647e61fdda88259443970`이다. 문법을 고친 뒤 캡처 변수 n을 nil cell로 가리는 추가 실패도 보존했다. v11의 define RHS 선평가·do의 동일 scope·정상 반환 시 captured mutation 전파를 소스로 확인하고 기존 정답을 유지했다. 중복 local cell을 만들지 않고 기존 바인딩을 재사용하는 수정은 `153400a669dac64c3334871c92f462abaf9a8a47`이다. 최소 시험 `3`개 단정, `L08_DEFINE_SEQUENCE_PASS=1 FAIL=0`, 종료 `0`. [소스 계약·이력](../tests/l08-platform/define-sequence-contract.md), [최초 실패](evidence/l08-platform/define-sequence-before.txt), [추가 실패](evidence/l08-platform/define-sequence-after-syntax-failure.txt), [수정 후](evidence/l08-platform/define-sequence-after.txt).

그 뒤 원래 runner `10`개가 JIT 함수 lookup에서 모두 실패했다. C 구현·선언은 있었지만 네 source 이름의 dispatch가 빠져 있었다. `60177c87e239c3c2ba90fea980ef8be946fb2115`에서 명시적 연결·metadata를 보완했다. 원래 jit-test.fl·gc-stress-test.fl·runner·기대값을 그대로 두고 `10 PASS / 0 FAIL / 10 총`, `JIT_NATIVE_AFTER_EXIT=0`이다. [계약](../tests/l08-platform/jit-native-contract.md), [수정 전](evidence/l08-platform/legacy-jit-name-before.txt), [수정 후](evidence/l08-platform/legacy-jit-name-after.txt).

동시성 적용 뒤 기존 실제 저장소 writer 1회·별도 reader 10회 ASan/UBSan은 `DB95_NATIVE_STORE_PASS=11 FAIL=0`, 종료 `0`, WAL 무변경이다. [원문](evidence/l08-platform/locked-store-after.txt). 최초 전체 앱 복사본에 중첩 app/themes의 기존 `.fl` 파일을 빠뜨려 compile이 실패했으며 제품 결함으로 보고하지 않는다. [첫 복사본 실패](evidence/l08-platform/locked-editor-source-copy-failure.txt), [경로를 잘못 보완한 추가 실패](evidence/l08-platform/locked-editor-theme-copy-failure.txt). 원래 중첩 FreeLang 화면 파일을 포함한 재시험 결과는 후속 원문으로 기록한다.

### WAL fsync 실패의 계약과 수정

최초 주입 시험 `08417e3e351ab33f8ff6f48b05b89bc1eb9cf9b8`은 첫 fsync를 prepare라고 잘못 가정했다. 원래 vendor는 prepare를 sync=false로 append하고 commit에서만 fsync한다. 따라서 commit record append 뒤 EIO는 저장 여부가 불확실하며 임의 rollback·WAL 기록 삭제로 해결할 수 없다. 최초 실패·기대값은 이력에 보존하고 소스 근거와 정정 이유를 구현 전에 기록했다. 새 계약은 불확실성 표시와 같은 과정의 후속 읽기·쓰기 차단이다. [정정·근거](../tests/l08-platform/store-fsync-contract.md), [최초 주입 원문](evidence/l08-platform/store-fsync-initial-contract-failure.txt), [정정 계약 수정 전 원문](evidence/l08-platform/store-uncertain-before.txt).

blog1 `29ff4123f0a5cc392cc756df030309ff88611469`는 WAL 단계가 commit 시도일 때 오류를 `DB_COMMIT_UNCERTAIN_RESTART_REQUIRED`, durable commit 뒤 후처리 오류를 `DB_COMMITTED_RESTART_REQUIRED`, prepare 실패를 `DB_WRITE_ABORTED_RESTART_REQUIRED`로 구분한다. WAL 단계에 진입한 오류 뒤에는 `DB_RECOVERY_REQUIRED`로 뒤 읽기·쓰기를 차단한다. 검증 오류처럼 WAL 단계 이전의 실패는 기존 오류를 전달한다. commit을 삭제하거나 운영 자료를 변경하지 않았다.

수정 후 고정 fx `bd1213b76b23d6e12e17a329c2dafe4638000eaf`의 driver와 source-only blog 복사본에서 fsync 주입은 `DB_FSYNC_UNCERTAIN_REPORTED=true`, `DB_FSYNC_READ_REJECTED=true`, `DB_FSYNC_WRITE_REJECTED=true`, writer `0`, 새 reader `DB_FSYNC_REPLAY_ROWS=1`, reader `0`, WAL 무변경, `RECOVERY_FSYNC_EXIT=0`이다. 이 주입은 실제 전원 장애가 아니며 page cache에 기록된 첫 거래가 재생된 경우다. 전원 장애에서 거래가 반드시 남거나 반드시 사라진다는 보장은 하지 않는다. 최종 raw와 남은 정상 동시성·HTTP·원래 앱 회귀는 같은 고정 pipeline이 끝난 범위만 후속으로 기록한다.

```bash
L08_ATOM_LOCK_OUT=<가림> L08_ATOM_LOCK_COMPILER=<가림>/compiler strace -f -e trace=execve -o <가림>/execve.log bash scripts/verify-l08-atom-lock.sh
L08_DEFINE_OUT=<가림> strace -f -e trace=execve -o <가림>/execve.log bash scripts/verify-l08-define-sequence.sh
L08_BLOG_SOURCE_ROOT=<가림> L08_BLOG_NATIVE_COMPILER=<가림>/compiler L08_BLOG_FSYNC_OUT=<가림> strace -f -e trace=execve -o <가림>/execve.log bash scripts/verify-l08-blog-fsync.sh
L08_BLOG_SOURCE_ROOT=<가림> L08_BLOG_NATIVE_COMPILER=<가림>/compiler L08_BLOG_CONCURRENT_OUT=<가림> strace -f -e trace=execve -o <가림>/execve.log bash scripts/verify-l08-blog-concurrent.sh
L08_BLOG_SOURCE_ROOT=<가림> L08_BLOG_NATIVE_COMPILER=<가림>/compiler L08_BLOG_HTTP_OUT=<가림> L08_BLOG_HTTP_SANITIZER_LIBDIR=<가림> strace -f -e trace=execve -o <가림>/execve.log bash scripts/verify-l08-blog-store-http.sh
```

## 미확인

- 최신 언어 전체 고정 source의 회귀는 실행 중이다. 앞선 고정 driver는 이후 JIT dispatch 보완을 포함하지 않는다. native bootstrap은 별도 고정 `60177c87e239c3c2ba90fea980ef8be946fb2115`에서 실행 중이며 stage2→stage3가 실제 CPU를 사용하고 있다. 아직 종료·stage2=stage3를 확인하지 못했으며 완료로 집계하지 않는다.
- WAL prepare write/close 실패, commit 성공 뒤 후처리 실패의 주입 시험, fd의 오류 정리, 재시작 없이 복구하는 절차, health의 복구 필요 상태, 실제 전원 장애·백업/복구·성능 p95·동시 읽기 snapshot의 전체 검증은 남아 있다. 새 오류 코드가 있다는 이유로 해당 분기를 시험했다고 보고하지 않는다.
- 일반 LSan·장시간 RSS·opaque promise cycle·startup 수명·포기한 future/반복·동시 대기/예외도 남아 있다. sanitizer는 detect_leaks=0이다.
- 실제 비밀번호 로그인·빈 scrypt 키 계약 결정, 브라우저 J안·이미지 처리·댓글·다중 작성자와 전체 PRD·운영 자료 호환·백업/복구는 완료되지 않았다. 기존 editor JS의 제공 시험은 브라우저 실행이나 J안 완료가 아니다.
- 서버 23·25 접속·운영 자료 접근/변경·합치기·배포·강제 푸시·백업 삭제를 하지 않았다. 각 확보한 종료 원문의 execve 감사는 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이며 이후 아직 실행 중인 전체 시험까지 확대하지 않는다.

## 후속으로 확인한 결과

### 오류 차단 수정 뒤 전체 저장소·실제 앱 회귀

fx 고정 `bd1213b76b23d6e12e17a329c2dafe4638000eaf`, blog source `29ff4123f0a5cc392cc756df030309ff88611469`의 fresh source-only 복사본에서 다섯 driver가 모두 종료 `0`, `RECOVERY_PIPELINE_EXIT=0`이다. 원래 중첩 FreeLang 화면 파일을 포함했다.

| 실제 실행 | 원문 결과 | 증거 |
|---|---|---|
| commit fsync EIO | uncertain/read 거부/write 거부 각각 true, reader 1행, WAL 무변경 | [원문](evidence/l08-platform/recovery-fsync-after.txt) |
| 실제 8-worker×4 거래 | live/고유 ID/replay 각각 32, WAL 무변경 | [원문](evidence/l08-platform/recovery-concurrent-after.txt) |
| ASan/UBSan 실제 HTTP | worker/peak 4, 32건 200, 중복 8건 409, 재시작 2회 모두 정확한 32행·WAL 무변경 | [원문](evidence/l08-platform/recovery-store-http-after.txt) |
| 기존 ASan/UBSan 저장소 | writer 1회·별도 reader 10회, `DB95_NATIVE_STORE_PASS=11 FAIL=0` | [원문](evidence/l08-platform/recovery-store-native-after.txt) |
| 전체 main·seed·editor-http native | 세 빌드 0, HTTP 확인 3회, 재시작 2회, WAL 무변경, `NATIVE_BLOG_EDITOR_PASS=3 FAIL=0` | [원문](evidence/l08-platform/recovery-editor-native-after.txt) |

각 execve 감사는 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다. 편집 화면은 합성 세션·HTML HTTP 200·기존 JS 제공이며 `PASSWORD_LOGIN=UNEXECUTED`, `EXISTING_EDITOR_JS=SERVED_NOT_EXECUTED`다. 새 JS나 실제 비밀번호 로그인·브라우저 J안 완료를 뜻하지 않는다.

### native source 이름의 공통 누락 보완

별도 `60177c87e239c3c2ba90fea980ef8be946fb2115` bootstrap에서 stage2 binary를 실제로 확인하고 가장 작은 `(println 1)` 입력을 보냈다. stage1은 `0`, stage2는 `[to-string] Function not found`로 `1`이었다. 큰 자기 컴파일은 아직 CPU·메모리를 사용하고 있었으며 독립적으로 확인한 이 결함 때문에 해당 stage2만 종료했다. `JIT_NATIVE_BOOTSTRAP_EXIT=143`이며 timeout·전체 통과로 보고하지 않는다. 생성 C·작은 probe·중단 이유를 보존했다. [작은 실패](evidence/l08-platform/stage2-small-probe-before.txt), [큰 검증 중단 원문](evidence/l08-platform/jit-bootstrap-stopped-after-probe.txt).

`bf6b99d4ab84ebdbb51794117dc9a2012c139d7b`에서 runtime.h에 선언된 FLValue C 함수의 underscore 이름과 hyphen source 이름을 공통 metadata에 함께 생성했다. 임의 미정의 함수 lookup이나 --extern 검증은 유지한다. 앞선 네 JIT 개별 dispatch는 공통 경로로 대체했다. [계약](../tests/l08-platform/native-spelling-contract.md), [to-string 수정 전](evidence/l08-platform/native-spelling-before.txt), [수정 후 실제 `42` 출력](evidence/l08-platform/native-spelling-after.txt).

고정 `2754d8f7d83ce520395ab1c2bfc99b6d72336379`에서 기존 names/redefine/scope/aliases 4개를 원래 기대값으로 확인했고, 원래 JIT·GC runner도 다시 `10 PASS / 0 FAIL / 10 총`, `NATIVE_NAMES_SELECTED_EXIT=0`이다. [원문](evidence/l08-platform/native-names-selected-after.txt). 같은 고정 bootstrap의 새 stage1과 stage2는 작은 `(println 1)` 입력에서 각각 종료 `0`, 생성 C도 동일하다. [원문](evidence/l08-platform/native-names-stage2-small-after.txt). 이 작은 입력 통과로 큰 자기 입력이나 전체 bootstrap 통과를 대신하지 않는다.

고정 `2754d8f7d83ce520395ab1c2bfc99b6d72336379` 전체 언어 회귀·native bootstrap은 실행 중이다. 공통 이름 보완 뒤 source-only blog의 실제 HTTP·전체 앱도 이 고정 소스와 새 compiler로 별도 재확인 중이다. 위의 종료한 다섯 단계와 구분하며 이후 실행을 미리 통과로 집계하지 않는다. 모든 PRD·실제 인증·브라우저·댓글·다중 작성자·운영 자료·백업/복구 완료는 여전히 미확인이다.

최신 공통 native 이름 보완을 포함한 고정 `2754d8f7d83ce520395ab1c2bfc99b6d72336379`와 blog source `29ff4123f0a5cc392cc756df030309ff88611469`의 후속 실제 앱 두 단계도 종료했다. ASan/UBSan 실제 HTTP는 worker/peak `4`, 저장 `32`건 모두 `200`, 중복 `8`건 모두 `409`, 재시작 `2`회 정확한 `32`행·WAL 무변경이다. 전체 main·seed·editor-http native 빌드는 모두 `0`, HTTP 확인 `3`회·재시작 `2`회·WAL 무변경, `NATIVE_BLOG_EDITOR_PASS=3 FAIL=0`이다. `NATIVE_NAMES_APP_PIPELINE_EXIT=0`이며 두 execve 감사 모두 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다. [실제 HTTP 원문](evidence/l08-platform/native-names-app-store-http-after.txt), [전체 앱·편집 HTML 원문](evidence/l08-platform/native-names-app-editor-native-after.txt). 합성 세션·기존 JS 제공만 확인했고 실제 비밀번호 로그인·JS 실행·브라우저 J안은 미확인이다. 전체 언어 회귀·큰 native bootstrap은 이 두 앱 결과와 별도로 아직 실행 중이다.

## 후속 파일 정리·compiler 메모리 조사

이후 WAL 중간 오류의 fd 누수·durable close 오류 분류를 실제로 재현하고 고쳤다. compiler의 큰 자기 입력에서 확인한 메모리 증가도 최소 lexer·concat 실패를 먼저 고정한 뒤 보완했다. 단계별 실패·최종 최소 시험 통과·아직 실행 중인 전체 검증의 구분은 [후속 진행](L08-wal-close-compiler-memory.md)을 따른다. 이 문서의 앞선 실행 중 표기는 당시 기록이다.
