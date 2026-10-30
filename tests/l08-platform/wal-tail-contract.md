# WAL 마지막 경계 뒤 쓰기 재개

현재 store는 마지막 비정상 JSON 조각을 읽을 때 무시하지만 그 뒤 새 기록을 붙이면 같은 줄에 합쳐질 수 있다. 완성된 마지막 commit에 개행만 없는 경우도 같은 경계 문제가 있다. 읽기 재생만 통과한 것으로 쓰기 재개를 정상 판정하지 않는다.

구현 전에 세 상황의 정답을 고정한다: prepare 일부 write 후 EIO, commit 일부 write 후 EIO, 완성된 commit 뒤 마지막 LF 하나만 없는 WAL. 실제 FreeLang store와 실제 syscall 주입으로 만든다. LF 제거는 새 합성 fixture에서만 수행하며 거래·정답을 제거하는 우회가 아니다.

먼저 한글·이모지 제목을 가진 kept 한 건을 정상 commit하고 그 시점 WAL 전체를 독립적인 committed-prefix.wal로 보관한다. 중단 후 새 과정에서 kept 내용이 그대로인지 확인하고 after-restart 한 건을 저장한다. 다시 새 과정으로 10회 시작하여 매번 정확한 두 ID·제목과 2행을 확인한다. interrupted 행은 미완성 거래이므로 재생되면 안 된다.

기존 유효한 commit의 byte prefix는 그대로 유지한다. 경계 정리가 필요했다면 정리 전 전체 WAL을 `.recovery-원문SHA256` 사본으로 남겨 원본 byte와 동일하게 검증한다. 이미 존재하는 백업은 덮어쓰거나 지우지 않는다. reader 10회 동안 WAL과 백업은 바뀌면 안 된다. 운영 WAL·이미지·설정·원본 백업에는 접근하지 않는다.

수정은 일반적인 복구 경계 처리여야 한다. 정상 거래를 빼서 통과시키거나 malformed 중간 기록을 임의로 건너뛰지 않는다. 필요한 raw byte·배타적 백업 생성·durable 파일 연산이 언어에 없다면 C/Bash 우회로 앱을 구현하지 않고 FreeLang 언어 API로 추가하여 자체 시험한다. 여기의 C는 syscall fault helper와 독립적인 fixture 판정에만 사용한다. 전체 crash·백업/복원 완료와 구분한다.
# 복구 재시도

`L08_BLOG_WAL_TAIL_PRESEED_BACKUP=1`은 세 경우 모두 복구 전에 원본과 같은 백업을 만들어 둔다. 프로세스가 백업 저장 뒤 중단된 상황을 모사한다. 동일한 재개·10회 재시작·유효 prefix·원본 백업 불변 정답을 그대로 적용한다. 기존 백업을 덮어쓰거나 지워서 통과시키지 않는다. 실제 전원 차단 시험의 대체는 아니다.

