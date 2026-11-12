# B-8: 브라우저 영속 초안의 언어 API 구현 제안

이 문서는 후속 언어 작업의 제안과 고정 시험 준비다. 컴파일러 수정 승인이나 제품 보관 정책의 선택을 의미하지 않는다. 현재 블로그는 언어에서 지원하는 타이머·HTTP·이벤트를 사용하지만 고정 compiler의 공개 browser builtin 표와 browser core에 저장소 API는 없다.

## 제안 계약

| .fl 함수 | 정상 값 | 인자 오류 |
|---|---|---|
| browser-storage-read key | localStorage 문자열, 키가 없으면 nil | 인자 1개, 비어 있지 않은 문자열 키 |
| browser-storage-write! key value | 저장 뒤 nil | 인자 2개, 비어 있지 않은 문자열 키·문자열 값 |
| browser-storage-remove! key | 삭제 뒤 nil, 재삭제도 nil | 인자 1개, 비어 있지 않은 문자열 키 |

빈 문자열 값은 허용하고 없는 키와 구분한다. 브라우저 표준 저장소를 얇게 연결하며 별도 JSON 파서를 만들지 않는다. Storage SecurityError는 `browser-storage: access denied`, QuotaExceededError는 `browser-storage: quota exceeded`, 그 밖의 저장소 오류는 `browser-storage: operation failed`로 드러내고 키/본문/원래 예외 메시지는 노출하지 않는다. 접근 거부를 nil로, 실패한 쓰기를 성공으로 반환하지 않는다. quota 실패 때 기존 값이 유지되는지도 시험한다.

자동 만료·임의 저장 한도·일괄 clear API는 넣지 않는다. 제품의 account/blog/draft/tab namespace, 복구 선택, 보관/정리 정책은 별도다. 이 primitive 구현만으로 E05의 새로고침/브라우저 재시작 복구가 완료되지는 않는다.

## 준비한 20개 정답

`tests/b8-storage-proposal.fl`과 `.expected`에 없음/쓰기 반환/재조회/빈 값/Unicode/덮어쓰기/삭제 반환/삭제 후 조회/재삭제/다른 키 보존 10개, 잘못된 키·값·인자 수 8개, 접근 거부·quota 실패와 기존 값 보존 2개를 고정했다. 아직 runtime 통과 수가 아니다.

대응 HTML은 새 합성 Chrome 프로필에서만 사용한다. 정상 읽기/쓰기는 실제 localStorage를 사용하고, 두 전용 시험 키에서만 표준 DOMException을 발생시킨다. 실제 메모리나 디스크를 가득 채우지 않는다. 제품 빌드에는 넣지 않는다.

준비 커밋은 `4d9a08fdbc18b32fd5d83e0bea90df4cc3d42005`다. 수정하지 않은 고정 compiler로 .fl을 컴파일한 실제 결과는 아래와 같다. 생성 출력이 없어 Chrome의 20개 runtime 사례는 모두 미실행이다. 실패 재현을 통과 수에 포함하지 않는다.

```text
STORAGE_PROPOSAL_COMPILE_EXIT=1
Uncaught error (line 440): browser target: unsupported operation: browser-storage-read
STORAGE_GENERATED_OUTPUT_PRESENT=NO
STORAGE_RUNTIME_CASES_EXECUTED=0
```

이 진단은 compiler가 출력한 원문이다. compiler/core/seed는 수정하지 않았으며 같은 실패를 반복 실행하지 않았다. compiler 실행에는 1 GiB 가상 메모리 상한을 뒀고 최대 RSS는 `482212 kbytes`였다. 원문은 `<가림>`에 보존했다.

## 구현 승인 시의 변경 범위

언어 저장소의 공개 browser builtin 연결, .fl browser core, 필요한 최소 host primitive와 그 생성 내장 데이터의 동기화가 대상이다. 고정 seed·기존 정답·문턱값·시간제한은 유지한다. 먼저 언어 저장소의 실제 HEAD·CLEAN과 동시 작업을 확인한 뒤 정답을 그 저장소에도 선행 커밋한다. 현 블로그의 시험 준비를 그 언어 저장소의 선행 커밋으로 대신하지 않는다.

승인 이후에도 실제 Chrome 20개, 기존 언어 의미/DOM/이벤트/타이머/HTTP/취소, native 회귀와 strace 없는 bootstrap의 stage2=3=4를 별도로 확인해야 한다. 고정점이 바뀌면 기존 값과 새 값을 원문으로 비교하고 이유를 기록한다. 이번 작업에서 compiler·core·seed를 수정하지 않는다.

## 제품 정책과 남은 별도 시험

D04의 보관 기간·개수·계정별 용량·로그아웃/공용 기기 처리는 미정이다. 그 값 없이 임의 자동 삭제나 브라우저 원문 영속 저장을 제품에 켜지 않는다. API 20개 외에 reload·브라우저 재시작·계정/탭 격리·손상 값·저장 접근 거부 UI·quota UI·서버본과 충돌할 때 선택과 양쪽 보존의 제품 시험이 필요하다.

## 언어 작업 시작

사용자가 브라우저 저장소 API 구현을 승인했다. 언어 출발 HEAD는 `02176817d44f4d998a3f0b8ac04cec4373e04a7f`, 작업 트리는 CLEAN이다. 독립 가지 `feat/browser-storage-b8`에서 위 정답 20개를 먼저 커밋한다. 제품 영속 저장 활성화와 D04 수치 선택은 보류한다.
