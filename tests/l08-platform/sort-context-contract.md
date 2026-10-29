# C20 중첩·동시 정렬의 콜백 문맥

v11 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4`의 src/eval-builtins.ts:3974~4010 소스를 읽었다. sort-by는 호출별 local keyFn·sbExtract를 사용하며 입력 배열 복사본을 정렬한다. 순수 숫자 key의 오름차순·음수 key의 내림차순과 중첩 key 호출에서 외부 callback 유지가 정답이다. Node는 실행하지 않는다.

현재 fx aliases.c는 전역 `_sort_by_fn`을 comparator가 매번 읽는다. callback 안의 다른 sort-by와 실제 8 native thread가 서로 callback을 덮어쓸 수 있다. TLS 전역 하나로 바꾸는 것만으로는 같은 thread의 중첩 정렬이 해결되지 않는다. 호출별 문맥을 사용해야 한다.

위 `.fl`·고정 정답으로 입력 보존·중첩·중첩 예외 catch·외부 예외 뒤 회복·빈 배열·8개 실제 thread별 반대 방향 20회 정렬을 확인한다. 시험 helper의 barrier와 짧은 pause만 사용하며 정렬 호출·단정은 `.fl`에서 수행한다. C 표준 라이브러리 정렬을 쓰면 comparator의 FreeLang 예외가 라이브러리 내부 cleanup을 건너뛰지 않도록 포착한 뒤 정상 반환 후 전파한다. comparator와 자기 배열 임시 버퍼를 전역 상태에 두지 않는다.

이는 숫자 key와 callback 문맥 검증이다. v11 전체 sort-by의 필드 이름 key·문자열 locale·동점 안정성·잘못된 인자 계약, 메모리 누수 검증과 C20 전체 완료를 이 시험만으로 선언하지 않는다.
