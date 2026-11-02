# L05 db95 자체 WAL 경계 재현

블로그 보정 없이 db95 원본만 로드하는 .fl 시험이다. 일반 테이블의 첫 행을 commit한 뒤 실제 prepare의 부분 기록, 완전한 prepare 뒤 부분 commit, 마지막 유효 commit의 LF 누락, invalid UTF-8 또는 NUL suffix를 합성 WAL 복사본에 만든다. 원본 합성 seed·수정 전 tail 복사본을 보존한다. 미완료 거래의 행은 재생하면 안 된다.

같은 파일에서 정상 둘째 행을 새로 commit한 뒤 별도 과정 재시작 10회 모두 정확한 두 행·한글/이모지와 WAL 무변경이 정답이다. 각 경우 resume 1 + read 10, 총 55묶음을 고정한다. 시험/정답·실패를 저장소 수정 전에 보존한다. 기록을 빼거나 기존 committed prefix를 바꿔 성공으로 만들지 않는다. 이후 native-store.fl 진입점을 비교할 수 있으나 시험 본문·정답·반복 수는 바꾸지 않는다.

이 최소 기준만으로 L05 전체 완료를 주장하지 않는다. 긴 WAL·인덱스·동시 거래·강제 I/O 실패·단일 과정 소유 및 실제 crash 내구성은 별도 증거가 필요하다. 운영 데이터·서버 23·25·배포에는 접근하지 않는다.

native 진입점 실행 후 verify-l05-db95-tail-artifacts.sh는 같은 실제 출력 폴더를 읽기만 한다. 다섯 경우 모두 SHA-256 이름의 forensic 원본 전체 byte 일치와 기존 유효 prefix byte 보존을 독립 비교한다. LF 누락 때는 LF 없는 원본 전체가 prefix다. 원래 55묶음의 반복·정답은 그대로이며 추가 비교는 forensic 5·prefix 5로 별도 집계한다.
