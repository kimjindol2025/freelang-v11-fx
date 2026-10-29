# C16 네이티브 HTTP 클라이언트 진행 보고

## 확인한 것

v11 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4`의 `src/stdlib-http.ts` 소스를 읽고 `.fl`·정답·실제 HTTP/TLS driver를 먼저 커밋했다. Node와 source worker JS는 실행하지 않았다. `HTTP_CLIENT_BEFORE_BUILD_EXIT=1`을 보존한 뒤 기존 libcurl 기반 코드에 v11 공개 API를 연결했다. 새 외부 라이브러리를 추가하지 않았다.

대상은 http_get, http_request, http_request_timeout, http_with_timeout이다. method/url/headers/body 순서, 성공 status/body map, HTTP 오류 응답과 연결 실패의 구분, redirect 미추적, 실제 body byte length와 UTF-8 replacement, NUL, timeout, header 검증을 구현했다. 기존 http_req 인자 순서는 그대로다. header를 고정 1024 byte buffer로 자르지 않으며 buffer 증가 실패 시 이전 포인터를 잃지 않는다. 요청 header 생성이 실패하면 header 없이 요청을 보내지 않는다.

수정 전 이름 없는 API 빌드 실패와 별도로 실제 NUL header 이름이 거부되지 않는 실패도 먼저 보존했다. token 검사에서 C strchr의 NUL 검색이 참이 되는 원인을 고쳤다. 빈 header 값은 libcurl의 `Name;` 방식으로 보내며 숫자·boolean은 source String 변환을 따른다. libcurl global 초기화는 pthread_once로 수행한다.

구현 커밋은 `8800b53f0299d58c53b4bca1f2697096ec9ccf85`이다. 아래 자체 실행의 runtime·시험 소스를 이 커밋에 담았으며 전체 driver에 client 단계를 추가했다.

```bash
CPATH=<가림> LIBRARY_PATH=<가림> L08_HTTP_CLIENT_OUT=<가림> strace -f -e trace=execve -o <가림> bash scripts/verify-l08-http-client-self.sh
bash scripts/audit-l07-execve.sh <가림>
```

```text
HTTP_CLIENT_PROGRAM_EXIT=0
RAW_SERVER_WIRE_EXACT=PASS
HTTP_CLIENT_RAW_PROGRAM_EXIT=0
HTTP_CLIENT_BOUNDARIES_PROGRAM_EXIT=0
HTTP_CLIENT_TLS_UNTRUSTED_PROGRAM_EXIT=0
HTTP_CLIENT_TLS_TRUSTED_PROGRAM_EXIT=0
L08_HTTP_CLIENT_PASS=6 FAIL=0
HTTP_CLIENT_SELF_EXIT=0
NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0
```

`6`은 driver의 시험 묶음 수다. 각 묶음에는 GET·POST·PUT·PATCH·DELETE, HTTP 404, 302, timeout, 주소·연결 실패, NUL/한글/이모지, 잘못된 UTF-8, header 경계와 TLS 검사가 들어 있다. TLS는 기본 신뢰에서 자체 서명을 거부하고, 소유 시험 과정의 SSL_CERT_FILE로 CA를 지정하면 성공하며, 같은 CA라도 hostname 불일치는 거부한다. port 0으로 시작한 시험 과정만 종료했다. cert/private key 내용은 Git·stdout·보고에 넣지 않았다.

[수정 전 빌드](evidence/l08-platform/http-client-before.txt), [NUL 경계 수정 전](evidence/l08-platform/http-client-boundary-before.txt), [수정 후 원문](evidence/l08-platform/http-client-after.txt), [계약·입력 정정 이력](../tests/l08-platform/http-client-contract.md).

최초 입력 오류도 보존했다. FreeLang lexer는 JSON과 달리 직접 `\u0000`을 해석하지 않으므로 JSON parser로 실제 NUL을 만들었다. 또한 C10 text file_read가 FF를 먼저 치환하고 있었으므로 L07 binary file-read로 fixture를 읽어 원래 FF가 wire에 나가는지 cmp했다. 출력 정답을 구현에 맞추지 않았다. [최초 실패](evidence/l08-platform/http-client-initial-after.txt), [최초 probe](evidence/l08-platform/http-client-initial-probe.txt)는 이 정정 이전의 출력이며 실제 NUL/invalid byte의 통과로 집계하지 않는다.

## 미확인 / 적용 범위

최신 코드 전체 회귀·bootstrap과 실제 blog1 HTTP 통합 시험은 아직 미확인이다. app 전체 실행을 뜻하지 않는다. 새 공개 연결 외의 JSON convenience API 전체를 구현했다고 주장하지 않는다. 오류의 OS/TLS 세부 문구는 libcurl과 Node가 다르며 timeout만 고정 문자열이다. request timeout은 전체 요청 상한이며 Node inactivity timeout의 진행 중 스트림 동작과 다르다. native response body 상한은 `64 MiB`, 요청 header는 최대 `64`개·각 이름/값 `32768` byte다. 요청 header 상한을 곧바로 같은 크기의 서버 수용 증거로 쓰지 않는다. 현재 native HTTP 서버의 각 header key/value 상한은 `511` byte로, 별도 전체 플랫폼 검증에서 확인·보완해야 한다.

v11 경로는 HTTP proxy 환경을 자동 사용하지 않고 http/https만 허용하며 peer/hostname을 검증한다. 기존 fx 전용 legacy http_req 계열은 별도 기존 정책 경로이며, 이번 v11 TLS 통과를 그 모든 인터페이스의 보안 검증으로 확대하지 않는다. 출시 전 해당 정책 경로 점검도 남아 있다.

인증 verify 결정·타이머/TCP·폼 경계·동시성·전체 앱 컴파일/실행, 댓글·다중 작성자·실제 화면·운영 데이터 호환·복구는 여전히 남아 있다. 서버 23·25 접속·운영 데이터 변경·합치기·배포는 하지 않았다.

## 작업 가지와 전체 시험 대상

분리 checkout 구현 `8800b53f0299d58c53b4bca1f2697096ec9ccf85`를 작업 가지 구현 `c4f8432590816d2c9e3024a53268e460397554ea`로 반영했다. 최신 전체 시험은 고정 checkout `569d44cb57c1d926de16c8a11fcf81d3b1652592`에서 실행 중이다. 반영 직후 runtime/self/scripts/tests/docs diff가 비어 있음을 확인했다(`CLIENT_TESTED_SOURCE_EQ_BRANCH=PASS`). 이 절 이후 보고 문서만 추가했다. 전체 시험이 끝나기 전에는 통과로 집계하지 않는다.
