# L08 2차 고정 정답 근거

기준 v11 SHA `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4`.

- 일반 인터프리터 `Interpreter.evalSExpr` → `evalBuiltin` 경로를 기준으로 한다. opt-in VM·fl-exec-op·fast helper의 다른 동작과 섞지 않는다.
- [기본 연산 원문](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/src/eval-builtins.ts): 이름의 `_`→`-` 정규화 후 builtin이 먼저 선택된다. `=`는 flDeepEq, `!=`는 참조 비교다. str_to_num은 parseFloat이며 빈/비숫자는 nil, 숫자 접두사를 허용한다. html-escape의 apostrophe는 `&#x27;`다. str_split의 추가 인자는 결과 조각 수 기준이다. str_replace는 split/join 전역 리터럴 치환이므로 빈 패턴은 문자 사이만 치환한다.
- [특수 형식](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/src/eval-special-forms.ts): if/and는 JS truthiness, or는 nil/false만 거짓이며 단락 평가한다.
- [출력](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/src/interpreter.ts): toDisplayString, println 인자 사이 공백, str의 배열/일반 객체는 JSON 표현.
- [문자열·배열 fallback 원문](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/bootstrap.js#L27165): builtin에 없는 str_slice/find/lines/URI·arr_*는 등록 모듈 계약을 따른다. 문자열 인덱스는 UTF-16 단위이며 이모지는 두 단위다.
- 잘못된 UTF-8은 텍스트 API 진입 시 U+FFFD로 해석한다. L07 raw 데이터의 바이트와 내부 UTF-16 단독 surrogate를 구분하며, raw 파일 복사는 원본과 cmp한다. URI의 잘못된 UTF-8·percent는 예외다. 파일 API 자체의 text alias(C10)는 구현하지 않는다.

Node·TS·JS·Python을 실행해 정답을 생성하지 않았다. 정답은 대응 수정 전에 커밋하며 수정해서 통과시키지 않는다. C ABI의 기존 바이트/산술 함수는 seed와 C 호출자용으로 유지하고 FreeLang 컴파일 경로를 연결한다.

추가 계약 시험 `contracts`는 같은 일반 평가기의 +·-·*·/ 숫자 타입 오류(원문 메시지), % 숫자 강제 변환, char-at의 Number 인덱스, num-to-str nil의 `"null"`, filter nil 함수의 원본 반환, drop 음수, 유한 range의 시작·끝·증분, take 인자 순서 오류를 고정한다. 무한 lazy sequence는 이번 구현 범위에 넣지 않는다. 이 정답도 대응 수정 전에 별도 커밋했다.

정답 정정: 최초 `contracts.expected`의 num-to-str nil은 빈 문자열로 잘못 고정했다. 일반 평가기 앞에 있는 `flExecOpNative` 보조 함수의 String(v0 ?? "")와 혼동했다. 실제 evalBuiltin의 num-to-str는 String(args[0])이므로 `"null"`이다. 이 한 줄을 원문 근거로 공개 정정하고, 이후 대응 구현을 수정한다. 정답이 처음부터 전부 불변이었다고 보고하지 않는다. 같은 원문의 char-at는 먼저 나오는 case에서 Number 인덱스를 지원한다. 뒤의 중복 case와 혼동하지 않는다.

`source-edge`는 불리언 타입명, +의 초기 0 누적, replaceAll의 빈 패턴·치환 기호, 기본 str-replace의 nil 기본값, split의 숫자 문자열 상한, str-index-of의 nil 기본값, flDeepEq의 같은 키 수/nil 조회 의미, append의 문자열 spread를 추가 고정한다.
