# C14 네이티브 HTTP 진행 보고

## 확인한 것

기준은 v11 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4`의 `src/stdlib-http-server.ts`다. TS 소스를 읽었으며 Node로 실행하지 않았다. 먼저 `.fl` 요청 처리기·계약표·PNG fixture를 커밋하고 `HTTP_C14_BEFORE_BUILD_EXIT=1`을 보존했다. 본문과 동적 경로가 debug 로그에 노출되는 별도 실제 HTTP 시험도 수정 전에 커밋했으며 `HTTP_LOG_REDACTION=FAIL`, `HTTP_LOG_BEFORE_EXIT=1`을 보존했다.

공개 server helper를 v11 가변 인자·응답 map 계약에 연결했다. nil과 false를 구분한 middleware, raw path parameter, 중복 query/header, cookie, 상태·JSON·HTML·중복 Set-Cookie, 보호된 static 응답을 구현했다. 기존 L06 listener·framing·업로드 parser를 공유하며 새 앱 우회 parser는 만들지 않았다. 바이너리 응답은 문자열의 byte length를 사용한다. HTTP JSON은 own `__proto__` key를 보존하며, 기존 json_str의 별도 직렬화 계약은 유지한다.

```bash
CPATH=<가림> LIBRARY_PATH=<가림> L08_HTTP_PLATFORM_OUT=<가림> strace -f -e trace=execve -o <가림> bash scripts/verify-l08-http-platform.sh
bash scripts/audit-l07-execve.sh <가림>
```

```text
HTTP_LOG_REDACTION=PASS
L08_HTTP_PLATFORM_PASS=35 FAIL=0
HTTP_PLATFORM_FIXED_EXIT=0
NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0
```

[전체 자체 시험 출력](evidence/l08-platform/http-after.txt), [debug 수정 전 실패](evidence/l08-platform/http-log-before.txt), [고정 계약](../tests/l08-platform/http-platform-contract.md), [실제 요청 시험](../scripts/verify-l08-http-platform.sh). 시험은 port 0으로 시작한 소유 서버만 종료하며 운영 데이터·계정·설정에 접근하지 않는다. PNG는 0~255 모든 바이트를 포함한 유효 ancillary chunk를 넣고 JPEG·GIF와 함께 원본/응답을 cmp했다. 외부 sentinel도 보존됐다.

네이티브 구현 커밋은 `b2e52adcd07f1b68364daa90fb678367c97dc506`이다. 위 자체 실행 때의 runtime·`.fl`·자체 driver 소스를 그대로 이 커밋에 담았다. 전체 driver에는 HTTP 단계도 추가했으며 이 driver 변경의 실행 증거는 전체 회귀 결과로 별도 제출한다.

## 적용한 보호와 범위

URI `8192` bytes, 전체 header `32768` bytes, 전체 body `64 MiB`, routes/middleware 각각 `256`의 네이티브 한도를 유지한다. 앱이 더 낮은 body limit를 선언할 수 있다. C10 요구에 따라 static 경로 이탈·symlink를 거부한다. response header의 CR/LF·불법 이름을 전송 전에 거부하며 임의 Content-Length·Transfer-Encoding을 사용하지 않는다.

DEBUG=2에서도 body는 byte count만, request/response path는 등록 route pattern만 기록한다. 동적 path·query·본문 값은 기록하지 않는다. 이는 v11 raw debug 출력보다 강한 PRD 비밀 값 비기록 요구를 적용한 의도된 차이다. 시험 fixture의 공개 marker만 사용했으며 비밀 값은 증거에 출력하지 않았다.

## 미확인

최신 C14 코드의 전체 회귀·L06 HTTP/TLS·L07·native bootstrap은 별도 실행으로 확인해야 한다. 이전 인증 코드의 전체 통과를 이 코드의 전체 통과로 대신하지 않는다. 원래 Node의 임의 regex route·비동기 Promise와 모든 incidental 동작의 일치를 주장하지 않는다. route 등록은 시작 전이라는 현재 앱 사용을 시험했으며 실행 중 동시 등록은 미확인이다.

폼 body accessor의 v11/L06 차이는 C15에서 별도 계약 검증한다. 인증 verify 결정·나머지 언어 항목·전체 blog1 native 실행·실제 로그인·화면·댓글·다중 작성자·운영 데이터 호환·백업/복구는 남아 있다. C14 자체 통과를 블로그 완성으로 집계하지 않는다. 서버 23·25 접속·운영 데이터 변경·합치기·배포는 하지 않았다.
