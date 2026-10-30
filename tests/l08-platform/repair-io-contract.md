# WAL 경계 복구에 필요한 native 언어 I/O 확장

새 source API를 추가하는 언어 작업이다. v11에 이미 있다고 보고하지 않는다. 기존 텍스트 fd-write/read는 v11 UTF-8 변환 계약을 유지하므로 invalid byte를 가진 forensic 원본 보존에 쓰지 않는다. 기존 L07 read-result·write-result와 함께 사용한다.

- bytes-count / bytes-slice: raw FLString byte 길이·반열린 범위 복사. NUL·0xff·잘린 UTF-8을 변환하지 않는다. 정수 위치(정수인 float 포함), 0≤start≤end≤byte 길이만 허용한다. 기존 length·문자 인덱스의 UTF-16 계약은 유지한다. 인자 평가 순서는 왼쪽부터 한 번씩이다.
- fd-create: 기존 secure parent walk로 같은 base 안에 새 regular file만 O_EXCL/O_NOFOLLOW, mode 0600으로 연다. 기존 파일·symlink·외부 경로를 덮지 않는다. file fd와 실제로 생성한 parent directory fd를 함께 소유한다. 기존 fd-open의 descriptor 개수·계약은 유지한다.
- fd-write-bytes / fd-read-bytes: 기존 fd 경계·크기 상한·부분 I/O 처리와 mutex를 재사용하되 UTF-8 변환하지 않는다. 텍스트 API의 대체나 동작 변경이 아니다.
- fd-sync-parent: fd-create가 소유한 원래 parent fd를 fsync한다. 경로를 다시 해석하지 않는다. 기존 fd-open handle에는 이 기능을 허용하지 않는다.
- fd-truncate: write 가능한 handle에서 0~67108864 byte의 정수 길이만 허용한다. 줄일 때 prefix를 유지하고 늘릴 때 kernel의 zero extension을 확인한다. read-only·closed·음수·소수·상한 초과는 오류다. durability는 명시적 fd-fsync로 확인한다.
- fd-close는 file과 소유한 parent를 모두 닫고 Linux close 오류는 재시도하지 않는다. 실패 때 descriptor 정리·주입 오류는 별도 native 시험이 필요하다.

실제 `.fl`에서 모든 256 byte·한글/이모지·순서·원본 보존·mode·경로 차단·truncate와 zero extension·descriptor 수를 확인한다. C helper는 fixture·독립적인 byte/descriptor 검사만 제공한다. 각 API의 부재 실패를 먼저 고정하고, 새 compiler의 정규 namespace로 자체 시험해야 한다. 블로그 복구 시험만으로 언어 완료 판정 금지. 새 TS/JS/Python·fl-build.sh·운영 자료 접근·서버 23/25·합치기·배포·강제 푸시 금지.
