# FreeLang compiler lexer의 누적 배열 복사

고정 `2754d8f7d83ce520395ab1c2bfc99b6d72336379`의 bootstrap은 가장 작은 stage2 입력을 통과했지만 큰 자기 입력은 실제 CPU를 사용하며 12분에 RSS 42243432 KiB까지 증가했다. 이 수치만으로 무한 반복이나 완료를 단정하지 않는다.

self/cgc-main.fl의 emit은 매 token마다 기존 flat vector에 append한다. v11 호환 immutable append는 이전 vector를 보존해야 하므로 길이 N까지 prefix 총합 O(N²)을 할당한다. process main에서 이 중간 buffer들이 즉시 해제되지 않는다. 이를 증명하기 위해 실제 lex 함수에 같은 `(println 1)` 1000행(12000 byte)을 넣어 native malloc/calloc/realloc/free의 미해제 payload와 peak를 계측한다. meter bookkeeping·공유 라이브러리 내부 할당·RSS 전체는 제외한다. 입력 문자열과 compiler 초기 전역 설정을 만든 뒤 계측을 켠다.

기대: 4000개 token의 모든 kind/value/line/col 순서가 그대로이며 lex 반환 직후 live payload ≤134217728 byte, peak ≤167772160 byte. 최소 단정은 코드 변경 전에 고정한다. 일시적 decode buffer의 누적 할당량을 누수나 live 값으로 보고하지 않는다.

driver는 실제 self/cgc-main.fl의 마지막 CLI 호출 한 줄만 제거한 시험용 library를 만든다. lex/emit 본문이나 token 결과를 C helper로 대체하지 않는다. 개선한다면 `.fl`의 균형 잡힌 chunk 누적으로 append 복사를 O(N log N)으로 줄이고 최종 flat token API·문법·오류·생성 C 기대값은 유지한다. 원래 v11 immutable array 동작이나 worker 수를 바꾸지 않는다. 최소 시험 뒤 실제 큰 자기 입력의 unchanged native bootstrap과 전체 회귀가 필요하다.
