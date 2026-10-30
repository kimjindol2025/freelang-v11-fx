# FreeLang compiler lexer의 누적 배열 복사

고정 `2754d8f7d83ce520395ab1c2bfc99b6d72336379`의 bootstrap은 가장 작은 stage2 입력을 통과했지만 큰 자기 입력은 실제 CPU를 사용하며 12분에 RSS 42243432 KiB까지 증가했다. 이 수치만으로 무한 반복이나 완료를 단정하지 않는다.

self/cgc-main.fl의 emit은 매 token마다 기존 flat vector에 append한다. v11 호환 immutable append는 이전 vector를 보존해야 하므로 길이 N까지 prefix 총합 O(N²)을 할당한다. process main에서 이 중간 buffer들이 즉시 해제되지 않는다. 이를 증명하기 위해 실제 lex 함수에 같은 `(println 1)` 1000행(12000 byte)을 넣어 native malloc/calloc/realloc/free의 미해제 payload와 peak를 계측한다. meter bookkeeping·공유 라이브러리 내부 할당·RSS 전체는 제외한다. 입력 문자열과 compiler 초기 전역 설정을 만든 뒤 계측을 켠다.

기대: 4000개 token의 모든 kind/value/line/col 순서가 그대로이며 lex 반환 직후 live payload ≤134217728 byte, peak ≤167772160 byte. 최소 단정은 코드 변경 전에 고정한다. 일시적 decode buffer의 누적 할당량을 누수나 live 값으로 보고하지 않는다.

driver는 실제 self/cgc-main.fl의 마지막 CLI 호출 한 줄만 제거한 시험용 library를 만든다. lex/emit 본문이나 token 결과를 C helper로 대체하지 않는다. 개선한다면 `.fl`의 균형 잡힌 chunk 누적으로 append 복사를 O(N log N)으로 줄이고 최종 flat token API·문법·오류·생성 C 기대값은 유지한다. 원래 v11 immutable array 동작이나 worker 수를 바꾸지 않는다. 최소 시험 뒤 실제 큰 자기 입력의 unchanged native bootstrap과 전체 회귀가 필요하다.

단계별 실패도 보존한다. chunk만 적용하면 공통 concat의 prefix 복사 때문에 live 1585809073 byte였다. 공통 concat을 단일 결과 할당으로 고친 뒤 concat 최소 시험은 live 65686 byte로 통과했지만 lexer는 1418591921 byte로 여전히 실패했다. 새 token-chunk-add의 이름 있는 tail recursion이 runtime run_tail의 heap_copy로 남은 forest 전체를 넘기기 때문이었다. compiler 내부의 carry를 기존 loop/recur로 표현해 불필요한 함수 간 전체 forest 전달을 피한다. generic tail-call의 의미나 snapshot 소유권을 바꾸지 않는다. generic tail-call의 큰 공유 인자 복사 비용을 모두 해결했다고 보고하지 않는다. 원래 메모리·token 기대값은 유지한다.
