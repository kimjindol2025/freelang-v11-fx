# WAL prepare fsync 실패의 가시성 (최초 가정·아래 정정 적용)

새 합성 WAL의 테이블 준비 뒤 첫 fsync에만 실제 EIO를 주입한다. C helper는 fsync 호출 한 번의 결과만 바꾸며 DB를 직접 호출하거나 값을 교체하지 않는다. FreeLang 자체 코드가 원래 app/store.fl과 vendor의 commit을 사용한다.

commit record 이전 prepare 동기화가 실패한 쓰기는 오류를 반환하고 live 읽기에도 나타나지 않아야 한다. 뒤의 정상 저장은 성공하여 live·새 과정 WAL 재생 결과 모두 good 한 행이어야 한다. 이미 실패한 쓰기가 live에만 남는 것을 재시작으로 덮지 않는다. WAL 기록 삭제·expectation 완화 금지. reader 전후 WAL 바이트는 동일해야 한다.

commit record 자체의 fsync 실패·전원 장애의 불확실한 commit 결과, fd 정리, 다중 AST 전체 rollback, 읽기와 snapshot의 일관성은 추가 검증이 필요하다. 본 시험은 prepare 실패의 최소 계약이며 운영 자료는 접근하지 않는다.

## 실제 소스에 따른 계약 정정

최초 시험 commit `08417e3e351ab33f8ff6f48b05b89bc1eb9cf9b8`은 첫 fsync를 prepare 동기화로 잘못 가정했다. 원래 blog vendor 기준 `88b4aa2ca11e5355240a1d93ece8321315c669cc`의 tcp-server.fls:721~736을 다시 확인하면 prepare는 `sync? false`, commit만 `sync? true`다. 실제 첫 실패는 commit record를 append한 뒤의 fsync EIO였고, 처음의 무조건 rollback·재생 good 1행 기대는 이 상황에서 안전한 계약이 아니다. 원문과 이전 기대는 Git 이력 및 store-fsync-initial-contract-failure.txt에 보존한다. 기록 삭제나 구현에 맞춘 성공 처리로 바꾸지 않는다.

구현 전에 정정한 계약: commit 동기화 실패는 `DB_COMMIT_UNCERTAIN_RESTART_REQUIRED`로 명시하고 같은 과정의 뒤 읽기·쓰기는 `DB_RECOVERY_REQUIRED`로 거부한다. 완성된 commit record가 page cache에 기록된 이 주입 시험은 실제 전원 장애가 아니므로 재시작 때 첫 failed 행 1개만 재생되어야 한다. 막아야 할 두 번째 good 쓰기가 추가되면 실패다. 실제 전원 장애에서 첫 거래가 반드시 남거나 반드시 사라진다고 보장하지 않는다. 새 계약에서도 현재 구현은 세 조건을 모두 어기며 reader는 2행으로 실패한다. 이 수정은 통과 기준을 없애는 것이 아니라 불확실성 표시·복구 전 접근 차단을 추가하는 것이다.

prepare write/close 실패는 별도 주입이 필요하다. append 오류 뒤 FD 정리, commit 성공 뒤 catalog-page 오류의 정확한 결과 표시, health의 복구 필요 상태도 별도 확인한다.
