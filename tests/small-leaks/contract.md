# E08 tail 오류 복사 누수 고정 계약

원본 언어 HEAD: 6093c151dd63bf91408214db39b4406f5ac797f2.
두 .fl과 stdout 정답은 기존 tail 회귀 원문을 그대로 고정한 복사본이다.
오류 메시지·호출 문맥·꼬리 재귀·반환값 수명 정답을 변경하지 않는다.

| 시험 | stdout | 종료 | ASAN/UBSAN/LSAN stderr | 보고된 누수 |
|---|---|---|---|---|
| tail-call-context | 고정 정답과 완전 일치 | 0 | 0바이트 | 0 byte / 0 allocation |
| tail-call-extra | 고정 정답과 완전 일치 | 0 | 0바이트 | 0 byte / 0 allocation |

검사는 detect_leaks=1:halt_on_error=1, UBSAN halt_on_error=1이며,
LSAN 오류 종료 23, 시간 제한 60을 유지한다. 억제·예외·기대값 변경은 금지한다.
before 모드는 두 실패 원문을 한 번씩 수집하므로 driver 종료 0을 통과로
읽지 않는다. after 모드는 각 고정 계약이 깨지면 즉시 중단한다.
빌드만 strace로 추적하고 sanitizer 실행은 ptrace 밖에서 한다.

E06 시작점 비교는 그 커밋의 compiler·runtime·원본 시험을 사용한다.
현재 새 시험 추가나 runtime 수정 내용을 과거 복사본에 섞지 않는다.
