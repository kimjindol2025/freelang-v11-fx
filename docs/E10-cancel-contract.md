# E10 비동기 취소 고정 계약

RUN-20261102-01. 이 계약·20개 정답·.fl·서버·C 관측기를 구현 전에 커밋한다.

`browser-http!`의 인자와 오류 결과 계약은 유지하고 반환값을 nil에서
불투명 작업 handle로 확장한다. E09 정답은 변경하지 않는다. HTTP 시작은
0 지연 비동기 작업으로 예약하므로 반환 직후 취소하면 fetch 자체를 시작하지 않는다.
`browser-after!`는 기존 불투명 handle 계약을 유지한다.

`(browser-cancel! handle)`은 두 종류의 handle에 공통이다. pending HTTP·
타이머를 취소하면 true, 이미 취소했거나 완료·callback 실행을 시작했으면 false다.
취소 사실은 이 명시적 반환값으로 관측하며 조용한 성공·실패로 처리하지 않는다.
취소가 true였던 작업은 성공·서버 오류·network·timeout callback 모두 0회다.
실행 중인 JS callback은 소급 취소하지 않는다. 잘못된 대상·인자 수는 동기 오류
문자열 `browser-cancel!: one task handle required`이며 다른 작업을 변경하지 않는다.
HTTP의 AbortController·시작 예약·timeout을 정리하고 callback 참조를 해제한다.

20개 고정 정답: 타이머/HTTP 시작 전 취소, 각각 두 번 취소, handle 동일성,
잘못된 nil/대상/인자 수의 정확한 오류 문자열, 서버가 시작을 확인한 HTTP 취소와
두 번 취소, 타이머+HTTP 혼합 취소, 취소 뒤 새 요청, HTTP 성공/서버 오류/
timeout 뒤 취소, 타이머 callback 안/완료 뒤 취소, 시작 전 취소 요청의 서버 수 0,
취소 callback 수 0, 정상 callback 수 4다. 마지막 proof는 모든 사례 뒤 1200ms
정착 기간과 서버 계수 확인 후 게시한다. 정착 시간·정답은 결과에 맞춰 바꾸지 않는다.

메모리: .fl에서 callback이 probe map을 캡처하는 타이머 200개·HTTP 200개를
취소하고 handle은 계속 보존한다. 실제 진행 중 HTTP 두 개도 probe를 캡처한다.
Chrome GC 뒤 heap snapshot의 E10_CANCEL_PROBE property edge는 의도한 live-control
1개만 남아야 한다. 취소된 callback의 잔류 probe는 0이다. snapshot metadata에서
property edge 형식을 읽고 수를 세며 string 상수 잔류를 객체 누수로 세지 않는다.
GC 전후 heap used bytes도 원문으로 기록하되 전체 heap 차이에 합격 임계값을
새로 추정하지 않는다. E08의 native ASan·LSan 누수 0 관문도 별도로 유지한다.

브라우저 관측은 기존 C CDP transport로 수행하고 JS 주입·Runtime.evaluate를
사용하지 않는다. .fl 검증·상태·완료 중복 방지와 표준 host primitive만 사용한다.
무거운 시험은 ps 확인 후 공용 flock 아래 실행한다. 수정 전 실패는 증거로
보존하고 수정 후 같은 관문 세 번째 실패에는 중단한다. 제품 자동저장 연결은 제외한다.

bootstrap은 사용자 확인대로 strace 없이 같은 seed·빌드·정답·고정점 검사를
수행한다. 실행 감사는 정적 감사로 구분하고 동적 실행 계수를 0으로 주장하지 않는다.
