# L05 db95 자체 검증 진행

## 확인한 것

PRD는 블로그 보정 코드를 빼도 db95 자체의 재생·거래·조회가 통과해야 한다고 정한다. app/store 통과를 이 조건의 증거로 대신하지 않는다. 이번 시험은 app/store나 블로그 테이블을 전혀 로드하지 않는다.

DB 기준은 `88b4aa2ca11e5355240a1d93ece8321315c669cc`다. 승인된 소스 복사본의 src 파일 `31`개를 기준 커밋의 blob과 각각 SHA-256 대조하여 전부 일치함을 확인했다. DB 원본 checkout은 그대로이며 운영 자료에는 접근하지 않았다.

시험·정답 고정 `e9607f9cbe2985d11a73906aca0a5ae3c2a3cd02`는 일반 레코드 삽입, 여러 AST의 수정/삭제/재삽입, 실패 batch의 catalog·기본/보조 인덱스 rollback과 실패 전후 WAL hash 일치를 검사한다. 실제 조회 결과의 plan이 index-scan이어야 한다. 이후 독립 과정 재시작 `10`회도 같은 정답·WAL 불변이어야 한다. fx runtime 구현은 `043151bd578774b86d8a1de7760b071137cd407b`이며 native compiler로 .fl을 생성 C로 컴파일해 ASan/UBSan 실행했다.

```text
DB95_INDEX_ATTEMPT=10 PHASE=read EXIT=0
DB95_INDEX_GROUP_PASS=11 FAIL=0
DB95_INDEX_BASELINE_EXIT=0
NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0
```

이 경우에는 DB 소스를 수정하지 않았다. 이 작은 시험의 통과만으로 블로그의 scan 우회를 바로 없애거나 L05 전체 완료로 판정하지 않는다. 원문은 docs/evidence/l08-platform/db95-index-baseline.txt다.

## 실행 명령

승인된 DB src·검증된 native compiler·전용 출력과 sanitizer libdir을 L05_DB_SOURCE_ROOT/L05_DB_NATIVE_COMPILER/L05_DB_INDEX_OUT/L05_DB_SANITIZER_LIBDIR에 지정했다. 실제 경로는 `<가림>`이다. `strace -f -e trace=execve` 아래 `bash scripts/verify-l05-db95-index.sh`를 실행하고 `bash scripts/audit-l07-execve.sh`로 trace를 감사했다.

## 미확인 / 진행 중

고정 `2ff24ad6fbed7f6dba6e8301733bdc6dce70cdba`의 WAL 경계 시험은 실행 중이다. 실제 prepare 일부·완전한 prepare 뒤 commit 일부·LF 누락·invalid UTF-8·NUL의 다섯 상태에서 정상 쓰기를 재개한 뒤 각각 독립 재시작 `10`회가 정답이다. resume `5` + reader `50`의 `55`묶음을 먼저 고정했으며 아직 결과를 통과로 기록하지 않는다. 운영 WAL이나 기존 committed 거래를 빼는 시험이 아니다.

일반 WAL 보정·운영 guard는 아직 app/store에 남아 있다. 이 문제의 저장소 자체 해결, 긴 WAL·여러 작업자·I/O 실패·다중 과정 소유·실제 crash 내구성은 별도 작업이다. 새 파일 해시 runtime의 전체 언어 회귀/bootstrap도 진행 중이다.

db95 submodule에는 GitHub 원격만 있다. Forgejo 공개 저장소 검색은 HTTP `403`이었으며 정확한 Forgejo 주소를 사용자에게 요청했다. 주소를 추측하여 제3 저장소에 푸시하지 않는다. 이는 fx/blog1의 이미 확인된 두 원격과 구분한다. DB 자체 변경·합치기·배포는 아직 하지 않았다.

테마·플러그인을 제외한 워드프레스 같은 플랫폼 전체가 목표이며, 로그인·브라우저 편집기·이미지 처리·예약·이력·운영 기능·필수 댓글/다중 작성자·화면·운영 복사본 인수는 계속 필요하다. 서버 23·25 접속·운영 자료 변경·원본 백업 삭제·합치기·배포·강제 푸시·새 TS/수작업 JS/Python·fl-build.sh 사용은 없다.
