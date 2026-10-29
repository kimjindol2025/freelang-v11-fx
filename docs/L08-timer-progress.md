# C19 타이머 부분 진행 보고

## 확인한 것

`.fl`·정답·계약을 먼저 커밋하고 `TIMER_BEFORE_BUILD_EXIT=1`을 보존한 뒤 set-timeout, await_result, fl-yield를 네이티브에 연결했다. v11 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4`의 timer 분기와 FreeLangPromise·await_result 소스를 텍스트로 조사했으며 Node로 실행하지 않았다. [고정 계약](../tests/l08-platform/timer-contract.md).

구현은 `ef5e597c21b875d29e889514fb314232431600fa`다. 별도 새 JS·TS·Python 없이 C에 monotonic timer queue·관리되는 promise와 private wake pipe를 추가했다. CLI 정상 종료 전에 pending 작업을 진행하며 명시적 process-exit은 취소하고 종료 코드를 유지한다. HTTP listener는 poll로 방문 없이 작업을 진행하며 요청 worker는 4개다. C19의 TCP I/O는 아직 없으므로 현재 fl-yield의 I/O 이벤트 수는 0이다.

요청 뒤에 실행되는 closure는 escape할 때 환경을 heap으로 승격한다. atom identity를 복제하지 않고 promise 참조는 원자적으로 관리한다. 요청이 끝나면 원래 closure와 임시 promise handle을 정리한다. callback에는 별도 arena scope를 주고 종료 뒤 caller arena를 복원한다. callback 실패는 rejected promise로 전달하며 완료 promise를 전역 표에 쌓지 않는다.

동시 yield에서 callback이 겹치는 실패를 먼저 확인한 뒤 timer dispatch를 직렬화했다. callback scope 할당 실패가 caller scope를 잃는 실패도 먼저 재현하고, 할당이 성공한 뒤에만 scope를 바꾸며 promise를 reject하고 다음 작업을 계속하도록 고쳤다. 이는 HTTP worker를 1개로 줄이는 우회가 아니다.

```bash
CPATH=<가림> LIBRARY_PATH=<가림> L08_TIMER_OUT=<가림> strace -f -e trace=execve -o <가림> bash scripts/verify-l08-timer-self.sh
bash scripts/audit-l07-execve.sh <가림>
```

```text
TIMER_PROGRAM_EXIT=0
TIMER_EXPLICIT_EXIT=7
EXIT_CANCELS_TIMER=PASS
CAPTURED_FILE_AFTER_CHURN=PASS
TIMER_ATOM_HTTP=PASS
TIMER_ONCE=PASS
TIMER_HTTP_SHUTDOWN=PASS
TIMER_WITHOUT_FURTHER_HTTP=PASS
TIMER_MEMORY_BOUNDARIES=PASS
timer-memory_PROGRAM_EXIT=0
TIMER_DISPATCH_SERIALIZATION=PASS
timer-serialization_PROGRAM_EXIT=0
TIMER_SCOPE_OOM_RESTORES_CALLER=PASS
TIMER_SCOPE_OOM_REJECTED=PASS
TIMER_SCOPE_OOM_RECOVERY=PASS
timer-arena-oom_PROGRAM_EXIT=0
L08_TIMER_PARTIAL_PASS=9 FAIL=0
TIMER_SELF_EXIT=0
NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0
```

[수정 전 API 실패](evidence/l08-platform/timer-before.txt), [동시 dispatch 실패](evidence/l08-platform/timer-serialization-before.txt), [scope 할당 실패](evidence/l08-platform/timer-arena-oom-before.txt), [자체 시험 원문](evidence/l08-platform/timer-after.txt). native memory 보조 시험은 1024개의 promise 참조·요청 arena 재사용·caller scope를 확인한다. 이 C 보조 시험만으로 완료를 판정하지 않았으며 `.fl` CLI와 실제 HTTP를 함께 실행했다.

최초 churn 사례의 파일 확인은 추가 HTTP 요청들이 있었다. 이를 무방문 증거로 사용하지 않는다. 별도 새 서버·빈 root에서 POST 1회 이후 추가 HTTP 없이 파일 생성만 확인한 [독립 probe](evidence/l08-platform/timer-no-visit.txt)가 `TIMER_NO_VISIT_EXIT=0`이다. 보강한 자체 driver에도 이 probe를 포함했다. [최초 5묶음 출력](evidence/l08-platform/timer-initial-after.txt)은 이 보강 전 범위다.

## sanitizer와 시험 의존성

최초 ASan/UBSan 빌드는 시스템 linker script가 가리키는 runtime 파일이 없어 링크 종료 `1`이었다. [실패 원문](evidence/l08-platform/timer-memory-sanitizer-link-before.txt). 공식 Fedora HTTPS 배포의 libasan·libubsan `15.2.1-2.fc43` 패키지를 시험용 임시 영역에만 풀어 다시 링크했다. 시스템 설치·sudo·/etc 변경은 하지 않았다. compiler는 `14.3.1`이다. [패키지명·SHA-256·LICENSE 원문](evidence/l08-platform/timer-sanitizer-packages.txt)을 보존했다. 이 라이브러리는 시험 의존성이며 제품 runtime에 추가하지 않았다.

추가 native memory fixture는 `TIMER_MEMORY_ASAN_UBSAN_EXIT=0`이다. [출력](evidence/l08-platform/timer-memory-sanitized.txt). `detect_leaks=0`, `halt_on_error=1`로 invalid access·undefined behavior를 검사했다. **누수 검사 통과를 뜻하지 않는다.**

이후 같은 소스의 native 보조 시험 3개와 실제 `.fl` CLI·무방문 HTTP까지 instrument한 별도 driver가 `L08_TIMER_SANITIZER_PASS=5 FAIL=0`, `TIMER_SANITIZER_EXIT=0`으로 끝났다. `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다. [출력 원문과 strace 감사](evidence/l08-platform/timer-sanitizers-after.txt). 명령은 `CPATH=<가림> LIBRARY_PATH=<가림> L08_TIMER_NATIVE_OUT=<가림> L08_TIMER_SANITIZER_OUT=<가림> L08_TIMER_SANITIZER_LIBDIR=<가림> strace -f -e trace=execve -o <가림> bash scripts/verify-l08-timer-sanitizers.sh`다. `TIMER_LEAK_CHECK=UNEXECUTED`이며, 이 결과도 전체 C19·C20 완료로 확대하지 않는다.

## 전체 앱과 남은 일

source 복사본 전체 C 변환은 `BLOG_AFTER_TIMER_TRANSLATE_EXIT=0`, C 구문 검사는 `BLOG_AFTER_TIMER_C_SYNTAX_EXIT=1`이다. 미연결 이름은 tcp_drop·tcp_write·tcp_server_tls·tcp_server_raw·tcp_server_stop·auth_verify_password의 `6`개로 줄었다. [원문](evidence/l08-platform/blog-after-timer-compile.txt). 앱 링크·실행 성공은 아니다.

최신 timer/GC 전체 시험은 `ef5e597c21b875d29e889514fb314232431600fa`에서 실행 중이다. 이전 HTTP/feed/client 전체 통과를 이 코드에 적용하지 않는다. native pending timer cap은 `4096`, delay는 Node timer 범위로 제한한다. 종료한 HTTP runtime의 재사용, 모든 allocation 실패 경로, RC cycle·기존 atom 영구 보존 정책·전체 동시성/누수는 아직 미확인이다. C20에서 확인·보완할 항목이며 메모리 안정성 전체 완료로 집계하지 않는다.

TCP·TLS 5개 API와 인증 verify의 결정 대기가 남아 C19/L08 전체는 미완료다. 실제 블로그 로그인·댓글·다중 작성자·브라우저·화면·운영 데이터 호환·백업/복구·운영 안정성도 별도다. 서버 23·25 접속·운영 데이터 변경·합치기·배포는 하지 않았다.
