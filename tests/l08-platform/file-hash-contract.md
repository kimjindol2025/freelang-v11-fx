# L07 파일 SHA-256 streaming 확장 계약

새 API `file-sha256-result(path)`와 underscore 별칭은 L07의 result map `ok/bytes/error/errno/data`를 사용한다. 성공 data는 raw 파일 SHA-256의 소문자 hex 64자다. 실패 data는 nil이며 errno는 양수다. 파일 크기 상한은 기존 L07과 같은 `67108864` byte다. 원본 파일을 변경하지 않는다. 기존 API는 유지한다.

FL_FILE_BASE가 지정되면 기존 안전 경로 순회·부모/최종 symlink 거부 계약을 따른다. 일반 파일만 지원하고 FIFO/디렉터리·NUL 경로·경로 이탈·상한 초과를 거부한다. EINTR 재시도·short read 처리·읽기/close 오류 보고·descriptor 정리가 필요하다. 오류 시 부분 hash를 성공으로 반환하지 않는다. 읽은 byte 수와 파일 metadata 변화가 확인되면 FILE_CHANGED다. 같은 파일을 다른 과정이 수정하는 모든 경합에 대한 원자적 snapshot은 보장하지 않는다.

시험은 블로그 없는 .fl 프로그램이다. 외부 sha256sum을 oracle로 고정하고 빈 파일·256개 모든 byte·한글/이모지/NUL/잘못된 UTF-8·64 MiB 경계를 검사한다. 오류 주입은 test C helper가 실제 읽기/close를 대상으로 하며 C 함수만 직접 호출하는 것으로 완료를 판정하지 않는다. 각 실행의 FD 수가 복귀해야 한다. 64 MiB hash 전후 ru_maxrss 증가가 16384 KiB 미만이어야 한다. 이 한계는 전체 RSS·동시 대량 백업 인수 기준을 대신하지 않는다. 초기 정답·실패를 구현 전에 커밋하며 값은 구현에 맞춰 낮추지 않는다.

순서: 이 시험/정답 커밋 → 미연결 실패 원문/종료 코드 → native 구현 → 일반·ASan/UBSan 자체 시험 → L07 및 언어 회귀·변경하지 않은 bootstrap → 실제 blog checksum 적용·손상 거부 회귀. Node·JS·Python은 strace로 미호출을 확인한다. 운영 자료에는 접근하지 않는다.
