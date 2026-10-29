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
