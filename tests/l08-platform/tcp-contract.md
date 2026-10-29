# C19 TCP·TLS 구현 전 고정 계약

기준 v11 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4` bootstrap.js:16436~16640의 raw TCP/stop/write/drop와 error drain을 텍스트로 조사했다. Node/내장 worker JS는 실행하지 않는다. 직접 raw 분기는 connect/data/close 3인자 callback, conn_N ID, binary 문자열, write ok/error/not-found, drop ok/not-found, stop ok/not-running을 제공한다. 별도 worker 내부 accept/listening 프로토콜을 이 직접 분기의 정답으로 섞지 않는다.

## API와 네이티브 확장

- tcp-server-raw(port, handler[, host]): 시작 ok, 같은 논리 port가 이미 살아 있으면 already-running. 기본 port 30390·host 0.0.0.0은 원래 분기를 따른다. 시험은 항상 port 0·127.0.0.1이다. host는 db95의 세 번째 인자를 실제 bind에 적용하는 확장이다. 잘못된 port는 예외, bind 등 OS 실패는 동기 error와 io-error queue로 보고한다. 원래 Node의 비동기 listen 실패와 구분한다.
- tcp-server-tls(port, handler, cert, key[, host, ca, require-client]): 이 기준의 TLS 구현은 확인되지 않았다. db95 `88b4aa2ca11e5355240a1d93ece8321315c669cc`의 호출 형식에 맞추는 네이티브 확장이다. 기존 OpenSSL을 사용한다. CA/require-client로 클라이언트 인증을 강제하며 config 실패는 error·source tls다. private key·certificate 값과 경로를 로그에 출력하지 않는다. 파일은 보호된 C10 read로 읽고 PEM chain/key를 메모리에서 해석한다.
- callback은 event/id/data를 유지하고 선택적 네 번째 peer 주소를 제공한다. 3인자 handler는 그대로 사용하며 db95의 네 번째 peer 인자가 실제 원격 주소를 받을 수 있다. ID는 프로세스 안에서 재사용하지 않아 여러 서버의 기존 conn_N 충돌을 막는다.
- 데이터는 Node binary/Latin-1 계약이다. 수신 byte 0~255를 같은 UTF-16 unit으로, 송신 문자열의 각 UTF-16 unit 하위 8비트를 byte로 변환한다. NUL과 PNG/JPEG/GIF를 손실 없이 왕복한다. raw chunk 경계·개수는 고정하지 않는다.
- fl-yield는 실제 소켓을 poll하고 callback을 실행한 이벤트 수를 반환한다. Native CLI 정상 반환 뒤에도 서버가 살아 있으면 event loop를 진행한다. HTTP/timer와 dispatcher를 공유해 콜백을 직렬화하며, callback은 별도 arena scope에서 실행한다. 요청 worker를 1개로 줄이지 않는다.
- nonblocking write는 부분 쓰기·EINTR/EAGAIN을 처리하고 순서를 보존한다. 전부 버퍼에 수용해야 ok이며 상한에서 error다. pending 출력은 전체 64 MiB, 서버 16개·연결 512개·close/error queue는 bounded profile이다. half-close 뒤에도 pending 출력을 비운 후 close한다.
- drop/stop은 close를 한 번만 전달한다. callback 안에서 stop/restart해도 이전 descriptor 이벤트가 새 연결에 적용되지 않는다. handler 오류는 [[io-err, recoverable, handler, message]] 형식에 보관하며 프로그램을 중단하지 않는다. fl-error-drain은 읽고 비운다. TLS handshake 실패는 사용자 handler connect가 아니다.

## 먼저 고정한 시험

1. tcp-control.expected: 실제 bind(0), duplicate/stop/restart, missing ID, invalid port, 잘못된 TLS 설정과 오류 drain. C 함수 직접 호출만으로 완료 판정하지 않는다.
2. tcp-echo.fl과 독립 C client: E LF 뒤 임의 binary echo, D LF drop, X LF의 고정 handler 오류, S LF stop. 명령 LF도 분할 송신한다. 0~255·NUL·2 MiB·PNG/JPEG/GIF를 cmp하고, 느린 reader/half-close와 동시 client를 검사한다.
3. 새 process에서 raw 재시작, 두 listener bind 충돌 시 기존 서버가 살아 있는지 확인. 남의 과정을 끄지 않는다.
4. TLS 실제 loopback: 지정 CA 정상, 신뢰 없는 CA/hostname 오류 거부, require-client에서 무인증·다른 CA client 거부·올바른 client 성공, handshake 중단 뒤 정상 재연결. 키는 시험용 임시 영역에만 생성한다.
5. event trace는 연결별 connect 먼저·close 정확히 1회·close 뒤 data 없음. handler 오류 후 다른 정상 연결도 동작한다. callback/peer는 실제 `.fl`에서 확인한다.
6. 수정 전 API 실패·수정 후 원문·종료 코드·strace, timer/L06/L07/이전 L08 전체 회귀와 native bootstrap. TLS 의존성이 없으면 미실행이다. 전체 블로그 compile/run·C20·PRD 완료와 구분한다.

구현은 .fl/C/Bash이며 새 TS/JS/Python·fl-build.sh는 금지한다. 운영 데이터·설정·계정·서버 23/25에 접근하지 않는다. 합치기·배포·강제 푸시는 하지 않는다.
