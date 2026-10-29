# C19 native TCP·TLS 부분 진행

## 확인한 것

v11 소스를 조사해 `.fl` 시험·정답·[계약](../tests/l08-platform/tcp-contract.md)을 먼저 고정하고 미연결 빌드 실패를 보존했다. 이후 C 런타임에 tcp-server-raw·tcp-server-tls·tcp-write·tcp-drop·tcp-server-stop과 오류 queue를 연결했다. TLS는 v11 고정 bootstrap에 없는 native 확장이므로 같은 계약이라고 주장하지 않는다. 구현 커밋은 `ec003915c7114ec3c8148508f81114eaa977ed20`다.

실제 loopback 시험은 `L08_TCP_PASS=8 FAIL=0`, `TCP_SELF_EXIT=0`이었다. RAW·TLS·mTLS, NUL·모든 byte·2 MiB slow reader·PNG/JPEG/GIF, 연결 4개 동시 echo, half-close·drop·handler 오류 뒤 회복·stop, 포트 충돌 시 기존 서버 보존, raw 과정 재시작, 인증서 비신뢰·hostname 불일치·잘못된 client 인증서·중단 handshake, connect/close 순서와 인증서 파일 무변경을 확인했다. RAW send EAGAIN과 SSL_write WANT가 실제로 발생했다. [초기 원문](evidence/l08-platform/tcp-initial-after.txt). `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다.

```bash
CPATH=<가림> LIBRARY_PATH=<가림> L08_TCP_OUT=<가림> strace -f -e trace=execve -o <가림> bash scripts/verify-l08-tcp-self.sh
bash scripts/audit-l07-execve.sh <가림>
```

## 미확인

초기 client의 drop/stop 확인은 음수 read를 close로 인정하여 timeout도 통과할 수 있었다. timeout·TLS WANT를 실패 처리하도록 시험을 강화했으며 최신 전체 driver에서 재실행한다. 초기 8묶음 결과를 강화한 시험의 결과라고 보고하지 않는다. 이후 snapshot 수명 수정도 별도다.

TCP의 sanitizer 검사, callback 중 server 재생성, 모든 TLS 설정 오류·파일 descriptor 정리 경계와 자원 고갈은 추가 검증이 필요하다. 전체 회귀·native bootstrap도 최신 코드에서 아직 미확인이다. 서버 23·25 접속·운영 데이터 수정·합치기·배포는 하지 않았다. 전체 앱의 auth_verify_password 결정 대기와 실제 DB 제약조건 시험 실패, PRD 기능·화면·운영 복구는 남아 있다.
