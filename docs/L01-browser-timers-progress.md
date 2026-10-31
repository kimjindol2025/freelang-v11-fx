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
