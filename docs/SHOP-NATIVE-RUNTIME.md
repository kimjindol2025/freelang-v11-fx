# Shop Native API 계약

기준 FX c99b6f78의 격리 변경. source→C 고정점과 ELF 해시는 쇼핑몰 Native 검증 보고 및 evidence/참조.

| API | 계약 |
|---|---|
| random_bytes(n) |정수0..1048576 byte의 CSPRNG, lowercase hex 길이2n, RNG 실패 throw |
| hash_eq(a,b) |각64hex SHA256digest만 허용,32byte CRYPTO_memcmp, 잘못된 digest=false |
| hmac_sha256(key,message) |UTF-8 key/message, SHA256 lowercase hex, 오류 throw |
| now_iso() |UTC YYYY-MM-DDTHH:mm:ss.SSSZ, 시스템 wall clock, 실패 throw |
| shell-exec-result(command) |문자열1개, UTF-8/NUL거부/command1MiB, stdout/stderr/exit/ok map; 합계8MiB; 실제 wait 결과 |
| server_req_client_ip(req) |실제 요청 처리 스레드의 getpeername 주소, 전달 헤더/요청 데이터 미신뢰 |
| dir-private(path) |nofollow directory, 현재UID 소유만,0700 생성/제한 |
| fd-publish(handle,target) |fd-create 전용 fresh0600파일; 소유/private 동일 부모; inode 대조;fsync→renameat→부모fsync |
| dir-fsync(path) |nofollow directory 전용 동기화; 기존 fd-open 디렉터리 차단 유지 |
| sha256(text) |기존 실제 SHA256 구현에 UTF-8 Native dispatcher 연결 |

fd-write 부분쓰기/EINTR는 기존 반복 구현을 유지한다. 게시 실패는 throw하며 rename 후 실패면
쓰기가 없었다고 판단하지 않는다. 동일 UID 악의적 조작과 조상경로 이동은 신뢰 경계 밖이다.
실행시간 제한 없는 shell API는 임의 사용자 명령 실행을 위한 안전한 sandbox가 아니다.
새 코드는 runtime/v11-shop-crypto.h와 process/http/FD/dispatch/catalog이며 crypto-stub를 링크하지 않는다.
해당 primitive 정상 검증은 실제 디스크/전원 장애 내구성을 의미하지 않는다.
