# 실제 HTTP 이미지 업로드의 불확실 거래 처리

## 확인한 것

대상 blog 구현은 `9095e9571cb1493065984aada54209a7c5f37171`이다. 관리자·AI 업로드의 오류 정리를 실제 앱 HTTP 요청으로 재현하고 수정했다. 시험·정답과 실패를 구현 전에 커밋했다. 전체 블로그 완료 보고가 아니다.

최초 시험 보조기 빌드는 stdio 선언 누락으로 종료 `1`이었다. HTTP 실행 실패로 집계하지 않았다. 보조기 수정 뒤 최초 HTTP 시험은 `IMAGE_UPLOAD_HTTP_PASS=6 FAIL=12`였다. prepare write/partial 오류를 추가한 시험은 `14 FAIL=16`, 공개 홈 가용성 확인까지 추가한 수정 전 결과는 `16 FAIL=24`였다. commit fsync/close 오류 때 응답이 500이고, 재시작 후 metadata는 살아나지만 이미지 파일은 삭제되어 HTTP 404였다. 원래 정답을 낮추지 않았다.

공통 store 판정 함수로 commit 결과가 불확실하거나 이미 durable인 경우를 구분한다. 관리자·AI 업로드는 이때 파일을 보존하고 503과 재시작 필요를 알린다. 복구 필요 상태에서 AI 감사 기록을 추가로 쓰지 않는다. 공개 홈 middleware도 복구 필요 상태를 503으로 알린다. 아직 commit 전인 prepare 실패에서는 파일을 정리한다. 백업 복원도 같은 판정 함수를 사용한다. 오류 응답을 거래가 없었다는 뜻으로 해석하지 않는다.

수정 후 ASan/UBSan으로 실제 app/main과 worker `4`개를 실행했다. 관리자와 AI 각각 정상·prepare write 실패·prepare partial 실패·commit fsync 실패·commit close 실패의 `10`개 경우다. POST `10`개, 현재 공개 홈 `10`개, 독립 재시작 `20`개의 정답 결과는 다음 원문이다.

```text
IMAGE_UPLOAD_HTTP_PASS=40 FAIL=0
IMAGE_UPLOAD_AUTHENTICATION=SYNTHETIC_SESSION_AND_TEST_KEY
IMAGE_UPLOAD_PASSWORD_LOGIN=UNEXECUTED
IMAGE_UPLOAD_SANITIZER_EXIT=0
NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0
SYNTHETIC_SECRET_TYPES=3
SYNTHETIC_SECRETS_IN_EXECVE=0
```

정상과 commit 오류의 재시작 후 metadata는 `1`행, 공개 이미지 HTTP `200`, raw SHA-256 일치, 조회 전후 WAL 불변이다. prepare 실패의 재시작 후 metadata는 `0`행, 영구 이미지 폴더는 비어 있고 WAL은 불변이다. fault 발생 횟수는 정상 `0`, 각 주입 `1`이다. 빈 포트는 kernel이 선택하며 시험 자신의 과정만 종료했다.

공통 함수 변경 뒤 백업 복원의 commit fsync/close 오류도 ASan/UBSan writer·별도 reader로 다시 검증했다.

```text
BACKUP_RESTORE_FAULT_PASS=4 FAIL=0
IMAGE_POLICY_RESTORE_SANITIZER_EXIT=0
NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0
```

원본 합성 archive와 reader 전후 WAL은 그대로다. 언어 전체 회귀 고정 `85a90c1b0f3f6e7b561ad4ad6f3c589244c33994` 이후 runtime/self/전체 driver diff는 `0`이다. 해당 전체 실행 종료 `0`과 bootstrap 통과는 앞선 보고에 기록했다. 이번에는 앱 오류 처리·시험 보조기를 검증했으며 언어 전체 시험을 새로 실행했다고 쓰지 않는다.

## 명령과 근거

실행 전 승인된 소스 전용 복사본·검증된 native compiler·격리한 출력 폴더·기존 sanitizer 의존 경로를 환경에 지정했다. 실제 경로는 `<가림>`이다. trace 아래 실행한 driver는 `bash scripts/verify-l08-blog-image-upload-faults.sh`, 복원 회귀는 `bash scripts/verify-l08-blog-backup-restore-faults.sh`다. execve 감사와 `bash scripts/audit-l08-test-secrets.sh`로 Node/JS/Python 호출과 합성 Cookie/Bearer/CSRF 값의 명령 인자 노출을 확인했다. 키 값 자체는 출력·커밋하지 않았다.

- [시험 계약](../tests/l08-platform/image-upload-fault.md), [고정 정답](../tests/l08-platform/image-upload-fault.expected)
- [첫 빌드 실패](evidence/l08-platform/image-http-first-build-failure.txt), [최초 HTTP 실패](evidence/l08-platform/image-http-before.txt), [확장 실패](evidence/l08-platform/image-http-expanded-before.txt), [가용성 포함 실패](evidence/l08-platform/image-readiness-before.txt)
- [수정 후 HTTP 원문](evidence/l08-platform/image-http-after.txt), [백업 복원 회귀 원문](evidence/l08-platform/image-policy-restore-regression.txt)
- [앞선 전체 회귀와 bootstrap](L08-bootstrap-backup-progress.md)

## 미확인

인증은 합성 세션·시험용 AI key이며 실제 비밀번호 로그인은 미실행이다. 빈 scrypt 검증값 처리의 사용자 결정도 대기 중이다. 원본 표시용 raw GIF prefix fixture는 유효 이미지 decoder·브라우저 렌더링 증거가 아니다. 기존 편집기 JS는 제공했지만 실행하지 않았다.

동시 백업/미디어 경합, 진행 중인 다른 요청의 모든 오류 응답, 이미지 파일·상위 directory 내구성, 실제 전원 차단·중단 작업 재조정, 대량 미디어 streaming hash·메모리 상한은 남아 있다. DB 일반 보정은 현재 app/store에 있어 L05의 ‘블로그 보정 없이 저장소 자체 통과’도 아직 증명하지 않았다. 브라우저 J안·편집기·이미지 처리·무방문 예약·이력·운영 기능·필수 계정/댓글·화면·승인된 운영 복사본 호환은 계속 필수다.

서버 23·25 접속·운영 자료 접근/변경·백업 원본 삭제·합치기·배포·강제 푸시는 하지 않았다. FreeLang·C·Bash·Markdown만 변경했고 새 TS·수작업 JS·Python과 fl-build.sh를 사용하지 않았다.
