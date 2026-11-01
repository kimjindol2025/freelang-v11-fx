# E07 — native CLI 메모리 수명 후속(검증 미완료)

## 범위와 승인

사장님 `가자`에 따라 E06 일반 누수 실패의 후속 언어 수정을 진행했다.
앱 우회 없이 fx compiler/runtime의 CLI 임시값 정리만 다룬다.
D04는 미결정이며 자동저장은 시작하지 않았다. WP 단계는 사장님 판정 전
닫지 않는다. 서버 접근·운영 변경·합치기·배포는 수행하지 않았다.
경로·주소는 `<가림>` 처리한다.

## 확인한 것

- 시작 언어 HEAD: `e9829396e8d4709ef968421cbc46d8d842db81ca`.
- 수정 전 실패: 기존 snapshot fixture, `exit 23`,
  `SUMMARY: AddressSanitizer: 150834 byte(s) leaked in 6430 allocation(s).`
  E06 원문은 유지했다. 같은 실패를 다시 실행하지 않았다.
- 정답 선행 커밋: `af5c060339c06f669c8fe66929f2e2c8d0e54f61`.
  기존 snapshot 정답 유지, 정상·잡히지 않은 예외·명시적 종료 정답 추가.
- 구현 커밋: `cbb4ddeddf952b648a16884f15460296961bde4f`.
  생성 native main에서 CLI arena 시작, timer atexit 처리 뒤 root와 남은
  중첩 scope 해제. 문자열 연결 임시값도 같은 arena를 사용하도록 변경했다.
  기존 `fl_init_argv` embedding 계약·고정 bootstrap seed는 바꾸지 않았다.
  이것은 코드 변경 내용이며 실제 누수 해결 판정은 아니다.
- 커밋 전 전체 diff 수동 검토 및 민감정보 패턴 검색 결과:
  `SENSITIVE_DIFF_PATTERNS=0`, `MANUAL_DIFF_REVIEW=PASS`.
- 언어 구현 커밋은 Forgejo 먼저, GitHub 다음 푸시했고 두 HEAD 모두
  `cbb4ddeddf952b648a16884f15460296961bde4f`로 확인했다.
  이 보고서의 최종 커밋 SHA는 푸시 후 별도 보고한다.

## 실제 검증 상태

`ps` 확인 후 `flock -o <가림> bash <가림>`으로 검증을 요청했다.
다른 부하 시험이 잠금을 보유해 `WAIT_ELAPSED=25:15` 동안 시작하지 못했다.
잠금을 우회하지 않고 제 대기 과정만 종료했다. 대기 명령 종료 `143`은
시험 종료 코드가 아니며, 수정 후 시험 실패로 세지 않는다.
다른 과정 종료는 `0`. 실제 원문은 `queue-deferred.txt`에 보존했다.

```text
COMPILER_STARTED=0
CLI_TESTS_EXECUTED=0
REGRESSION_EXECUTED=0
BOOTSTRAP_EXECUTED=0
FAILURE_COUNT=1/3
OWN_QUEUE_TERMINATED=1
OTHER_PROCESSES_TERMINATED=0
QUEUED_PROCESS_GONE=PASS
NO_COMPILER_OR_TEST_OUTPUT=PASS
```

실행은 기존 `RUN-20261101-01`을 유지한다. `E06-REGRESSION`의 연속 미통과
`1/3`은 그대로다. 시도 A06은 실행 대기에서 보류됐으므로 완료된 시도에
추가하거나 통과로 세지 않았다. 과거 실행 기록도 바꾸지 않았다.

## 미확인과 재개 순서

CLI 수명 수정의 빌드·실행, 누수 해결 여부, 기존 전체 언어 회귀,
최신 stage2/3/4 bootstrap 및 금지 실행 수는 모두 미확인이다.
이번 변경에 대한 브라우저 재시험·새 화면 증거도 없다.
LSAN 미실행 범위를 해소하지 않았으므로 E06 `5/6`과 WP 미완료를 유지한다.

잠금 사용이 가능해지면 고정 소스 복사본에서 native compiler를 빌드하고
`verify-cli-lifetime.sh`를 실행한다. ASAN/UBSAN/LSAN 활성 상태의 snapshot·
정상·예외·exit 네 사례가 모두 통과해야 한다. LSAN 자체는 ptrace 밖에서,
빌드 실행 감사는 strace로 구분한다. 이어서 기존 회귀와 누수 보완,
실제 Chrome·고정 정답, native bootstrap 고정점과 실행 수를 확인한다.
이 전체 관문을 통과하기 전에 부분 통과로 기존 `1/3`을 초기화하지 않는다.
수정 후 같은 관문 세 번째 연속 미통과면 즉시 중단한다.
