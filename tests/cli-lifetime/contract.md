# CLI 임시값 수명 정답

기존 atom-snapshot.fl/.expected를 원문 그대로 재사용한다. 수정 전 실제
LSAN 실패는 E06 leak-supplement.txt의 exit 23, 150834 bytes / 6430 allocations다.
이 실패는 이번 수정 전 증거이며 불필요하게 다시 실행하지 않는다.

추가 정답은 구현 전에 고정한다. normal은 snapshot·catch·문자열 연결·
main 반환 뒤 timer callback의 순서를 확인한다. uncaught는 runtime/error.c의
print_uncaught 및 cgc-main.fl의 throw line 계약에 따라 stdout·stderr·exit 1을
고정한다. 기존 timer-exit.fl은 예약 callback을 취소하고 exit 7, 출력 없음이다.
모든 경우 실제 .fl 컴파일 실행, ASAN/UBSAN/LSAN 활성, 누수·오류 보고 0건을
요구한다. suppressions·앱별 arena 주입·정답 수정은 금지한다.

검증기는 첫 실패에서 중단한다. 실패하면 같은 E06-REGRESSION의 연속 미통과에
합산한다(현재 1/3). 정상 종료 통과만으로 전체 회귀·bootstrap 완료 판정 금지.
LSAN은 ptrace 아래에서 실행할 수 없으므로 빌드만 strace하고, 실제 LSAN 실행은
별도로 기록한다. heavy lock과 ps 확인은 호출자가 수행한다.
