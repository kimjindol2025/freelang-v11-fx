# WAL prepare fsync 실패의 가시성

새 합성 WAL의 테이블 준비 뒤 첫 fsync에만 실제 EIO를 주입한다. C helper는 fsync 호출 한 번의 결과만 바꾸며 DB를 직접 호출하거나 값을 교체하지 않는다. FreeLang 자체 코드가 원래 app/store.fl과 vendor의 commit을 사용한다.

commit record 이전 prepare 동기화가 실패한 쓰기는 오류를 반환하고 live 읽기에도 나타나지 않아야 한다. 뒤의 정상 저장은 성공하여 live·새 과정 WAL 재생 결과 모두 good 한 행이어야 한다. 이미 실패한 쓰기가 live에만 남는 것을 재시작으로 덮지 않는다. WAL 기록 삭제·expectation 완화 금지. reader 전후 WAL 바이트는 동일해야 한다.

commit record 자체의 fsync 실패·전원 장애의 불확실한 commit 결과, fd 정리, 다중 AST 전체 rollback, 읽기와 snapshot의 일관성은 추가 검증이 필요하다. 본 시험은 prepare 실패의 최소 계약이며 운영 자료는 접근하지 않는다.
