# L08 2차 고정 정답 근거

기준 v11 SHA `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4`.

- 일반 인터프리터 `Interpreter.evalSExpr` → `evalBuiltin` 경로를 기준으로 한다. opt-in VM·fl-exec-op·fast helper의 다른 동작과 섞지 않는다.
- [기본 연산 원문](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/src/eval-builtins.ts): 이름의 `_`→`-` 정규화 후 builtin이 먼저 선택된다. `=`는 flDeepEq, `!=`는 참조 비교다. str_to_num은 parseFloat이며 빈/비숫자는 nil, 숫자 접두사를 허용한다. html-escape의 apostrophe는 `&#x27;`다. str_split의 추가 인자는 결과 조각 수 기준이다. str_replace는 split/join 전역 리터럴 치환이므로 빈 패턴은 문자 사이만 치환한다.
- [특수 형식](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/src/eval-special-forms.ts): if/and는 JS truthiness, or는 nil/false만 거짓이며 단락 평가한다.
- [출력](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/src/interpreter.ts): toDisplayString, println 인자 사이 공백, str의 배열/일반 객체는 JSON 표현.
- [문자열·배열 fallback 원문](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/bootstrap.js#L27165): builtin에 없는 str_slice/find/lines/URI·arr_*는 등록 모듈 계약을 따른다. 문자열 인덱스는 UTF-16 단위이며 이모지는 두 단위다.
- 잘못된 UTF-8은 텍스트 API 진입 시 U+FFFD로 해석한다. L07 raw 데이터의 바이트와 내부 UTF-16 단독 surrogate를 구분하며, raw 파일 복사는 원본과 cmp한다. URI의 잘못된 UTF-8·percent는 예외다. 파일 API 자체의 text alias(C10)는 구현하지 않는다.

Node·TS·JS·Python을 실행해 정답을 생성하지 않았다. 정답은 대응 수정 전에 커밋하며 수정해서 통과시키지 않는다. C ABI의 기존 바이트/산술 함수는 seed와 C 호출자용으로 유지하고 FreeLang 컴파일 경로를 연결한다.
