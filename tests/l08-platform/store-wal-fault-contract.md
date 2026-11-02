# WAL 오류 때 fd 정리와 결과 구분

원래 vendor tcp-server.fls의 append는 open→write→선택적 fsync→close이며 중간 throw 시 close를 실행하지 않는다. 현재 app/store.fl의 guard·오류 차단만으로 이 descriptor 누수와 fsync 성공 뒤 close 실패의 잘못된 uncertain 분류가 해결되지 않는다. 새 정답을 먼저 고정한 뒤 app의 FreeLang append 경계에서 finally로 정리한다.

| 주입 | 필요한 진단 | 새 과정 재생 행 |
|---|---|---|
| prepare 첫 write EIO, 쓰인 바이트 0 | DB_WRITE_ABORTED_RESTART_REQUIRED | 0 |
| prepare 일부 write 후 EIO | DB_WRITE_ABORTED_RESTART_REQUIRED | 0, 잘린 마지막 prepare 무시 |
| prepare close EIO (실제 fd는 닫힘) | DB_WRITE_ABORTED_RESTART_REQUIRED | 0, prepare만 존재 |
| commit fsync EIO | DB_COMMIT_UNCERTAIN_RESTART_REQUIRED | 1 (전원 장애가 아닌 page-cache 주입) |
| commit fsync 성공 뒤 close EIO (실제 fd는 닫힘) | DB_COMMITTED_RESTART_REQUIRED | 1 |

각각 오류 한 번을 실제 WAL inode의 syscall에만 주입한다. own /proc/self/fd를 조회하여 열린 WAL fd 0개, 후속 읽기·쓰기 차단, 새 과정 reader의 기대 행·ID, reader 전후 WAL 무변경을 요구한다. Linux close 오류에서 fd는 이미 닫혔다고 보고 재시도하지 않는 기존 native fd 계약을 유지한다. 시험 driver는 실제 close를 먼저 실행한 뒤 EIO를 돌려주는 방식이다. 중간 실패도 성공으로 바꾸거나 WAL 거래를 삭제하지 않는다.

원래 WAL JSON·길이 상한·checksum·prepare/commit 형식은 유지한다. WAL 오류에 따른 복구 차단·재시작 정책도 유지한다. 운영 자료·서버 23·25·Node·TS/JS·Python 접근/실행 금지. 실제 전원 장애·원격 파일 시스템의 close 보장·catalog page 오류·health·전체 플랫폼을 이 시험으로 완료 판정하지 않는다.
