# B-8: 브라우저 저장소 언어 API 계약

사용자가 브라우저 저장소 API 구현을 승인했다. 아래 20개 정답은 언어 저장소에서 구현보다 먼저 `c20dc641b0fb067e343cba9647cd9c801bbf27b8`로 고정했다. 개발용 native 컴파일러에서 생성한 결과는 실제 Chrome 20/20이지만, 정식 소스 빌드·전체 회귀·bootstrap은 미완료다. 제품 보관 정책의 선택이나 영속 저장 활성화를 의미하지 않는다. 실행 범위와 실패 원문은 `B8-browser-storage-progress.md`에 구분한다.

## 승인된 API 계약

| .fl 함수 | 정상 값 | 인자 오류 |
|---|---|---|
| browser-storage-read key | localStorage 문자열, 키가 없으면 nil | 인자 1개, 비어 있지 않은 문자열 키 |
| browser-storage-write! key value | 저장 뒤 nil | 인자 2개, 비어 있지 않은 문자열 키·문자열 값 |
| browser-storage-remove! key | 삭제 뒤 nil, 재삭제도 nil | 인자 1개, 비어 있지 않은 문자열 키 |

빈 문자열 값은 허용하고 없는 키와 구분한다. 브라우저 표준 저장소를 얇게 연결하며 별도 JSON 파서를 만들지 않는다. Storage SecurityError는 `browser-storage: access denied`, QuotaExceededError는 `browser-storage: quota exceeded`, 그 밖의 저장소 오류는 `browser-storage: operation failed`로 드러내고 키/본문/원래 예외 메시지는 노출하지 않는다. 접근 거부를 nil로, 실패한 쓰기를 성공으로 반환하지 않는다. quota 실패 때 기존 값이 유지되는지도 시험한다.

자동 만료·임의 저장 한도·일괄 clear API는 넣지 않는다. 제품의 account/blog/draft/tab namespace, 복구 선택, 보관/정리 정책은 별도다. 이 primitive 구현만으로 E05의 새로고침/브라우저 재시작 복구가 완료되지는 않는다.

## 준비한 20개 정답

`tests/l01/browser-storage.fl`과 `tests/l01/browser-storage.expected`에 없음/쓰기 반환/재조회/빈 값/Unicode/덮어쓰기/삭제 반환/삭제 후 조회/재삭제/다른 키 보존 10개, 잘못된 키·값·인자 수 8개, 접근 거부·quota 실패와 기존 값 보존 2개를 고정했다. 대응 페이지는 `tests/l01/browser-storage.html`이다. 기존 정답은 변경하지 않았다.

대응 HTML은 새 합성 Chrome 프로필에서만 사용한다. 정상 읽기/쓰기는 실제 localStorage를 사용하고, 두 전용 시험 키에서만 표준 DOMException을 발생시킨다. 실제 메모리나 디스크를 가득 채우지 않는다. 제품 빌드에는 넣지 않는다.

최초 블로그 준비 커밋은 `4d9a08fdbc18b32fd5d83e0bea90df4cc3d42005`다. 당시 수정하지 않은 고정 compiler의 실제 결과는 아래와 같았다. 당시에는 생성 출력이 없어 Chrome의 20개 runtime 사례를 실행하지 못했다. 이 과거 실패를 현재 통과 수에 포함하지 않는다.

```text
STORAGE_PROPOSAL_COMPILE_EXIT=1
Uncaught error (line 440): browser target: unsupported operation: browser-storage-read
STORAGE_GENERATED_OUTPUT_PRESENT=NO
STORAGE_RUNTIME_CASES_EXECUTED=0
```

이 진단은 블로그 준비 단계의 compiler 출력 원문이며 당시 compiler/core/seed는 수정하지 않았다. 당시 실행의 1 GiB 가상 메모리 상한과 최대 RSS `482212 kbytes`도 과거 값이다. 승인 후 언어 구현·재시험과 구분한다. 원문은 `<가림>`에 보존했다.

## 승인된 변경 범위와 남은 관문

공개 browser builtin 연결, .fl browser core의 인자 검사/오류 처리, 최소 host primitive와 생성 내장 데이터 동기화를 구현했다. 고정 seed·기존 정답·문턱값·시간제한은 유지했다. 다른 검수와 분리한 새 작업 트리의 실제 HEAD·CLEAN을 확인한 뒤 언어 저장소에도 정답을 선행 커밋했다.

native 전체 회귀와 strace 없는 정식 bootstrap의 stage2=3=4는 여전히 별도로 확인해야 한다. 고정점이 바뀌면 기존 값과 새 값을 원문으로 비교하고 이유를 기록한다. 이번에는 browser core와 프린터/내장 데이터만 바꿨으며 seed와 native compiler .fl 원본은 바꾸지 않았다. 개발용 컴파일러의 부분 검증을 정식 bootstrap 통과로 대체하지 않는다.

## 제품 정책과 남은 별도 시험

D04의 보관 기간·개수·계정별 용량·로그아웃/공용 기기 처리는 미정이다. 그 값 없이 임의 자동 삭제나 브라우저 원문 영속 저장을 제품에 켜지 않는다. API 20개 외에 reload·브라우저 재시작·계정/탭 격리·손상 값·저장 접근 거부 UI·quota UI·서버본과 충돌할 때 선택과 양쪽 보존의 제품 시험이 필요하다.

## 언어 작업 시작

언어 출발 HEAD는 `02176817d44f4d998a3f0b8ac04cec4373e04a7f`, 작업 트리는 CLEAN이었다. 독립 가지 `feat/browser-storage-b8`에서 위 정답 20개를 먼저 커밋했다. 제품 영속 저장 활성화와 D04 수치 선택은 보류한다.
