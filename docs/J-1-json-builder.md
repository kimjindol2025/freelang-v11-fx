# J-1 native JSON builder

접수번호: RUN-20261102-02. 검수 창.

시작 FX HEAD: 7379dd85c5960893becb9eecfbb67ee77dda0990
시작 blog1 HEAD: e1980fc8e24d07ecb52a08653ea688683212ad3a
두 작업 트리 CLEAN, 검수 기록과 일치. E10 시험 프로세스 없음.
경로·주소·키: <가림>. 무거운 시험 전 ps 기록, 공통 flock 아래 실행.

수정 전 측정: `evidence/j1-json-builder/before.txt`.
입력은 0으로 채운 배열 1000/10000/100000개. 두 API 각각 별도 프로세스.
고정 측정 조건: 가상 메모리 2097152 KiB, timeout 120초. 기존 관문 임계값은 변경하지 않음.
원문 시간·최고 RSS·종료 코드를 보존. 100000개는 두 경로 모두 종료 139;
측정 한도 내 미완료이며 실행 완료 시간으로 해석하지 않음.
GNU time의 신호 종료 시 EXIT=0 출력과 별도로 MEASUREMENT_EXIT=139가 실제 셸 종료 코드.
첫 측정기의 v11 숫자 태그 확인 실수는 `before-probe-1.txt`에 보존.
v11의 기존 float 숫자 표현도 허용하도록 측정기를 수정한 뒤 언어 수정 전 재측정.
값 0, 개수, 불변 결과 확인은 두 경로 동일.

J1_SUMMARY=0/6
