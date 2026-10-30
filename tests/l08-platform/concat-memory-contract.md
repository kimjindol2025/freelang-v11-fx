# immutable vector concat의 단일 결과 할당

lexer chunk 개선만 적용한 실행은 4000개 정확한 token에도 live 1585809073 byte로 실패했다. 공통 C append(v1,v2)가 빈 배열에 두 입력의 항목을 하나씩 fl_vec_push하여 concat 한 번조차 O(N²)의 prefix 할당을 만드는 것을 확인했다. `.fl` chunk 개선이나 C helper가 이미 통과했다고 보고하지 않는다.

표준 라이브러리 수정 전 추가 정답: 2048개씩인 두 vector를 concat하여 정확한 순서의 4096개 원소를 반환하고 입력을 유지한다. 입력 준비 뒤 계측한 live·peak native payload 각각 ≤8388608 byte. 시간 timeout만으로 메모리 개선을 주장하지 않는다. 원래 v11 immutable concat·append 두 vector의 얕은 원소 복사 동작과 non-vector C 인자 동작을 유지한다. 일반 `.fl append vector scalar`의 계약은 바꾸지 않는다.

공통 C 함수는 전체 결과 길이를 검사하고 최종 vector·buffer만 할당해 입력 원소를 각각 memcpy해야 한다. arena와 main의 allocation failure 정리도 보존해야 한다. C만 직접 호출한 시험으로 언어 API 완료를 대신하지 않고 실제 `.fl concat`·lexer·기존 C08·native bootstrap을 확인한다.
