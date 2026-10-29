# C20 arena 버퍼 보존 상한과 실패 후 정리

HTTP worker는 요청마다 fl_arena_begin/end로 같은 arena를 재사용한다. 기존 allocator는 다음 청크가 작을 때 새 청크를 앞에 삽입하고 요청 종료 시 모든 청크를 남긴다. 점점 큰 요청이 반복되면 이전 큰 버퍼의 합계가 계속 쌓인다. 이 문제는 살아 있는 값의 누수와 별개인 cache 보존 문제다.

정답 정책: 경계 정리 후에는 원래 첫 64 KiB 청크만 재사용하고 나머지 청크는 해제한다. 현재 요청의 데이터는 해당 경계 전까지 유지하며, 자식 arena를 정리해도 부모의 큰 버퍼는 유효해야 한다. 청크 확보 실패 뒤의 fallback malloc도 성공하면 arena가 소유해 경계에서 해제해야 한다. fallback 정리 등록까지 실패하면 해당 할당을 되돌리고 NULL을 반환한다. size 정렬/청크 크기의 overflow 요청은 쓰기 없이 NULL로 거부한다.

`.fl` callback이 64 KiB부터 4 MiB까지 64개 문자열을 만들고 C helper가 전체 payload를 확인한다. fl_arena_alloc 안에서 실행된 malloc과 실제 free만 link wrapper로 계측한다. scope를 먼저 만들고 첫 청크를 준비하므로 첫 청크는 계측 대상이 아니며, 경계 뒤 추가 청크의 보존 바이트 정답은 0이다. 단순 RSS 감소에 의존하지 않는다. 작은 할당의 주소 재사용·부모 버퍼 보존·첫 malloc 실패·두 번 실패·정리 등록 실패도 고정한다. source runtime 정책을 바꾸기 전에 `.fl`·C 계측·정답을 커밋한다.

이 시험은 compiler-generated `.fl` callback과 실제 native allocator를 연결한다. 실제 HTTP 업로드·기존 L06/L07·bootstrap은 별도 회귀로 확인하며, 이를 blog 1000글/이미지의 장시간 RSS·p95 목표 완료로 집계하지 않는다. 사용자 데이터·백업·다른 server/process는 사용하지 않는다.
