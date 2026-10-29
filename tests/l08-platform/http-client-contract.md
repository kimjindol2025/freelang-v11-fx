# C16 HTTP 클라이언트 구현 전 계약

기준 v11 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4`, `src/stdlib-http.ts`의 nodeHttpRequest·http_get·http_request·http_request_timeout이다. TS/내장 worker JS는 텍스트로만 읽으며 실행하지 않는다. blog1의 HTTP 통합 시험은 http_request의 method/url/headers/body 순서를 사용한다. 현재 native http_req의 method/url/body/headers와 구분해 공개 연결하며 앱에서 인자를 뒤집지 않는다.

- 성공: status/body만 있는 map. headers나 성공 error:nil을 덧붙이지 않는다. HTTP 404도 정상 응답이며 error는 없다.
- 연결·주소·TLS·timeout 실패: status 0/body 빈 문자열/error 문자열. timeout은 정확히 timeout. OS/TLS 세부 오류 문구는 libcurl과 Node가 달라 byte 동일성으로 판정하지 않는다. 원문을 꾸미거나 주소·인증 header·본문을 로그에 추가하지 않는다.
- method는 기본 GET, 대문자로 정규화한다. POST뿐 아니라 PUT/PATCH/DELETE도 body를 보내며 UTF-8 byte length로 Content-Length를 정한다. NUL 뒤를 자르지 않는다.
- response body는 v11 Buffer.toString UTF-8 replacement 계약을 따른다. 잘못된 FF는 U+FFFD, NUL은 그대로 남는다.
- Node 기본 HTTP처럼 redirect를 자동 추적하지 않는다. 환경 HTTP proxy를 자동 사용하지 않는다. http/https만 허용하며 file 등의 로컬 자원은 읽지 않는다.
- header 이름·값의 CR/LF를 보내기 전에 거부한다. request timeout 기본은 10000 ms, request_timeout의 Number(value)||5000 규칙과 양의 시험값 50 ms를 확인한다. native timeout은 전체 요청의 상한으로 적용한다. source의 inactivity timeout과 같은 진행 중 무한 스트림 동작을 주장하지 않는다.
- TLS는 peer chain과 hostname을 모두 검증한다. 시험용 자체 서명 CA는 별도 생성하며 표준 SSL_CERT_FILE로 지정한 시험 과정에서만 신뢰한다. 기본 신뢰에서 실패, 지정한 CA에서 성공, hostname 불일치에서 실패해야 한다. cert/private key 내용은 출력·Git에 넣지 않는다.

실제 loopback에서 FreeLang으로 컴파일한 서버/클라이언트를 실행한다. port 0과 소유 과정만 사용한다. 정답을 먼저 커밋하고 미연결 빌드 실패를 보존한 뒤 구현한다. TLS 도구가 없으면 미실행으로 명시한다. 함수 직접 호출만으로 완료 판정하지 않는다. 기존 legacy http_req API는 보존하며 새 v11 API에 앱 우회를 넣지 않는다.

구현 전 fixture 정정: 최초 POST fixture는 X-Value에 한글을 넣었으나 source Node HTTP header 값은 Latin-1 범위만 허용한다. 성공 요청의 header를 ASCII sample로 고쳤으며 한글·이모지·NUL body 정답은 유지했다. native 구현을 시작하거나 시험 출력에 맞추기 전에 기준 header 계약으로 입력 오류를 정정한 것이다.
