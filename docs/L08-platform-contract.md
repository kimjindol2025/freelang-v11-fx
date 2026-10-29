# L08 잔여 호환 계약과 통합 검증

## C18: 먼저 고정한 정답

기준 v11 커밋 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4`의 `src/stdlib-process.ts`와 `bootstrap.js`의 `createErrorModule`·`uuid_short`를 Git 텍스트로 조사했다. Node를 실행하지 않았다.

- env_get: 누락·빈 값은 nil. env_or: 누락·빈 값은 주어진 기본값, 나머지는 문자열. 일반 함수 인자로 기본값 표현식도 평가한다.
- error_message: 문자열 그대로, truthy map.message는 JS String 변환, 나머지는 JS String 변환. nil은 null, 일반 map은 [object Object], 배열 message는 쉼표 연결이다. native에서 throw 값은 catch에 그대로 전달된다.
- process_exit: 생략 시 0, 명시 7이면 7. 시험 자식만 종료하며 후속 출력은 없어야 한다.
- uuid_short: 암호학적 난수 4바이트의 소문자 hex 8자리. 512개 표본 형식·중복 확인은 난수 품질의 수학적 증명이 아니다.

시험 `.fl`과 기대 출력을 구현 전에 커밋한다. 기존 native C ABI는 보존하고 public v11 별칭을 보완한다. 운영 환경 변수·데이터·키는 시험 입력으로 쓰지 않는다.

## 전체 목표의 남은 검증

C11 fd 내구성, C12 WAL JSON·해시, C13 인증, C14 HTTP, C15 폼, C16 HTTP 클라이언트, C17 feed, C18 환경, C19 타이머/TCP/TLS, C20 동시성의 언어 시험을 각각 요구한다. 이 문서와 C18 통과는 다른 항목이나 blog1 전체 통합의 완료 증거가 아니다.

## 전체 컴파일에서 추가 확인한 C01·C02 차이

C18과 별도로 전체 앱의 실제 실패에서 최소 재현을 추출했다. 기존 자체 시험만으로 전체 load 호환을 판정하지 않는다.

- db95 `engine-order-rows`의 map 리터럴 안 lambda가 외부 column·collation을 캡처하지 못한다. `cgc-collect-vars`가 Map 블록에 Array 전용 `get-block-items`를 적용하는 것이 원인이다. 언어 쪽에서 수정하며 앱 코드를 평탄화하지 않는다.
- v11 `bootstrap.js:2070~2100`은 `(catch [error] ...)`와 `(catch error ...)`를 모두 받는다. `evalTryBlock`은 catch 변수에 message·type·raw를 가진 객체를 넣는다. fx의 binding pass는 vector catch 변수를 읽지 못하며 기존 catch는 raw 값을 그대로 반환한다.
- C18의 `(error_message e)` 시험은 raw 문자열과 v11 오류 객체 모두에서 같은 결과이므로 catch 구조 호환의 증거가 아니다. catch 객체·범위·finally는 별도 시험으로 보완한다. 예전 음성 시험이 raw catch 문자열을 전제한다면 source 계약 근거와 변경 이력을 남겨 입력을 보완하며 정답을 구현에 맞춰 바꾸지 않는다.

`compiler-blockers.fl`과 기대 출력은 위 재현의 수정 전에 고정한다. 이 파일은 C18 자체 시험 집계에 넣지 않는다.
