# L08 WAL 파일 정리와 native compiler 메모리 진행

플랫폼 전체 완료 보고가 아니다. 실제 비밀번호 로그인·브라우저 J안·이미지 처리·댓글·다중 작성자·운영 자료 호환·백업/복구와 나머지 PRD 범위는 유지한다.

## 확인한 것

### WAL 오류 뒤 파일 정리와 저장 결과

새 자체 `.fl` 시험·기대값과 C syscall 주입 helper를 먼저 커밋했다. 실제 app/store.fl·원래 vendor를 source-only 복사본에서 실행하며, 새 합성 WAL inode의 write/fsync/close에만 한 번씩 EIO를 주입한다. C helper가 DB를 직접 구현하거나 결과를 바꾸지 않는다. [계약](../tests/l08-platform/store-wal-fault-contract.md).

수정 전은 `WAL_FAULT_PASS=6 FAIL=4`, 종료 `1`이다. prepare write·부분 write·commit fsync 오류 뒤 열린 WAL fd가 각각 `1`개 남았다. commit의 fsync가 이미 성공한 뒤 close 오류는 저장 완료 대신 불확실한 결과로 잘못 분류했다. [실패 원문](evidence/l08-platform/wal-fault-before.txt).

blog1 `d3ebbebbd8aa27938b6793e0e452e763c1c33c0c`에서 FreeLang WAL append에 finally를 적용했다. WAL JSON·길이 상한·checksum·prepare/commit 형식을 유지한다. fsync 성공 직후 durable 상태를 기록하고 이후 close 오류는 이미 저장된 거래로 구분한다. Linux close 오류에서 이미 닫힌 fd를 재시도하지 않는 native 계약도 유지한다. 파일 열기 자체가 실패하면 닫을 fd가 없으며, 열린 뒤의 경로는 finally에서 정리한다.

| 실제 주입 | 필요한 진단 | 열린 WAL fd | 새 과정 재생 |
|---|---|---|---|
| prepare write EIO | DB_WRITE_ABORTED_RESTART_REQUIRED | 0 | 0행 |
| prepare 일부 write 후 EIO | DB_WRITE_ABORTED_RESTART_REQUIRED | 0 | 0행, 잘린 마지막 prepare 무시 |
| prepare close EIO | DB_WRITE_ABORTED_RESTART_REQUIRED | 0 | 0행, prepare만 존재 |
| commit fsync EIO | DB_COMMIT_UNCERTAIN_RESTART_REQUIRED | 0 | 1행, 전원 장애가 아닌 page-cache 주입 |
| fsync 성공 뒤 commit close EIO | DB_COMMITTED_RESTART_REQUIRED | 0 | 1행 |

각각 후속 읽기·쓰기는 `DB_RECOVERY_REQUIRED`로 차단했고 reader 전후 WAL도 동일했다. ASan/UBSan 결과는 `WAL_FAULT_PASS=10 FAIL=0`, `WAL_FAULT_AFTER_EXIT=0`이다. [원문](evidence/l08-platform/wal-fault-after.txt). 실제 전원 장애의 불확실한 거래가 반드시 남거나 반드시 사라진다고 보장하지 않는다.

같은 blog source의 정상 회귀도 `WAL_CLOSE_CONCURRENT_EXIT=0`, `WAL_CLOSE_STORE-HTTP_EXIT=0`, `WAL_CLOSE_NORMAL_PIPELINE_EXIT=0`이다. 8-worker×4 거래의 live/고유 ID/재생 각각 `32`, 실제 HTTP worker/peak `4`, 저장 `32`건 모두 `200`, 중복 `8`건 모두 `409`, 재시작 `2`회 정확한 `32`행·WAL 무변경을 확인했다. [동시 저장](evidence/l08-platform/wal-close-concurrent-after.txt), [실제 HTTP](evidence/l08-platform/wal-close-store-http-after.txt). 뒤의 compiler/concat 변경까지 같은 실행에서 확인했다고 확대하지 않는다.

### Compiler lexer와 표준 라이브러리 concat

큰 자기 입력의 bootstrap은 실제 실행 중 RSS가 `64034964` KiB까지 증가했다. source `2754d8f7d83ce520395ab1c2bfc99b6d72336379`의 이 과정은 아래 실제 lexer heap 실패를 재현한 뒤 종료했다. `NATIVE_NAMES_BOOTSTRAP_EXIT=143`이며 통과·단순 timeout으로 보고하지 않는다. [중단 이유·원문](evidence/l08-platform/native-names-bootstrap-stopped.txt).

실제 self/cgc-main.fl에서 마지막 CLI 진입 호출 한 줄만 제거한 library를 만들고 실제 lex 함수에 `(println 1)` 1000행을 넣었다. 입력·전역 준비 뒤 malloc/calloc/realloc/free의 live payload·peak를 계측했다. meter bookkeeping·공유 라이브러리 내부 할당·전체 RSS는 이 값에 포함하지 않는다. C helper는 lexer나 token 출력을 대체하지 않는다. 4000개 token과 순서·kind·값·위치를 검증하고 live ≤134217728 byte·peak ≤167772160 byte를 먼저 고정했다. [계약](../tests/l08-platform/compiler-lex-memory-contract.md).

| 실제 lexer 실행 | live byte | 결과 |
|---|---|---|
| 원래 매 token flat append | 187835551 | token 확인 통과, heap 기준 실패 |
| immutable chunk만 적용 | 1585809073 | 실패, 첫 시도는 더 나빠짐 |
| 공통 concat을 선형으로 수정 | 1418591921 | 실패, 이름 있는 tail-call이 forest 전체를 복사 |
| 같은 chunk carry를 local loop/recur로 처리 | 64095103 | 기존 token·heap 정답 모두 통과 |

원문: [최초](evidence/l08-platform/compiler-lex-memory-before.txt), [chunk 실패](evidence/l08-platform/compiler-lex-chunks-before-concat-fix.txt), [tail 복사 실패](evidence/l08-platform/compiler-lex-chunks-tail-before.txt), [최종](evidence/l08-platform/compiler-lex-memory-after.txt). 최종 peak는 `64095152`, `COMPILER_LEX_LOCAL_AFTER_EXIT=0`이다. 메모리 상한이나 token 수를 낮추지 않았다.

공통 C append(vector,vector)가 빈 결과에 한 원소씩 fl_vec_push하여 concat 한 번에도 O(N²)의 prefix를 할당하는 것을 확인했다. 2048개씩인 두 vector의 실제 `.fl concat` 시험·정답을 먼저 고정하고 결과의 4096개 순서와 입력 보존을 확인했다. `089583e402a1f9f4e42016ed23d4c029341790f7`에서 최종 vector와 buffer를 각각 한 번 할당하고 입력을 memcpy하도록 바꿨으며, `5938179e3862cf050d24cc0a03c5ad1b59305d40`에서 내부 arena 선언을 포함해 빌드를 보완했다. 길이 overflow를 거부하고 buffer allocation 실패 때 main의 먼저 할당한 header를 정리한다. 일반 append(vector,scalar)의 의미를 바꾸지 않았다. [계약](../tests/l08-platform/concat-memory-contract.md).

실제 concat의 live는 `134348950`→`65686` byte, 최종 peak `65735`, `CONCAT_CURRENT_AFTER_EXIT=0`이다. [수정 전](evidence/l08-platform/concat-memory-before.txt), [수정 후](evidence/l08-platform/concat-memory-after.txt). 결과 순서·입력 보존·원래 heap 상한을 모두 유지했다.

self compiler는 immutable chunk 누적을 사용하되 `39085d8d3c9c211b6610c131fc95030ddcf1f5c1`에서 carry를 기존 loop/recur로 표현했다. compiler 내부에서 큰 forest를 함수 간 tail snapshot으로 넘기는 비용을 피하며 generic tail-call의 소유권이나 immutable array 계약을 바꾸지 않는다. generic tail-call의 큰 공유 인자 복사 비용 전체를 해결한 것은 아니다.

새 library 시험 driver의 GNU feature macro·실제 compiler bridge 연결 누락과 최초 concat 빌드의 내부 선언 누락도 보존했다. bridge 연결을 시도한 명령 한 번이 적용되지 않아 후속 commit에서 실제 patch를 적용했다. 이 driver/build 실패를 정상 lexer 측정이나 통과로 보고하지 않는다. [feature macro 실패](evidence/l08-platform/compiler-lex-first-driver-build-failure.txt), [bridge 실패](evidence/l08-platform/compiler-lex-driver-bridge-failure.txt), [concat 빌드 실패](evidence/l08-platform/concat-first-build-failure.txt).

수정 중간 source `5938179e3862cf050d24cc0a03c5ad1b59305d40`의 큰 bootstrap도 별도 lexer heap 실패를 확인한 뒤 종료했다. `CONCAT_NATIVE_BOOTSTRAP_EXIT=143`이며 [중단 원문](evidence/l08-platform/concat-bootstrap-stopped.txt)을 남겼다. 기존 verify-native-bootstrap.sh·stage0 source·기대 SHA를 변경하지 않았다.

각 확보한 종료 원문의 strace execve 감사는 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다. ASan/UBSan은 detect_leaks=0이다. fd가 실제 닫힌 것을 확인한 결과를 일반 heap 누수 검사 통과로 대신하지 않는다.

```bash
L08_BLOG_SOURCE_ROOT=<가림> L08_BLOG_NATIVE_COMPILER=<가림>/compiler L08_BLOG_WAL_FAULT_OUT=<가림> L08_BLOG_WAL_FAULT_SANITIZER_LIBDIR=<가림> strace -f -e trace=execve -o <가림>/execve.log bash scripts/verify-l08-blog-wal-faults.sh
L08_CONCAT_OUT=<가림> L08_CONCAT_COMPILER=<가림>/compiler strace -f -e trace=execve -o <가림>/execve.log bash scripts/verify-l08-concat-memory.sh
L08_COMPILER_LEX_OUT=<가림> L08_COMPILER_LEX_COMPILER=<가림>/compiler strace -f -e trace=execve -o <가림>/execve.log bash scripts/verify-l08-compiler-lex-memory.sh
L07_OUT=<가림> unshare --user --map-root-user --pid --fork --mount-proc bash scripts/verify-l07-bootstrap.sh
L08_PLATFORM_OUT=<가림> L08_TIMER_SANITIZER_LIBDIR=<가림> bash scripts/verify-l08-platform-all.sh
```

## 미확인

- 최종 code를 포함한 고정 `54435153fb5a536b137730c815d07ec8baacf73d`의 전체 언어 회귀·native bootstrap은 실행 중이다. 작은 lexer/concat 시험으로 큰 자기 입력의 stage2=stage3나 전체 회귀 완료를 대신하지 않는다. 중간 source의 실패·중단과 최신 실행을 구분한다.
- 같은 고정 runtime·새 compiler와 blog source `d3ebbebbd8aa27938b6793e0e452e763c1c33c0c`로 WAL 오류·실제 HTTP·전체 앱을 별도 재확인 중이다. 앞서 종료한 오류 10개와 정상 회귀를 현재 source 모든 gate의 단일 전체 통과로 집계하지 않는다.
- concat의 새 overflow/OOM 가드 주입, WAL 열기 실패·catalog page 후처리 오류·health, 일반 LSan·장시간 RSS·future/promise 수명·실제 crash·백업/복원·성능 p95·전체 PRD 검증은 남아 있다.
- 실제 비밀번호 검증·빈 scrypt 키 계약의 사용자 결정, 브라우저 J안·이미지 처리·댓글·다중 작성자·실제 화면과 승인된 운영 자료 복사본의 호환·백업/복구도 미완료다. 기존 editor JS의 제공을 실행이나 J안 완료로 보고하지 않는다.
- 서버 23·25 접속·운영 자료 접근/변경·합치기·배포·강제 푸시·백업 삭제를 하지 않았다. 임시 시험 process의 중단은 goal 중단이나 운영 process 종료가 아니다.

## 이후 종료한 검증

고정 `54435153fb5a536b137730c815d07ec8baacf73d`의 runtime과 그 source에서 새로 만든 native compiler로 blog source `d3ebbebbd8aa27938b6793e0e452e763c1c33c0c`를 다시 생성한 뒤 WAL 오류 10개도 ASan/UBSan에서 통과했다. `WAL_FAULT_PASS=10 FAIL=0`, `LEXER_FINAL_APP_WAL-FAULTS_EXIT=0`, `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다. [원문](evidence/l08-platform/wal-fault-latest-compiler-after.txt). 같은 pipeline의 실제 HTTP·전체 앱과 별도 전체 언어 회귀·큰 native bootstrap은 아직 실행 중이며 해당 결과를 미리 통과로 집계하지 않는다.

최종 고정 `54435153fb5a536b137730c815d07ec8baacf73d`·blog source `d3ebbebbd8aa27938b6793e0e452e763c1c33c0c`의 후속 두 단계도 종료했다. 실제 ASan/UBSan HTTP는 저장 32건 모두 200·중복 8건 모두 409·worker/peak 4·재시작 2회 정확한 32행·WAL 무변경이다. 전체 main·seed·editor-http native 빌드 모두 0, HTTP 확인 3회·재시작 2회·WAL 무변경, `NATIVE_BLOG_EDITOR_PASS=3 FAIL=0`, `LEXER_FINAL_APP_PIPELINE_EXIT=0`이다. 두 execve 감사도 모두 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다. [실제 HTTP](evidence/l08-platform/wal-close-final-store-http-after.txt), [전체 앱·편집 HTML](evidence/l08-platform/wal-close-final-editor-native-after.txt). 합성 세션이며 `PASSWORD_LOGIN=UNEXECUTED`, `EXISTING_EDITOR_JS=SERVED_NOT_EXECUTED`다. 전체 언어 회귀·큰 bootstrap은 별도 실행 중이며 미완료다. 잘린 마지막 prepare를 읽은 뒤 새 쓰기를 재개하고 다시 재시작하는 경우도 아직 검증하지 않았으므로 부분 재생 통과를 완전한 복구로 판정하지 않는다.
