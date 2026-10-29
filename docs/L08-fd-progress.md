# C11 fd 기본 구현 진행 보고

## 확인한 것

별도 native handle 표와 잠금을 추가하고 fd_open/write/read/fsync/close/seek/flush 및 밑줄·하이픈 별칭을 연결했다. C10의 경로·심볼릭 링크 보호를 사용하며 임의의 시스템 fd를 닫을 수 없다. 요청 arena 수명과 독립된 C handle을 사용한다. write는 부분 쓰기·EINTR를 반복 처리하고 0바이트 진행은 오류로 처리한다. fsync는 실제 syscall이다. Linux close의 EINTR에서는 재시도하지 않고 handle을 폐기한다. 텍스트는 v11 UTF-8 계약이며 L07의 바이너리 API와 공존한다. 개별 읽기·파일 쓰기 범위는 기존 한도인 64 MiB다.

기본 `.fl` writer/reader와 정답을 먼저 커밋하고, 미연결 API로 native 빌드가 실패하는 원문을 보존한 뒤 구현했다. 실제 synthetic WAL 파일의 image-add/image-delete 문자열을 썼다. writer 과정 종료 뒤 reader를 별도 과정으로 10회 실행했다. 이는 **db95 재생 시험이 아니라 fd 내용 보존 시험**이다.

```bash
CPATH=<가림> LIBRARY_PATH=<가림> L08_FD_OUT=<가림> strace -f -e trace=execve,write,fsync,close -o <가림> bash scripts/verify-l08-fd-self.sh
```

```text
C11_BEFORE_BUILD_EXIT=1
L08_FD_BASIC_PASS=11 FAIL=0
FD_BASIC_SELF_EXIT=0
NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0
```

writer의 고정 출력 10개, 각 reader의 고정 출력 13개를 비교했다. w truncate·a append·r read, 실제 fsync·flush, EOF, cursor를 바꾸지 않는 v11 fd_seek, closed/invalid handle·잘못된 mode·read-only write·음수 read 오류를 검사했다. fd_write/fsync/close 성공 값은 byte count가 아니라 true다.

수정 전후 및 실제 fsync syscall 원문은 docs/evidence/l08-platform/fd-before.txt·fd-basic-after.txt·fd-basic-fsync.txt·fd-basic-trace-audit.txt에 보존한다.

## 미확인

부분 쓰기·EINTR·fsync/close 실패의 강제 주입, NUL·읽기 경계·한도·동시 호출·경로 음성 시험의 C11 전용 보완, C11 추가 후 전체 회귀·native bootstrap은 아직 실행하지 않았다. 따라서 C11 전체 완료라고 판정하지 않는다. 쓰기 도중 오류가 나면 일부 prefix가 파일에 남을 수 있으며 오류를 성공으로 바꾸거나 기록을 잘라내지 않는다.

C12 JSON·기존 WAL checksum 검증, db95의 잘린 끝·추가/삭제 반복 재생, blog1 전체 HTTP 실행·백업·복구·안정성은 별도 남은 요구사항이다. 현재 실행 중인 컴파일러 보완 전체 검증은 별도 체크아웃의 C01·C02 소스에 대한 것으로 이 fd 변경의 회귀 증거가 아니다. 운영 데이터 수정·서버 23·25 접속·합치기·배포는 하지 않았다.

## 후속 검사

공개 `.fl`을 통한 write 부분 진행/EINTR·write 0, fsync EINTR/EIO, read EINTR/짧은 read, close EINTR/EIO의 8개 모드는 `L08_FD_FAULT_PASS=8 FAIL=0`, `FD_FAULT_SELF_EXIT=0`으로 통과했다. close 두 모드 모두 `FD_REUSE_GUARD=PASS`이며 실패가 성공으로 표시되지 않았다.

추가 경계 시험 26개와 base symlink 시험은 통과했다. 실제 스레드 8개가 barrier에서 만난 뒤 공유 handle에 800개 레코드를 쓰는 시험의 고정 출력 6개도 일치했다. 최초 확장 드라이버는 C 보조기의 count 이름 충돌과 강제 include가 GNU feature macro보다 먼저 처리된 빌드 문제로 종료 코드 1이었다. 보조기 이름·시험 빌드 플래그만 수정했고 언어 구현·기대 출력은 바꾸지 않았다. 수정한 전체 확장 드라이버는 최신 전체 검증에서 다시 실행하며 그 결과가 확정되기 전에는 확장 드라이버 전체 통과라고 쓰지 않는다. 실제 db95 재생·운영 WAL 호환은 여전히 별도 미확인이다.
