# E07 — CLI 수명 검증 후 중단 보고

상태: `RUN-20261101-01`의 같은 `E06-REGRESSION` 관문 `3/3`으로 중단.
구현·재시험은 중단했고 증거·상태·보고·정상 푸시만 수행한다.
WP 플랫폼 완료가 아니며 사장님 완료 판정도 없다.

## 확인한 것

언어 시작 HEAD: `e9829396e8d4709ef968421cbc46d8d842db81ca`.
정답 선행 커밋: `af5c060339c06f669c8fe66929f2e2c8d0e54f61`.
CLI 수명 구현: `cbb4ddeddf952b648a16884f15460296961bde4f`.
시험 도구 최종 커밋: `4ed4d662cbbdc7548f4cbd64e813bb8fcdb5bc38`.

생성 native main에서 CLI arena를 시작하고 timer 종료 처리 뒤 정리하도록
연결했다. 문자열 연결 임시값도 arena를 사용한다. `.fl` 정답을 먼저
커밋했으며 기존 snapshot fixture와 정답은 그대로 재사용했다.
기존 `fl_init_argv` embedding 계약과 고정 bootstrap seed는 변경하지 않았다.

수정 전 기존 snapshot 원문:

```text
SUMMARY: AddressSanitizer: 150834 byte(s) leaked in 6430 allocation(s).
```

종료 `23` 원문은 기존 E06 증거에 보존했다. 이번 CLI 자체 시험은
ASAN/UBSAN/LSAN 활성 상태로 다음 고정 정답에 통과했다.

| 사례 | 실제 종료 | 정답 종료 |
|---|---:|---:|
| 기존 snapshot .fl | 0 | 0 |
| 정상 종료·snapshot·catch·timer capture | 0 | 0 |
| 잡히지 않은 예외 | 1 | 1 |
| 명시적 종료·예약 작업 취소 | 7 | 7 |

```text
COMPILER_BUILD_EXIT=0
CLI_LIFETIME_EXIT=0
CLI_LIFETIME=4/4
NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0
```

실제 Chrome 재검증은 타이머 `20/0`, 예외 검사 `3/0`, 정상 시 관측 예외
`0`, 의도한 callback 예외 `1`이었다. 화면은 `768×1024`다.
`timers-768.png`와 `exception-768.png`는 언어 자체 시험 화면이다.
후자는 화면 PASS 두 개와 CDP 예외 관측 검사 한 개로 `3/0`이다.
블로그 제품 화면 완성 증거로 확대하지 않는다.

승인된 J안의 compiler 생성 JS를 Chrome에서 실행했다. 새 수작업 JS·TS·
Python은 작성/실행하지 않았다. `JS=0`은 별도 JS 실행기 수이며 브라우저
내부 생성 JS 실행이 없었다는 뜻이 아니다. JS 주입은 `0`이다.

정상 타이머 입력의 생성 로그에서는 unsupported operation 일치 `0`,
생성물 존재 PASS다. 전체 scenario 로그의 같은 문구 `2`개는 별도
`native`·`eval` 거부 fixture의 예상 종료 `1`이다.
`timer-generation.txt`와 `final-metrics.txt`에
계수 원문과 생성물 SHA-256을 보존했다.

## 전체 회귀와 오류 주입기 수정

첫 전체 실행에서 기존 브라우저 의미·이벤트·DOM·블로그 모델, CLI·시간대,
현재 native 자체 검사와 이전 L08 3차 `11/0`·2차 `11/0`·1차 `17/0`,
L06 HTTP `110/0`·의도한 timeout `2/0`·TLS `110/0`까지 통과했다.
이후 다음 원문으로 중단됐다.

```text
FAIL read-allocation-failure
L07_SELF_PASS=220
L07_SELF_FAIL=1
REGRESSION_CHAIN_EXIT=1
```

원드라이버의 UNEXECUTED 표시는 `8`, SKIP 표시는 `0`이다. 이는 미실행이
없다는 뜻이 아니다. 의도한 timeout 검사 `2/0` 외에 종료 `124` 표시는
`0`이었다. 별도 LSAN에서도 `12`사례가 남았고 최신 bootstrap은 미실행이다.
원문 계수는 `final-metrics.txt`에 보존했다.

소스 원인은 L07 오류 주입기가 `malloc`만 감싼 데 있다. 실제 파일 데이터
할당은 `fl_arena_alloc`인데 CLI 캐시가 있으면 그 크기의 `malloc`을 거치지
않는다. 정답 `ALLOCATION_FAILED`는 유지했다. 실제 할당 경계도 감싸고
미소비 주입을 덮어쓰면 실패하도록 강화했다. 앱·runtime·compiler·기존
`.fl`/`.expected`는 이 시험 도구 수정에서 바꾸지 않았다.

수정 후 실제 L07 원문:

```text
PASS read-allocation-failure
L07_SELF_PASS=221
L07_SELF_FAIL=0
L07_RUNTIME_REGRESSION=PASS
L07_AFTER_EXIT=0
```

첫 전체 실행의 strace `49`개와 수정 후 L07 trace 감사는 각각
`NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이었다. 이는 실행된 부분의 감사다.
LSAN은 ptrace 밖에서 실행하며 미실행 bootstrap까지 감사했다고 보지 않는다.

## 세 번째 미통과 원문과 원인

LSAN 보완 검증기는 실행 전에 커밋했다. 기존 정답·C 자체 assert를
재사용하며 첫 실패에서 중단한다. HTTP 정상 진단 stderr를 허용하는 근거는
기존 `http.c`와 기존 HTTP helper다. sanitizer 오류·비정상 종료는 거부한다.
이는 실행 전 검증기 정비이며 언어 정답 변경이 아니다.

동일 production source의 기존 ASAN ELF를 사용했다. 새 복사본과 이전
복사본의 `runtime`·`self`가 일치함을 확인했다.

```text
PRODUCTION_RUNTIME_AND_COMPILER_UNCHANGED=PASS
atom-snapshot-atom-snapshot_LSAN_EXIT=0
SUMMARY: AddressSanitizer: 262464 byte(s) leaked in 8 allocation(s).
atom-snapshot-atom-snapshot-memory_LSAN_EXIT=23
STRICT_LEAKS_EXIT=1
```

14개 계획 사례 중 `1` 통과·`1` 실패·`12` 미실행이다. 실패 사례는
기존 C 자체 시험 `atom-snapshot-memory`다. 원문의 직접 누수는 `224`
바이트/`4`객체, 간접 누수는 `262240`바이트/`4`객체다. 할당 stack은
`fl_arena_begin`과 reader thread를 가리킨다.

소스 분석: reader 네 개가 `fl_arena_begin/end`를 반복한 뒤 반환한다.
`fl_arena_end`는 재사용을 위해 캐시를 보존하며 thread 종료 시 그 캐시를
회수하는 처리가 없다. 이번 CLI 정리는 이 worker 수명을 해결하지 못했다.
worker 정리는 구현하지 않았다. C fixture에 임시 해제 호출을 넣거나
LSAN을 끄는 우회도 하지 않았다. HTTP의 실제 LSAN 사례는 도달 전 중단돼
미실행이며, HTTP 누수를 관측했다고 보고하지 않는다.

| 실행 기록 | 같은 관문 결과 | 연속 미통과 |
|---|---|---:|
| RUN03-A04 | 기존 CLI snapshot LSAN 종료 23 | 1/3 |
| RUN03-A06 | 전체 회귀 L07 220/1 | 2/3 |
| RUN03-A07 | L07 수정 후 통과, 별도 LSAN worker 종료 23 | 3/3 |

부분 통과로 연속 횟수를 초기화하지 않았다. 과거 RUN02 `3/3`과 실행 이력도
변경하지 않았다. 세 번째 미통과 뒤 남은 사례·전체 드라이버·bootstrap은
실행하지 않았다. 제품/언어 재시험 없이 JSON의 중단 상태와 이력 보존만
읽기 검사했다. 언어 수정은 이 시점 이후 없다.

## 미확인과 재개 조건

- worker arena 자동 정리 구현과 일반 누수 보완 나머지 `12`사례.
- 최신 CLI compiler/runtime의 전체 회귀 통과 및 native bootstrap.
- 최신 source의 stage2/3/4 SHA·CMP_2_3·CMP_3_4와 bootstrap 실행 수.
- WP 단계 완료와 사장님 닫기 판정. D04는 미결정이며 자동저장은 미착수.

이전 source의 bootstrap 통과를 최신 source의 결과로 전용하지 않는다.
최신 source 기준 E06은 회귀와 bootstrap 묶음이 미통과/미확인이므로 `4/6`이다.
다음 언어 범위는 thread 종료 시 arena 캐시를 자동 회수하는 수명 계약과
자체 시험이다. 요청 사이 캐시 재사용과 기존 borrow 계약은 보존해야 한다.
사용자 지시의 `같은 체크포인트 3회 연속 미통과 시 즉시
중단·보고` 기준에 따라 새 구현·재실행은 시작하지 않는다.

## 보존·푸시

원문·화면·trace digest는 `cli-lifetime` 증거 묶음에 보존했다.
`queue-deferred.txt`와 `checkpoints.txt`는 이전 제출의 대기·검사 이력이다.
최신 중단 상태는 `stop-state.txt`와 `checkpoints-stop.txt`다. 이전 대기 종료
`143`은 compiler/시험 시작 전의 대기 과정 종료여서 미통과로 세지 않았다.
주소·경로는 `<가림>`, 수치·SHA는 출력 그대로다. 기존 E06 원문과 백업은
삭제하지 않았다. 서버 23·25 접속, 운영 자료 접근/변경, 합치기·배포·
force-push·sudo·시스템 설정 수정은 수행하지 않았다.

코드 제출 시 Forgejo 먼저 → GitHub 다음 푸시했고 확인한 HEAD는 각각
`4ed4d662cbbdc7548f4cbd64e813bb8fcdb5bc38`,
`4ed4d662cbbdc7548f4cbd64e813bb8fcdb5bc38`이다.
이 보고서·증거를 포함한 최종 양쪽 HEAD는 푸시 후 최종 응답에 제출한다.
