# WAL 경계 복구와 native 메모리 보완

이 문서는 앞선 발행 당시 기록이다. 이후 자기 컴파일의 코드 생성 순서 수정·독립 gate 통과·백업 후속 결과는 [최신 후속 보고](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/blog-platform-completion/docs/L08-bootstrap-backup-progress.md)에 있다.

## 확인한 것

플랫폼 전체 완료 보고가 아니다. 운영 자료·설정·계정에 접근하지 않고 승인된 소스 복사본과 새 합성 WAL만 사용했다. 서버 23·25 접속, 합치기·배포·강제 푸시, 백업 원본 삭제는 하지 않았다. 새 TS·수작업 JS·Python 파일은 없다.

### 실패를 먼저 고정

기존 store는 마지막 불완전 JSON을 재생할 때 무시하지만 그 뒤 새 JSON을 같은 줄에 붙였다. 쓰기 재개는 성공해도 다음 30회 재시작이 모두 `WAL_CORRUPT`였다. 최초 세 경우의 결과 원문은 `WAL_TAIL_PASS=6 FAIL=37`, `WAL_TAIL_BEFORE_EXIT=1`이다. 37은 재시작 실패 30과 백업·prefix 보존 검사 실패 7이며 서로 다른 거래 37개를 뜻하지 않는다. 거래를 삭제하거나 정답을 줄이지 않았다.

새 native API의 최초 실패는 `bytes-count`·`fd-create` 미발견, `REPAIR_IO_GROUP_PASS=0 FAIL=2`, `REPAIR_IO_BEFORE_EXIT=1`이다. 기존 forensic 백업을 재사용하는 데 필요한 `fd-open-with-parent`도 시험·정답을 먼저 커밋하고 미발견 `REPAIR_IO_GROUP_PASS=1 FAIL=1`, `REOPEN_BEFORE_EXIT=1`을 보존했다. 기존 백업이 있으면 복구가 거부되던 앱 실패도 별도로 남겼다.

### 구현과 언어 자체 시험

native I/O 구현 `5a593883ba5a54d26b3a47cb7f62ec9a1110d32c`는 raw `bytes-count`·`bytes-slice`, `fd-read-bytes`·`fd-write-bytes`, `fd-create`, `fd-open-with-parent`, `fd-sync-parent`, `fd-truncate`를 제공한다. 기존 텍스트 API의 UTF-8 변환·UTF-16 길이 계약과 fd-open의 descriptor 하나 소유를 유지한다. 새 생성은 secure parent walk·O_EXCL·O_NOFOLLOW·0600, parent 소유 handle은 실제로 열었던 directory fd를 동기화한다.

모든 256 byte, NUL·잘린 UTF-8·한글·이모지, 인자 평가 순서, raw 파일 왕복, 경로 이탈·symlink·기존 파일 차단, mode·descriptor 정리, truncate prefix·zero extension을 실제 `.fl`로 확인했다. 일반·ASan/UBSan 각각 `REPAIR_IO_GROUP_PASS=2 FAIL=0`, `REPAIR_FINAL_EXIT=0`, `REPAIR_FINAL_SANITIZER_EXIT=0`이다. 2는 자체 시험 묶음 수다. 첫 ASan 실행 중 정답 파일을 추가해 비교가 실패한 원문도 보존했고, 이후 소스·정답을 고정한 재실행으로 확인했다.

### 실제 blog 복구와 재시도

blog 구현은 `65d6b5b6be32df29f5f0cd79f940dccf8e45cf38`이다. 첫 새 append 전에 raw 입력 전체를 `.recovery-<SHA-256>` forensic 파일로 배타적으로 보관하고 file·parent fsync·크기·SHA-256을 확인한다. 이미 있는 백업은 같은 열린 handle로 동일성을 확인한 뒤 재동기화한다. 유효한 마지막 JSON에 LF만 없으면 LF를 더한다. 원래 복구가 허용한 불완전 마지막 조각은 별도 원본을 먼저 보존한 뒤 그 조각의 바이트 범위만 줄인다. 유효한 committed prefix는 바이트 그대로 남는다. 복구 오류 뒤 같은 과정의 읽기·쓰기는 차단한다.

fx 고정 `911d718564ad90cfb953d52f7271824b666a6f88`·새 native compiler·위 blog 소스의 세 경우와 두 백업 상태 시험은 `WAL_TAIL_PASS=72 FAIL=0`, `WAL_RETRY_FINAL_EXIT=0`이다. driver `23e670ae068925b1f856fcdf662e3659b40b3525`에서 invalid UTF-8·NUL 조각을 추가한 다섯 경우·두 상태는 `WAL_TAIL_PASS=120 FAIL=0`, `WAL_RAW_EXIT=0`이다. 각 조합 create·resume·verify 10회, 총 fresh verify 100회를 포함한다. 매번 정확한 두 ID·한글/이모지 내용·2행, 유효 prefix·forensic 원본·read-only 재시작 뒤 WAL 무변경을 확인했다.

확장 raw 시험은 같은 소스로 방금 빌드한 세 실행물을 재사용했다. SHA-256은 create `fe4b3e115cb4f391cec4f66edbbbfc6f012a4f3156cc3fbe8514629cbc05e718`, resume `6cd6b606bc5cfd63da2bc714b7c45bf425363bcd31939c9b637e8187b7ec4433`, verify `df4749648e2fa5cd0fc4e313c792df6db58f96c99d4e83cd24a17901043b2bc7`이며 실행 뒤 다시 동일함을 확인했다. native compiler·앱 구현 변경을 재빌드 없이 검증했다고 주장하지 않는다.

잘못된 내용·짧은 백업·덧붙여진 백업·symlink 백업은 실제 store에서 거부한다. writer·별도 reader 각각 4개, `WAL_REFUSE_PASS=8 FAIL=0`, `WAL_REFUSE_EXIT=0`이다. WAL·백업·외부 sentinel은 무변경, 오류 뒤 읽기 차단·descriptor 수 복귀를 확인했다.

같은 fx `911d718564ad90cfb953d52f7271824b666a6f88`로 빌드한 최신 blog의 ASan/UBSan 저장 오류는 `WAL_FAULT_PASS=10 FAIL=0`이다. 실제 HTTP는 저장 32건 200·중복 8건 409·worker/peak 4·동시 curl 10·재시작 2회 정확한 32행·WAL 무변경이다. 전체 main·seed·editor-http native 빌드, HTTP 3회, 재시작 2회도 종료 0, `REPAIR_APP_PIPELINE_EXIT=0`이다. 합성 세션만 사용했으며 `PASSWORD_LOGIN=UNEXECUTED`, `EXISTING_EDITOR_JS=SERVED_NOT_EXECUTED`다.

### 큰 자기 컴파일의 메모리

최초 parser 시험·정답 `44d0cc312e0d1630fee8a06194d03d840d8af050`은 lexer가 만든 400 token을 준비한 뒤 실제 parser의 100개 AST 전부와 live 67108864·peak 83886080 byte 상한을 고정했다. AST는 맞지만 live `67613728`, peak `67871115`, `PARSER_BEFORE_EXIT=1`이다. local loop로 바꾼 시도는 live `67591728`, peak `67849177`, `PARSER_LOCAL_EXIT=1`로 부족했으며 원문을 남기고 그 변경은 되돌렸다. 상한·입력·정답은 바꾸지 않았다.

native 할당 주소별 계측으로 반복 snapshot의 fallback 할당을 확인했다. 구현 `77a2ded366344af091b6153cea70c3032d891afa`는 이미 heap이 소유한 불변 string/vector/map을 참조 유지하며, snapshot도 caller의 arena 정리 때 참조를 해제한다. arena 값·mutable builder·atom·closure의 기존 구분은 유지한다. 같은 원래 parser에서 live `1998872`, peak `2062056`, 모든 AST와 상한 통과, `PARSER_SHARED_EXIT=0`, `COMPILER_PARSE_SELF_EXIT=0`이다. 이것은 전체 RSS나 일반 누수 검사 완료가 아니다.

이전 고정 소스의 큰 실행은 최소 parser 실패와 같은 self 소스임을 확인하고 소유한 과정만 중단했다. 마지막 관찰 RSS `61982020` KiB는 peak 측정이 아니다. 이전 전체 실행은 `LEXER_FINAL_PLATFORM_FULL_EXIT=1`, 그 안의 `BOOTSTRAP_EXIT=143`이다. 다른 독립 실행은 `LEXER_FINAL_BOOTSTRAP_EXIT=137`이며 종료 원인은 확정하지 않았다. parser 보완 전 fx 고정 `911d718564ad90cfb953d52f7271824b666a6f88`의 전체 재시험도 소유한 과정만 중단해 `REPAIR_PLATFORM_FULL_EXIT=143`이다. 어느 것도 통과로 바꾸지 않는다.

## 미확인 / 진행 중

- 최신 고정 `ad68090d7439e1ac867d611d8f8725854b763127`의 전체 언어 회귀·L06/L07·변경하지 않은 verify-native-bootstrap.sh와 실제 앱 재검증은 별도로 실행 중이다. 앞선 I/O·앱 통과를 새 GC 구현 전체 통과로 대신하지 않는다.
- 실제 전원 차단, backup write/file fsync/parent fsync·truncate 오류의 새 주입, 복구 중 재차 중단되는 모든 시점, 일반 LSan·장시간 RSS·대규모 부하·여러 앱 과정의 동일 WAL 소유는 남아 있다.
- 실제 비밀번호 로그인과 빈 scrypt 키 처리의 사용자 결정, 생성 브라우저 J안·편집·이미지 처리, 예약·이력·운영 기능, 댓글·다중 작성자, 실제 화면, 승인된 운영 자료 복사본 호환·백업/복원·성능 인수는 여전히 전체 목표에 포함된다.

## 명령과 원문

경로·설정 값은 `<가림>`으로 표시한다. 아래 driver는 고정 소스 복사본에서 실행했다. 출력·종료 코드는 연결한 원문에 있다.

```bash
L08_REPAIR_IO_OUT="$out" bash "$snapshot/scripts/verify-l08-repair-io.sh"
L08_REPAIR_IO_COMPILER="$compiler" L08_REPAIR_IO_OUT="$out" L08_REPAIR_IO_SANITIZER_LIBDIR="$sanitizer_lib" bash "$snapshot/scripts/verify-l08-repair-io.sh"
L08_BLOG_SOURCE_ROOT="$blog_copy" L08_BLOG_NATIVE_COMPILER="$compiler" L08_BLOG_WAL_TAIL_OUT="$out" bash "$snapshot/scripts/verify-l08-blog-wal-tail.sh"
L08_COMPILER_PARSE_COMPILER="$compiler" L08_COMPILER_PARSE_OUT="$out" bash "$snapshot/scripts/verify-l08-compiler-parse-memory.sh"
L08_PLATFORM_OUT="$out" L08_TIMER_SANITIZER_LIBDIR="$sanitizer_lib" bash "$snapshot/scripts/verify-l08-platform-all.sh"
unshare --user --map-root-user --pid --fork --mount-proc env L07_OUT="$out" bash "$snapshot/scripts/verify-l07-bootstrap.sh"
```

자체·앱 실행물·compiler 빌드는 strace execve로 확인했다. 확보한 종료 trace 감사는 각각 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다. 소스 diff 공백 검사와 민감 정보 패턴 검사 `SENSITIVE_DIFF_SCAN=PASS MATCHES=0`, 새 TS·JS·Python 파일 `0`이다. 이 패턴 검사를 모든 형태의 비밀 부재 증명으로 확대하지 않는다.

원문: [I/O 실패](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/blog-platform-completion/docs/evidence/l08-platform/repair-io-before.txt), [I/O 최종 자체](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/blog-platform-completion/docs/evidence/l08-platform/repair-io-final.txt), [원래 WAL 실패](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/blog-platform-completion/docs/evidence/l08-platform/wal-tail-before.txt), [raw 120](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/blog-platform-completion/docs/evidence/l08-platform/wal-raw-final.txt), [백업 거부](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/blog-platform-completion/docs/evidence/l08-platform/wal-refuse-final.txt), [앱 회귀](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/blog-platform-completion/docs/evidence/l08-platform/wal-repair-app-final.txt), [parser 실패](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/blog-platform-completion/docs/evidence/l08-platform/compiler-parse-before.txt), [parser 보완](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/blog-platform-completion/docs/evidence/l08-platform/compiler-parse-shared-after.txt).
