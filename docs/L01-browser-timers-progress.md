# L01 브라우저 타이머: 수정 전 실패와 중단

## 확인한 것

2026-11-01, 정답 선행 커밋 `b236e76d627864902dcb1c06888ef63f308bc91e`의 `.fl` 시험을 native compiler의 `--browser-js`로 한 번 실행했다. compiler 캐시는 `5859f61fd4e8fc310c2f021b5fedad1403de82d4`에서 만들었으며 기준 커밋과 `self`·`runtime`의 차이가 없음을 `git diff --quiet` 종료 코드 `0`으로 확인했다. 실행 대상은 추적된 소스만 추출한 복사본이며 운영 자료는 포함하지 않았다.

실패 원문:

```text
Uncaught error (line 440): browser target: unsupported operation
TIMER_BASELINE_EXIT=1
TIMER_GENERATED_OUTPUT=ABSENT
NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0
```

[전체 기록](evidence/l01-browser-timers/before.txt). `runtime/browser-target.h` 공개 builtin 표에 `browser-after!`·`browser-cancel!` 연결이 없고, `.fl` 지원 소스에도 그 구현이 없다. 생성 단계에서 중단됐으므로 20개 정답을 실행 실패 20건으로 집계하지 않는다. 실제 실패는 검증 호출 1회이며 Chrome 실행은 0회다.

블로그 실행 `RUN-20261031-02`의 기존 미통과 2회에 이번 1회를 더해 `3/3`이다. 기존 규칙은 수정 전 양성 시험 실패도 포함한다. 집계 변경에 대한 사용자 답변이 없으므로 변경하지 않고 즉시 구현·재시험을 중단했다. 이후 작업은 증거·중단 보고서 기록과 해당 작업 가지 푸시뿐이다.

## 해결 방향과 미확인

재개 시 `.fl` 지원 소스에서 불투명 handle·타입/범위 검사·pending/fired/cancelled 상태·callback 참조 해제를 구현하고, compiler 출력기에는 표준 timer 호출/취소 primitive만 연결한다. 생성 JS를 손으로 쓰거나 앱 우회로 구현하지 않는다. 두 API 연결과 `.fl` 지원 소스가 함께 필요하며 이름 표 추가만으로 완료되지 않는다.

미확인: 수정 후 정답 20개, callback 예외 관측, 실제 Chrome, 생성 재현성, 기존 browser/native 회귀, bootstrap. 이번에 모두 미실행이다. 타이머·자동저장·플랫폼 완료가 아니다. 기존 내장 JS는 유지했고 이번에는 새 JS·TS·Python을 작성/실행하지 않았다. 서버 23·25 접속·운영 자료 변경·합치기·배포·강제 푸시를 하지 않았다.

## E06 재개: 기능 확인과 공통 회귀 보류

2026-11-01 사용자 승인으로 `RUN-20261101-01`을 `0/3`에서 시작했다. 위의 이전 실행 `3/3` 기록은 보존했다. 수정 전 실패는 증거만 보존하고 수정 후 같은 체크포인트가 3회 연속 미통과하면 중단하는 새 기준을 적용했다.

타이머 구현 소스는 `b993d7396aeb9c4b47e4df432e40df5004cdca3c`, 전체 회귀/확장 bootstrap 소스는 `2816e4e84bfca847eacccc6cc4ee71564c2f5182`다. .fl에서 인자 검사·불투명 WeakMap handle·한 번 실행/취소·callback/ID 참조 해제를 구현했다. C 출력기는 표준 timer 호출만 연결한다. 고정 시험/정답 20개와 stage0 seed는 변경하지 않았다.

확인한 것: 타이머 Chrome `20/20`, 별도 예외 fixture `3/0`, 정상 관측 예외 `0`·의도한 callback 예외 `1`, 생성물 두 번 일치. 기존 browser 의미 `37/0`·이벤트 `26/0`·DOM `31/0`·블록 모델 `21/0`, CLI `4/0`·실제 clock 3시간대도 통과했다. 원문/생성물 hash와 화면은 E06 증거 묶음에 보관했다. J안의 생성 JS를 Chrome에서 실행했으며 수작업 JS·TS·Python은 작성/실행하지 않았다. 별도 JS 실행기 감사 0을 브라우저 내부 JS 미사용으로 설명하지 않는다.

정식 기존 전체 회귀 드라이버는 종료 코드 `0`, assertion 실패 `0`, 시험 제어 시간초과 `0`이다. 원래 드라이버의 미실행 표기는 `8`개다. 생성 전용 Chrome 미실행 `1`개는 별도 실제 Chrome 시험으로 보완했지만 일반 누수 표기 `7`개가 남아 추가 확인했다.

기존 ASan native `atom-snapshot` ELF에 `detect_leaks=1`을 적용한 첫 추가 사례는 종료 `23`으로 실패했다.

```text
SUMMARY: AddressSanitizer: 150834 byte(s) leaked in 6430 allocation(s).
```

추가 검증은 1개 실행·1개 실패이며, 다른 누수 그룹 6개는 실행하지 않았다. 이 추가 실행의 execve 감사는 미실행이다. 기존 회귀/부트스트랩의 strace 감사와 별도다. 최초 출력 표시용 `head` 파이프는 SIGPIPE `141`이었고, 출력 전에 저장된 실제 프로그램 종료 파일 `23`을 재실행 없이 확인했다. 원문 전체는 보존했다.

관측된 스택에는 `fl_arena_alloc`의 arena 없는 malloc fallback과 `fl_vec_from`·`fl_str_val`이 있다. 관련 native core·CLI init/call context와 snapshot fixture는 시작 HEAD와 동일하다. 관측은 CLI fixture 범위이며 운영 서버 메모리는 측정하지 않았다. 최종 누수 원인/소유권·정리 시점은 확정하지 않았다. 이 작업에서 native GC·CLI 메모리 수명 코드를 변경하거나 LSAN을 끄고 통과로 바꾸지 않았다.

bootstrap은 `CMP_2_3=PASS`, `CMP_3_4=PASS`, 배열/잔여 과정 검사 통과다. 생성 C의 stage2/3/4 SHA-256은 모두 다음 원문이다.

```text
6f7c143734a8492fb6ae9ab77036f619c41915d0181a6f263e95985cf90ad423
NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0
```

미확인: 일반 누수 범위의 완전한 검증과 native CLI 메모리 수명 정리. 앱에 arena 호출을 넣거나 suppressions/기대값 변경으로 우회하지 않는다. 다음 언어 작업은 CLI 정상 종료·비정상 종료·예외·중첩 호출·snapshot/함수 값·스레드의 메모리 소유권 계약과 실패 .fl/정답을 먼저 고정하는 범위로 제안한다. 현재 타이머 지시 범위를 넘어 그 구현을 시작하지 않았다.

전체 회귀 인수는 보류이며 새로운 실행의 미통과는 `1/3`이다. 세 번째 실패로 중단한 상태가 아니고, 공통 관문과 다음 언어 수정 범위에 대한 사용자 검토를 기다린다. WordPress 앱 자동저장·D04 결정·단계 닫기·운영 자료 변경·합치기·배포는 수행하지 않았다.
