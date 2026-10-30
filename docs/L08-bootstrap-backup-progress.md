# 자기 컴파일과 백업·복구 후속 검증

## 확인한 것

목표는 테마·플러그인을 제외한 워드프레스 같은 블로그 플랫폼 전체다. 이번 부분 통과를 플랫폼 완료로 판정하지 않는다. 서버 23·25 접속, 운영 자료·설정·계정 접근/변경, 합치기·배포·강제 푸시·백업 원본 삭제는 하지 않았다. 승인된 소스와 새 합성 자료만 사용했다. 새 TS·수작업 JS·Python 파일과 fl-build.sh 사용은 없다.

### 변경하지 않은 자기 컴파일 검증 통과

앞선 고정 소스는 stage3 생성까지 끝났지만 `STAGE2_EQ_STAGE3=FAIL`, `WAL_PARSER_BOOTSTRAP_EXIT=1`이었다. 기대값·기준 stage0·상위 검증 스크립트는 바꾸지 않았다. wrapper에 생성 C와 실행물 보관만 추가하여 cleanup 전에 진단 자료를 남겼다.

완전한 compiler 소스 식을 나누어 비교하니 91개 동일 뒤 replace-pct-list에서 차이가 났다. 고정 seed가 만든 stage1의 C 함수 인자 평가 순서 때문에 cgc-if-branch가 임시 번호를 다른 순서로 할당했다. 최초 식·생성 C diff·실패를 먼저 남겼다. 구현 `a3afe725981f50f090ade58d21a37fe3f750738f`는 tail/ordinary 코드 생성의 부작용을 let에서 명시적으로 순서화한다. stage1과 앞선 stage2의 완전한 식 275개 비교는 `BOOTSTRAP_FORM_PASS=275 FAIL=0`, `BOOTSTRAP_ORDER_EXIT=0`이다. 진단 비교에서 임시 이름을 정규화하지 않았다.

이후 고정 `37aa457b9487192197eb10966969fd0a33d3c077`에서 원래 verify-native-bootstrap.sh 전체를 다시 실행해 `BOOTSTRAP_SEQUENCED_EXIT=0`을 확인했다. stage2→stage3 생성·바이트 동일성·배열 반복 생성/빌드/실행·잔여 과정 0을 모두 통과했다. 이 독립 검증은 전체 언어 회귀와 구분한다.

```text
STAGE2_EQ_STAGE3=PASS
ARRAY_LOOP=PASS
NODE_INVOCATIONS=0
NPM_INVOCATIONS=0
TS_JS_INVOCATIONS=0
CGC_BIN_INVOCATIONS=0
AFJ_FILE_ACCESSES=0
STAGE0_SHA=f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c
STAGE1_SHA=cb872631385823f7b41a8c26b09faeb155b4cab649cb42b33343d24f3a5f8696
STAGE2_SHA=a51faa52652845e488ff7920a16e85ad8f2a147b99089c0ebf623dd02f4ead3a
STAGE3_SHA=a51faa52652845e488ff7920a16e85ad8f2a147b99089c0ebf623dd02f4ead3a
RESIDUAL_PROCESSES=0
```

상위 검증 파일 SHA-256은 `2899b57106ddbfceb611a54fc3af34fdf998961effe6d67e5b6289bcc1b769cb`로 그대로다. 원문의 STAGE0_EQ_CANONICAL=DIFFERENT도 그대로 보존했다. 지정된 stage0 SHA를 다른 것으로 바꾸지 않았다. 식 비교용 compiler.c도 위 stage1.c와 같은 `cb872631385823f7b41a8c26b09faeb155b4cab649cb42b33343d24f3a5f8696`이다. 확보한 trace의 공통 감사도 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다.

### WAL 경계 복구 중 오류

실제 store에 백업 부분 write, 백업 fsync, parent fsync, 백업 close, WAL truncate, WAL 복구 fsync, WAL 복구 close를 주입했다. ASan/UBSan에서 `WAL_REPAIR_FAULT_PASS=21 FAIL=0`, `WAL_REPAIR_FAULT_SANITIZER_EXIT=0`이다. 같은 과정의 후속 읽기·쓰기 차단과 descriptor 수 복귀를 확인했다. truncate 전 오류에서는 전체 WAL이 그대로이며 truncate 이후 오류에서는 valid prefix와 full forensic 원본이 유지된다. 부분 백업 write가 남긴 불완전 forensic 파일은 삭제·덮어쓰지 않고 재시도를 거부하는 상태를 확인했다. 이것을 자동 복구 성공으로 집계하지 않는다. 나머지 여섯 경우는 새 과정에서 재개·재시작하여 정확한 두 행과 백업·WAL 보존을 확인했다.

앞선 immutable snapshot runtime 고정 `ad68090d7439e1ac867d611d8f8725854b763127`에서도 실제 앱의 저장 오류 10, HTTP 32건 200·중복 8건 409·재시작 2회, 전체 앱 3회, raw WAL 경계 120, 잘못된 백업 거부 8의 다섯 단계가 모두 끝났다. `WAL_PARSER_APP_PIPELINE_EXIT=0`이다. 원래 WAL 거래·worker·정답을 줄이지 않았다.

### 같은 크기인 이미지 손상의 사전 거부

blog의 실제 export/import를 native `.fl`에서 호출했다. 최초에는 원본 백업의 복사본에서 이미지 byte 하나만 바꿔도 복구가 성공했고 대상 WAL이 바뀌었다. 정답·시험을 먼저 고정했으며 `BACKUP_INTEGRITY_PASS=2 FAIL=1`, `BACKUP_INTEGRITY_BEFORE_EXIT=1` 원문을 보존했다. 원본 백업은 무변경이었다.

blog 구현 `bd252ac332fcbd5756ea8b2040d58ffa8880a113`은 새 version 2 manifest에 image_hashes를 기록하고 복구 전에 raw byte SHA-256을 확인한다. 기존 version 1/2의 무해시 archive 호환은 유지한다. 이 이전 형식은 추가 내용 검증 정보가 없으며 같은 수준의 무결성 증거라고 주장하지 않는다. manifest 서명·인증을 제공하지 않는다. 실제 백업과 관리자/AI 이미지 publication·deletion에 같은 store guard를 적용했다. 실제 경합 시나리오는 아직 별도 검증이 필요하다.

정상 복사본·같은 크기 손상·대상 불변의 최초 수정 후 결과는 `BACKUP_INTEGRITY_PASS=3 FAIL=0`, `BACKUP_INTEGRITY_AFTER_EXIT=0`이다. 외부 sha256sum과 manifest의 raw image hash도 둘 다 `20ba11a1616ddf24770001eb7888d27e2b32d153a4257dda9029eadf5a573f56`이다. fixture는 GIF prefix·NUL·ff를 가진 raw byte이며 완전한 GIF decoder나 이미지 처리 시험을 뜻하지 않는다.

최종 blog `120d5d453905c37434af3d6f652c482f235ed16d`에서는 ASan/UBSan으로 정상 복원, 무해시 과거 version 2 복원, 같은 크기 손상, null/빈/wrong-key/잘못된 hex/불일치 checksum, 잘린 이미지, 경로 이탈 ID, 다른 blog 행, 예상 밖 파일까지 확장했다. `BACKUP_INTEGRITY_PASS=13 FAIL=0`, `BACKUP_BOUNDARIES_EXIT=0`이다. 모든 거부에서 대상 WAL byte·metadata·이미지 부재와 원본 archive·외부 sentinel의 무변경을 확인했다. 앞서 같은 소스로 빌드한 ASan 실행물을 재사용했으며 seed SHA-256 `9721a62ca8fe9684fb135775c403adcd167b0f2c8a0fe3f03bd3e70b4a958a9c`, check `ce6761c4b14081dea47cc96861c481db1a9bb560cdbff59921adde732540c377`은 실행 뒤에도 같다.

### 불확실 commit 뒤 이미지 유실 방지

다음 실제 복구 시험은 commit fsync/close 실패 뒤 metadata가 재시작에서 살아나는데 catch가 이미지를 지우는 문제를 재현했다. 시험·정답과 실패 `BACKUP_RESTORE_FAULT_PASS=0 FAIL=4`, `BACKUP_RESTORE_BEFORE_EXIT=1`을 먼저 남겼다. blog 구현 `120d5d453905c37434af3d6f652c482f235ed16d`는 commit 결과가 불확실하거나 이미 durable인 경우 복사한 이미지를 남기고 BLOG_BACKUP_RESTORE_RESTART_REQUIRED로 구분한다. 이를 ‘아무 변경 없이 실패’라고 보고하지 않는다. 재시작 후 결과 확인이 필요하다.

수정 후 ASan/UBSan writer·독립 reader 각각 두 경우는 `BACKUP_RESTORE_FAULT_PASS=4 FAIL=0`, `BACKUP_RESTORE_SANITIZER_EXIT=0`이다. 오류 표시·이미지 보존·WAL fd 0, 재시작 뒤 1글·1이미지와 이미지 raw hash, reader 전후 WAL 및 원본 archive 무변경을 확인했다. 실제 전원 차단의 결과를 대신하지 않는다.

같은 최종 blog의 checksum ASan 3묶음, 원래 version 1 fixture export/import/nonempty 거부 3묶음, 전체 main·seed·editor-http 빌드·HTTP 3회·재시작 2회의 pipeline은 모두 종료 0, `BACKUP_FINAL_PIPELINE_EXIT=0`이다. nonempty의 실제 프로그램 종료 1은 기대한 거부 결과이며 suite 실패로 집계하지 않는다. 합성 세션만 사용했다. `PASSWORD_LOGIN=UNEXECUTED`, `EXISTING_EDITOR_JS=SERVED_NOT_EXECUTED`다.

- language implementation과 전체 driver를 고정한 `85a90c1b0f3f6e7b561ad4ad6f3c589244c33994`의 전체 회귀·L06/L07·포함된 bootstrap은 `SEQUENCED_PLATFORM_FULL_EXIT=0`으로 끝났다. 원문은 `L08_P3_PASS=11 FAIL=0`, `L08_P2_PASS=11 FAIL=0`, `L08_PASS=17 FAIL=0`, `L06_HTTP_PASS=110 FAIL=0`, `L06_TLS_PASS=110 FAIL=0`, `L07_SELF_PASS=221`, `L07_SELF_FAIL=0`, `STAGE2_EQ_STAGE3=PASS`, `BOOTSTRAP_EXIT=0`, `L08_PLATFORM_CURRENT_GATES=PASS`다. 공통 감사는 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다. 이후 runtime/self/전체 driver 차이는 0이다. `L08_AUTH_PARTIAL_PASS=5 FAIL=0`은 아래 미구현 비밀번호 검증을 포함하지 않는다. 이 driver의 현재 gate 통과를 C13 전체나 PRD 완료로 확대하지 않는다. [전체 원문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/blog-platform-completion/docs/evidence/l08-platform/sequenced-full-final.txt). 이전 ad68090 고정의 전체 실행은 코드 생성 차이를 확인하고 수정 후 독립 gate 통과를 검증한 뒤 소유 과정만 중단해 `WAL_PARSER_FULL_EXIT=143`이다.

## 미확인 / 남은 필수 작업

- 실제 비밀번호 검증은 빈 scrypt 검증값을 거부할지 source 동작을 보존할지 사용자 결정 대기다. 질문을 갱신했고 답변을 가정하지 않았다. C13 전체·실제 비밀번호 로그인은 완료가 아니다.
- 관리자·AI 업로드의 불확실 commit 정리도 실제 HTTP 오류로 재현하여 파일 보존을 검증해야 한다. backup과 이미지 작업의 실제 동시 경합·복원 미리보기/운영 상태·중단된 작업 재조정은 남아 있다.
- checksum 구현은 이미지를 request arena로 읽는 제한이 있다. 대량 미디어 인수 전에 streaming file hashing과 메모리 상한 검증이 필요하다. 복구 이미지와 상위 directory의 내구성, 실제 crash·전원 차단·원래 코드 복귀, 1000글/1000이미지·동시 10명 p95·일반 LSan/장시간 RSS도 미확인이다.
- 브라우저 J안·편집기·이미지 처리·무방문 예약·버전/운영 기능, 필수 댓글·다중 작성자, 실제 화면과 승인된 운영 자료 복사본 호환은 전체 목표에 계속 포함한다. 계정·댓글의 마지막 구현 순서는 유지한다. 운영 배포 준비 완료가 아니다.

## 명령과 원문

시험 소스 복사본·compiler·결과·의존성의 실제 경로는 `<가림>`이다. 원문은 공백과 숫자를 보존하고 해당 경로만 가렸다.

```bash
unshare --user --map-root-user --pid --fork --mount-proc env L07_OUT="$out" bash "$snapshot/scripts/verify-l07-bootstrap.sh"
L08_BOOTSTRAP_STAGE1="$stage1" L08_BOOTSTRAP_STAGE2="$stage2" L08_BOOTSTRAP_FORMS_OUT="$out" bash "$snapshot/scripts/verify-l08-bootstrap-forms.sh"
L08_PLATFORM_OUT="$out" L08_TIMER_SANITIZER_LIBDIR="$sanitizer_lib" bash "$snapshot/scripts/verify-l08-platform-all.sh"
```

앱 시험은 source-only 복사본·검증된 native compiler를 L08_BLOG_SOURCE_ROOT·L08_BLOG_NATIVE_COMPILER로 지정했다. 실행한 driver는 verify-l08-blog-wal-repair-faults.sh, verify-l08-blog-backup-integrity.sh, verify-l08-blog-backup-restore-faults.sh, verify-l08-blog-legacy-backup.sh, verify-l08-blog-editor-native.sh다. 각 driver는 strace execve 아래 실행했고 sanitizer driver에는 보관한 libdir을 지정했다. 확보한 종료 trace 감사는 각각 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다.

원문: [자기 컴파일 최초 불일치](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/blog-platform-completion/docs/evidence/l08-platform/bootstrap-full-inequality-captured.txt), [식별자 순서 최초 차이](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/blog-platform-completion/docs/evidence/l08-platform/bootstrap-form-order-before.txt), [275식 동일](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/blog-platform-completion/docs/evidence/l08-platform/bootstrap-form-order-after.txt), [변경하지 않은 gate 통과](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/blog-platform-completion/docs/evidence/l08-platform/bootstrap-sequenced-final.txt), [WAL 복구 오류 21](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/blog-platform-completion/docs/evidence/l08-platform/wal-repair-faults-sanitizer.txt), [손상 archive 최초 실패](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/blog-platform-completion/docs/evidence/l08-platform/backup-integrity-before.txt), [checksum 수정 후](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/blog-platform-completion/docs/evidence/l08-platform/backup-integrity-after.txt), [신뢰 경계 13](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/blog-platform-completion/docs/evidence/l08-platform/backup-boundaries-final.txt), [복구 이미지 유실 최초 실패](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/blog-platform-completion/docs/evidence/l08-platform/backup-restore-faults-before.txt), [이미지 보존 후](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/blog-platform-completion/docs/evidence/l08-platform/backup-restore-faults-after.txt), [최종 앱 회귀](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/blog-platform-completion/docs/evidence/l08-platform/backup-final-app-regression.txt).
