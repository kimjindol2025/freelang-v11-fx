# 실제 parser의 메모리 경계

큰 자기 컴파일의 메모리 증가를 lexer와 분리한다. 실제 self/cgc-main.fl에서 마지막 CLI 호출만 제거한 library의 lex·parse를 그대로 호출한다. 기존 compiler-lex-memory C helper는 malloc/calloc/realloc/free 계측만 하므로 그대로 재사용한다. token 400개를 준비한 뒤 계측을 시작하여 parser의 추가 live·peak payload만 센다. 기존 lexer 시험·정답은 변경하지 않는다.

정답은 100행 `(println 1)`의 모든 AST kind/op/args/type/value/line이다. 현재 소스 make-literal·make-sexpr·parse-atom이 그 근거다. 입력과 정답을 줄이거나 구문 분석을 우회하지 않는다. 100개의 작은 식에 추가 live 67108864 byte·peak 83886080 byte를 상한으로 고정한다. 이는 식마다 수백 KiB를 허용하는 여유 있는 최소 회귀 경계이며 전체 자기 컴파일·RSS·일반 누수 검사를 대체하지 않는다. 실제 할당 payload만 포함하고 준비한 token·계측 장부·공유 라이브러리는 제외한다. 구현을 바꾼 뒤 상한을 높여 통과시키지 않는다.
