# C20 실제 HTTP 요청 사이 atom 상태

실제 loopback HTTP server를 `FL_WORKERS=4`로 실행하고 curl 10개 동시 과정으로 POST /increment를 1000번 보낸다. 응답 1000개 모두 HTTP 200, 최종 counter 1000, 서로 다른 응답 값 1000개와 정확한 값 집합 1~1000이 정답이다. 원자적 swap!이 없으면 중첩 요청이 같은 이전 값을 덮어쓰므로 중복 값과 유실 증가가 발생한다.

C helper는 첫 요청 4개의 barrier와 pthread identity·handler 겹침 측정, callback의 짧은 pause만 제공한다. 실제 4 worker와 handler 겹침 4를 확인한다. counter 자체와 증가는 `.fl`에서 수행한다. 쓰기 뒤 별도 GET 10회에서도 값이 유지돼야 한다. 이 counter는 WAL을 사용하지 않는 휘발 상태이며 시험 server 새 과정 시작 뒤 0으로 초기화되는 것이 정답이다. 이를 blog 저장 자료의 재시작 보존으로 집계하지 않는다.

빈 포트는 bind(0)으로 OS가 선택하며 기존 test-only wrapper로 loopback에 제한한다. 자기 PID의 시험 server만 중지한다. 원본·운영 자료는 쓰지 않는다. runtime source root를 지정하는 시험 인자로 수정 전 고정 checkout과 수정 후 checkout을 각각 검증하며 정답은 바꾸지 않는다. 이 HTTP 언어 자체 시험은 DB 거래·WAL 일관성·blog 1000글/1000이미지 부하·p95 목표의 증거가 아니다.

후속 요청 수명 검증: 별도 POST 100개에서 각 100개의 self-cycle atom을 만들고 실제 free 계측으로 요청 종료 뒤 live=0을 요구한다. `/save`의 atom 하나는 전역 keeper를 통해 유지돼 별도 `/saved` 요청에서 한글·이모지를 읽어야 하며 live=1이어야 한다. `/clear` 이후에는 다시 live=0이다. 총 생성 계측 10001개, 원래 증가 요청·상태·고유 값·worker·재시작 정답은 유지한다. 요청 종료와 관찰 요청의 경합 때문에 live 값은 최대 100회×0.05초 범위에서 정답까지 확인하되, 끝까지 도달하지 못하면 실제 값을 기록해 실패시킨다. 이것은 native worker arena의 실제 수거 확인이며 모든 malloc의 LSan 검증을 대신하지 않는다.
