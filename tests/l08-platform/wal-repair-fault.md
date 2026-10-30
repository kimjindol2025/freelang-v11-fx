# 복구 중 syscall 오류 계약

실제 FreeLang store의 첫 새 append 경계에서 백업 부분 write→EIO, 백업 fsync, parent fsync, 백업 close, WAL truncate, WAL 복구 fsync, WAL 복구 close를 각각 한 번 주입한다. helper는 새 합성 WAL·forensic 파일·그 parent inode만 확인한다. close 오류는 Linux처럼 실제 fd를 먼저 닫고 EIO를 반환한다. 다른 파일·과정에는 주입하지 않는다. 기존 거래·정답·worker 수는 바꾸지 않는다.

오류 시 해당 거래는 거부, 같은 과정의 후속 읽기·쓰기는 차단, 열린 descriptor 수는 원래 값으로 돌아와야 한다. backup write/fsync/parent fsync/close·truncate 오류까지 원래 WAL 전체가 그대로여야 한다. 복구 fsync/close 오류는 이미 잘린 불완전 suffix가 제거될 수 있으나 모든 valid committed prefix와 full forensic 원본은 그대로여야 한다. 어느 경우에도 새 거래를 추가하면 안 된다.

부분 백업 write는 불완전 forensic 파일을 남길 수 있다. 이 파일은 삭제·덮어쓰지 않고 다음 재시도가 거부되는지 확인한다. 그 상태를 자동 복구 성공으로 집계하지 않는다. 나머지 경우는 이미 저장된 동일 원본을 재검증·동기화하여 새 과정에서 재개한 뒤 기존 verify 정답으로 두 행·내용·유효 prefix·백업 무변경을 확인한다. 실제 전원 차단·파일시스템 durability의 대체는 아니다.
