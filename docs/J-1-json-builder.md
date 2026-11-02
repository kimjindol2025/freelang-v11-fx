# J-1 native JSON builder

접수번호: RUN-20261102-02. 검수 창.

시작 FX HEAD: 7379dd85c5960893becb9eecfbb67ee77dda0990
시작 blog1 HEAD: e1980fc8e24d07ecb52a08653ea688683212ad3a
두 작업 트리 CLEAN, 검수 기록과 일치. E10 시험 프로세스 없음.
경로·주소·키: <가림>. 무거운 시험 전 ps 기록, 공통 flock 아래 실행.

## 선커밋

수정 전 측정 커밋: dda1dd09edba533908f3db220a92236df78a6f4f
실패 경로 정답 선커밋: fcdaff9dd8b5af419a08def8c04449407c6d385e
이 두 커밋에는 runtime 수정 없음. 정답과 측정 입력은 구현 후 변경하지 않음.

## 측정 원문과 해석

증거 파일: before.txt, before-probe-1.txt, after.txt.
입력은 0으로 채운 배열 1000/10000/100000개. 두 API 각각 별도 프로세스.
고정 측정 조건: 가상 메모리 2097152 KiB, timeout 120초. 기존 관문 임계값은 변경하지 않음.
원문 시간·최고 RSS·종료 코드를 보존. 수정 전 100000개는 두 경로 모두 종료 139;
측정 한도 내 미완료이며 실행 완료 시간으로 해석하지 않음.
GNU time의 신호 종료 시 EXIT=0 출력과 별도로 MEASUREMENT_EXIT=139가 실제 셸 종료 코드.
첫 측정기의 v11 숫자 태그 확인 실수는 before-probe-1.txt에 보존.
v11의 기존 float 숫자 표현도 허용하도록 측정기를 수정한 뒤 언어 수정 전 재측정.
값 0, 개수, 불변 결과 확인은 두 경로 동일.

| API | 항목 | 수정 전 parse 초 | 수정 후 parse 초 | 수정 전 process 최고 RSS KiB | 수정 후 process 최고 RSS KiB |
|---|---:|---:|---:|---:|---:|
| legacy | 1000 | 0.005535366 | 0.000091292 | 14060 | 5456 |
| legacy | 10000 | 0.471204865 | 0.001198108 | 800256 | 6004 |
| legacy | 100000 | 미완료, 종료 139 | 0.007306987 | 2054252 | 8844 |
| v11 | 1000 | 0.007588252 | 0.000309623 | 13420 | 5404 |
| v11 | 10000 | 0.520217792 | 0.002841057 | 823880 | 6048 |
| v11 | 100000 | 미완료, 종료 139 | 0.036938665 | 1487980 | 10568 |

1만→10만 입력 10배에 시간은 legacy 약 6.1배, v11 약 13.0배.
단일 실행의 스케줄링·캐시 변동을 포함한 측정이며, 엄격한 성능 임계값을 신설하거나 바꾸지 않았다.
구조적으로 기존 builder의 용량 배증 push와 마지막 1회 freeze로 배열 누적은 상각 O(n).
객체 key 삽입 비용은 이번 범위 밖.

## 구현·실패 경로

두 API의 기존 builder new/push/freeze를 재사용. 새 JSON 해석기·API 병합 없음.
legacy는 기존의 부분 배열·nil 허용 동작까지 유지하며, 배열 반환 전에 freeze하여
상위 parse의 trailing-input throw보다 먼저 builder를 해제한다.
v11은 구문 오류를 p.error로 전달하며 각 중첩 배열에서 builder data와 builder를 해제한다.
성공 시 결과는 기존 불변 vector. JSON 숫자·문자·오류 메시지·제한은 유지.

고정 시험 9개 × API 2개 × 반복 100회. 중간 잘못된 문자, 잘린 배열,
중첩 배열·객체 뒤 오류, escape 오류, 정상·빈 배열 포함.
수정 전후 모두 JSON_FAILURE_CASES=18 FAIL=0, JSON_FAILURE_ASAN_UBSAN_LSAN=PASS.
오류 후 arena reset와 프로세스 정리까지 sanitizer stderr 비어 있음, 누수 보고 0.
메모리 할당 실패 주입 시험은 수행하지 않았으며 구문 오류 해제 검증과 구분한다.
증거: failures-before.txt, failures-after.txt. sanitizer 실행에는 strace 없음.

## 빌드 안내

README 첫 빌드 안내를 검증기가 사용하는 native 함수로 변경.
기존 C seed → 현재 .fl compiler → 앱 순서와 개발 라이브러리를 명시.
AFJ·Python 의존 경로는 호환용(옛 경로)으로 표시.
fl-build.sh, build.sh 동작 수정 없음. HTTP·타이머·생성 내장 데이터 수정 없음.
검증기 통합 없음. seed 교체 없음.

## 남은 확인

기존 JSON 시험, 전체 회귀, E09 HTTP, E10 취소, strace 없는 native bootstrap 실행 중.
bootstrap 실행 감사는 승인된 정적 감사로 분리하며 동적 실행 횟수를 0으로 주장하지 않는다.
Forgejo→GitHub 푸시·원격 HEAD 일치·최종 CLEAN 확인은 아직 미완료.

J1_SUMMARY=4/6
