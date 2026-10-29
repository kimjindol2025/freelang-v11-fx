# C14 실제 HTTP 시험 정답

기준 v11 b49cc9194a00649b8cf58e1d6e8ee38ffce770d4의 stdlib-http-server.ts와 Node request.headers 계약을 조사했다. 시험은 loopback·bind(0)·소유한 서버 과정에서만 수행한다. C 함수 직접 호출을 통과 증거로 사용하지 않는다.

| 경우 | 먼저 고정한 기대 |
|---|---|
| /order | 200, first,second,handler 순서 |
| /false | 200, false. false는 nil이 아니므로 middleware가 단락 |
| /guard/ok | header 없으면 401 DENIED, X-Allow yes면 200 GUARDED |
| /guard/missing | header 없으면 route가 없어도 middleware가 먼저 401 |
| /request/a%2Fb | id a%2Fb, path /request/a%2Fb 유지. pathname을 미리 decode하지 않음 |
| q=한글+tag&q=second&flag | keyed query는 첫 값, 전체 query q는 두 값 배열, flag는 빈 문자열. percent는 한 번, plus는 공백 |
| X-Repeat 중복·Host | 이름은 case-insensitive, X-Repeat 값은 first, second, Host 값은 그대로 |
| Cookie token=a=b, encoded=a%20b | token은 a=b, encoded는 a%20b. 이 helper는 URI decode를 하지 않음 |
| 6000-byte query/parameter | 8192 URI 상한 안에서는 잘리지 않으며 응답 길이 6000 |
| /html·/created | 201. HTML charset UTF-8, JSON은 {"n":0.1} |
| /conflict | 409 JSON {"error":"conflict"} |
| /echo | Content-Type과 무관한 req_json. malformed는 nil, JSON null/false도 구분. JSON 데이터의 __proto__ own key·NUL 보존 |
| /header 뒤 /base | 추가 header는 원래 response를 바꾸지 않음 |
| /cookies | 302 Location /after, Set-Cookie 2행. one=a%20b; Max-Age=0; Path=/; HttpOnly; SameSite=Lax 및 two=x%3Dy; Path=/; HttpOnly; Secure; SameSite=Strict |
| /bad-header | 500, 응답 splitting·Injected 헤더 없음 |
| static PNG/JPEG/GIF | gate 거부 403, 허용 200·MIME·Content-Length·전체 바이트 cmp. PNG private ancillary chunk에 0~255 포함 |
| static ..·symlink | C10 요구의 더 강한 경로 보호를 적용해 403. 외부 sentinel 보존 |
| body 513 bytes | 선언된 512 byte 상한에서 413 |
| wildcard 미등록 URL | 등록된 마지막 /* fallback 404 |

중복 폼·multipart의 제한과 accessor 차이는 C15에서 별도 검증한다. 기존 L06의 native server_req_body map 계약과 v11 helper의 string 계약 차이를 숨기지 않으며, 이 HTTP 시험에서 임시 앱 parser나 accessor 우회를 추가하지 않는다. static 경로 보호는 v11의 단순 startsWith보다 강한 C10 요구를 유지한다. 사람 작성 JS·Node·Python은 사용하지 않는다.
