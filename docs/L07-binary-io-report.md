# L07 바이너리 파일 입출력 구현 보고

## 확인한 것

저장소는 `kimjindol2025/freelang-v11-fx`, 작업 가지는 `feat/l07-binary-io`다. main/master에 합치거나 배포하지 않았다. 서버 23·25에 접속하거나 작업하지 않았다. 실데이터·백업 원본을 입출력 대상으로 사용하지 않았다.

| 순서 | 전체 SHA | 내용 |
|---|---|---|
| 시작 기준 | `1d217f53a8aade8a288c0dcad0a1ba23b610f102` | PRD 조사 기준과 같은 fx HEAD에서 새 가지 시작 |
| 실패 시험·정답 선행 커밋 | `38bd1207abdb540eac0b1f84b4afbde4c4212130` | 재현 `.fl`·정답·네이티브 빌드 도구·API 계약을 먼저 커밋 |
| 검증한 구현 커밋 | `2ce2ab89607d6f5cf0bfe1d865d981e1309ad76c` | 명시적 길이·I/O 오류·결과 API·경계·자체 시험 구현 |

시험은 구현 커밋과 같은 소스에서 실행했고, 이후 변경은 이 보고서와 증거 파일뿐이다. 원본 `verify-native-bootstrap.sh`와 선행 재현 시험·정답은 수정하지 않았다. 고정 정답 파일 SHA-256은 `a657951fd81482f296df460d49f3a4cbd203d458e5f0aa1950bff5a9d8a69134`다.

## 원인과 변경

원인은 파일 데이터의 실제 `FLString.len` 대신 `strlen`을 사용한 write·append였다. read·write·append·copy를 공유 파일 처리에 연결해 NUL·비UTF-8 바이트를 보존하고, 짧은 처리·read/write/flush/close 오류를 확인한다. 기존 반환 형식은 유지한다. legacy write의 nil로는 실패를 구분할 수 없으므로 `file-read-result`, `file-write-result`, `file-append-result`, `file-copy-result`를 추가해 `ok`, `bytes`, `error`, `errno`, `data`를 제공한다.

64 MiB 상한·자료형·빈 경로·긴 경로·내장 NUL·일반 파일 여부를 검사한다. 같은 inode로 복사하면 자르기 전에 거부한다. `FL_FILE_BASE`가 설정되면 해당 read/write/append/copy에 디렉터리 fd·no-follow 루트 제한을 적용한다. 경로 이탈·심볼릭 링크와 실패 뒤 fd 누수를 자체 시험으로 확인했다. 설정하지 않은 프로그램은 기존 OS 권한 범위에서 동작한다. [API 계약](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l07-binary-io/docs/L07-contract.md)에 반환·상한·제약을 기록했다.

## 수정 전 실패 원문

`41 00 42`의 입력은 3바이트다. 기대 write는 3바이트, 기존 `50` 뒤 append는 4바이트다. [수정 전 출력](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l07-binary-io/docs/evidence/l07/before-repro.txt):

```text
L07_REPRO_INPUT_BYTES=3
L07_REPRO_WRITE_BYTES=1
L07_REPRO_APPEND_BYTES=2
L07_REPRO_WRITE_MATCH=FAIL
L07_REPRO_APPEND_MATCH=FAIL
L07_BEFORE_EXIT=1
```

이 출력과 정답을 확인한 뒤 선행 커밋을 만들었고 그 후 런타임을 수정했다. 초기 진단에서 일반 문자열 `length`도 NUL 뒤를 세지 않는 것을 확인했으므로, 커밋한 재현 시험은 텍스트 길이를 바이트 증거로 사용하지 않고 독립적인 입력 파일 크기를 사용했다. 일반 텍스트 함수의 의미는 변경하지 않았다.

## 수정 후 출력 원문

같은 `.fl`과 같은 정답 파일로 실행한 [수정 후 재현 출력](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l07-binary-io/docs/evidence/l07/after-repro.txt):

```text
L07_REPRO_INPUT_BYTES=3
L07_REPRO_WRITE_BYTES=3
L07_REPRO_APPEND_BYTES=4
L07_REPRO_WRITE_MATCH=PASS
L07_REPRO_APPEND_MATCH=PASS
L07_REPRO_AFTER_EXIT=0
```

[FreeLang 자체 시험 전체 출력](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l07-binary-io/docs/evidence/l07/after-self.txt)과 [기존 런타임 회귀 출력](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l07-binary-io/docs/evidence/l07/runtime-regression.txt):

```text
L07_SELF_PASS=221
L07_SELF_FAIL=0
L07_INPUT_AND_OUTSIDE_HASHES=UNCHANGED
L07_RUNTIME_REGRESSION=PASS
L07_FINAL_EXIT=0
```

기존 런타임 회귀는 `PASS` 53건·`FAIL` 0건이다. 전체 L07 실행의 종료 코드는 `0`이다. 회귀의 기존 고정 임시 파일 이름만 격리된 fixture 위치로 치환했고 assertion·기대값은 바꾸지 않았다. 빈 파일·텍스트·NUL 세 위치·모든 바이트 값·비UTF-8·8191/8192/8193·다중 블록·64 MiB 경계, public/legacy/별칭, 경로·권한·심볼릭 링크·같은 inode, 짧은 I/O·입출력·할당·stat·stream·truncate·flush·close 실패를 확인했다. 실패 주입은 시험 바이너리의 C linker wrapper에만 있고 운영 런타임에는 시험 스위치를 넣지 않았다.

[입력 전 해시](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l07-binary-io/docs/evidence/l07/input-before.sha256)와 [입력 후 해시](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l07-binary-io/docs/evidence/l07/input-after.sha256), 루트 밖 표본 해시도 일치했다. 이 값들은 자체 시험 fixture의 해시이며 운영 데이터 해시가 아니다.

## 네이티브 부트스트랩

자체 시험 이후 원본 `verify-native-bootstrap.sh`를 실행했다. 원본 스크립트가 내부 strace를 사용하고 종료 시 로그를 정리하므로 [L07 보존 드라이버](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l07-binary-io/scripts/verify-l07-bootstrap.sh)는 실제 strace를 그대로 실행한 뒤 로그만 복사한다. gcc는 별도로 추적해 중첩 ptrace를 피했다. 원본 gate의 판단·정답·SHA를 수정하지 않았다.

[부트스트랩 출력 원문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l07-binary-io/docs/evidence/l07/native-bootstrap.txt)의 핵심 결과:

```text
STAGE0_C_BUILD=PASS
STAGE0_TO_STAGE1=PASS
STAGE0_NATIVE_EXEC=PASS
STAGE1_C_BUILD=PASS
STAGE1_TO_STAGE2=PASS
STAGE2_C_BUILD=PASS
STAGE2_TO_STAGE3=PASS
STAGE2_EQ_STAGE3=PASS
ARRAY_LOOP_GENERATE=PASS
ARRAY_LOOP_BUILD=PASS
ARRAY_LOOP=PASS
NODE_INVOCATIONS=0
NPM_INVOCATIONS=0
TS_JS_INVOCATIONS=0
CGC_BIN_INVOCATIONS=0
AFJ_FILE_ACCESSES=0
STAGE0_SHA=f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c
STAGE1_SHA=696b7faaafdee7d0d241767399deb154c9462756158e572c0c2f84d8223590ce
STAGE2_SHA=696b7faaafdee7d0d241767399deb154c9462756158e572c0c2f84d8223590ce
STAGE3_SHA=696b7faaafdee7d0d241767399deb154c9462756158e572c0c2f84d8223590ce
RESIDUAL_PROCESSES=0
CLEAN_CHECKOUT=PASS
L07_BOOTSTRAP_TRACE_ARCHIVE=RETAINED
L07_BOOTSTRAP_EXIT=0
```

원문에 있는 `STAGE0_EQ_CANONICAL=DIFFERENT`는 보존된 seed와 과거 canonical 산출물의 차이를 표시하는 기존 정보다. seed SHA를 바꾸지 않았으며 현재 컴파일러의 stage2·stage3 동일성은 위 SHA로 통과했다. `CLEAN_CHECKOUT`도 원본 스크립트의 출력 이름이며 시험 당시 작업 트리에 변경이 없었다는 별도 주장으로 사용하지 않는다.

## 명령·프로세스 추적

아래는 실제 사용한 명령의 로컬 위치만 `<가림>`으로 치환한 것이다. 재현 전·후에는 서로 다른 자체 시험 출력 디렉터리를 사용했다.

```text
CPATH=<가림> LIBRARY_PATH=<가림> L07_OUT=<가림> strace -f -e trace=execve -o <가림> bash scripts/verify-l07-repro.sh
CPATH=<가림> LIBRARY_PATH=<가림> L07_OUT=<가림> strace -f -e trace=execve -o <가림> bash scripts/verify-l07-binary-io.sh
CPATH=<가림> LIBRARY_PATH=<가림> L07_OUT=<가림> bash scripts/verify-l07-bootstrap.sh
bash scripts/audit-l07-execve.sh <가림>
```

[추적 집계 원문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l07-binary-io/docs/evidence/l07/trace-audit.txt)에서 수정 전·후 재현, 최종 자체 시험 전체 빌드·실행, 부트스트랩 gcc·네이티브 실행은 각각 다음과 같다.

```text
NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0
```

프로그램 실행 경로를 검사해 단순히 grep 인자에 `bootstrap.js`라는 문자열이 있는 것을 JS 실행으로 세지 않는다. 부트스트랩 gcc·네이티브 로그는 9개다. [공개한 추적 로그](https://github.com/kimjindol2025/freelang-v11-fx/tree/feat/l07-binary-io/docs/evidence/l07/traces)는 절대 경로만 `<가림>` 처리했고 실행 프로그램 이름·결과는 남겼다. [원본 로그 SHA-256 목록](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l07-binary-io/docs/evidence/l07/trace-original-sha256.txt)은 가리기 전 원본의 해시이며 공개한 사본의 해시라고 주장하지 않는다. 가린 사본에도 같은 실행 집계를 적용해 0을 확인했다.

## 준비 실패·재시도 기록

첫 준비 빌드는 `curl/curl.h: No such file or directory`로 시험 전에 종료 코드 `1`이었다. 설치된 libcurl은 `8.9.1`이며 개발 헤더만 없었다. 같은 버전의 [공식 source archive](https://curl.se/download/curl-8.9.1.tar.xz)에서 헤더를 시험용 위치에 풀고 기존 시스템 라이브러리를 연결했다. archive SHA-256은 `f292f6cc051d5bbabf725ef85d432dfeacc8711dd717ea97612ae590643801e5`다. sudo·패키지 설치·시스템 설정 변경은 하지 않았다.

자체 시험 첫 빌드의 강제 include가 feature macro보다 먼저 적용돼 `strptime` 선언이 보이지 않아 종료 코드 `1`이었다. 시험 빌드에 `_GNU_SOURCE`를 명시해 해결했으며 날짜 기능이나 시험 정답을 바꾸지 않았다. 이후 218건 통과 뒤 경로 경계·짧은 읽기 내용·fd 누수 검사를 보강해 최종 221건을 실행했다.

부트스트랩 첫 시도는 바깥 strace와 내부 strace를 중첩한 실행에서 첫 네이티브 실행 전에 종료 코드 `1`이었다. 이 결과를 성공으로 계산하지 않았다. 원본 gate와 런타임 소스를 바꾸지 않고 중복 추적을 피하는 로그 보존 방식으로 다시 실행해 최종 종료 코드 `0`을 확인했다.

## 화면 증거·미확인

[실제 터미널 화면 텍스트 캡처](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l07-binary-io/docs/evidence/l07/terminal-screen.json)는 Orca의 `terminal read --screen` 결과이며 `source: screen`을 확인했다. 이미 저장한 시험 로그의 요약을 열람한 화면으로, 캡처 때 시험을 다시 실행했다고 주장하지 않는다. Linux 화면 제공자는 이미지 screenshot을 지원하지 않아 PNG는 제출하지 못했다. 텍스트 화면 캡처를 이미지라고 표시하지 않는다. 캡처용으로 만든 터미널은 종료했다.

L07 자체 시험·기존 런타임 회귀·네이티브 부트스트랩의 필수 실행을 생략한 항목은 없다. 블로그 통합 시험, L06 업로드, L01 생성 JS, 운영 이전·배포는 이번 범위가 아니므로 실행하지 않았다. J안은 [블로그 PRD 15장](https://github.com/kimjindol2025/blog1/blob/feat/wordpress-blog-core/docs/PRD-blog-core.md)에 사용자 결정으로 기록했으며 구현은 L01 차례다.

이번 결과는 Linux x86_64·GCC `14.3.1` 환경의 결과다. 다른 OS·아키텍처, 원자적 파일 교체·fsync·WAL 거래, 일반 텍스트 함수의 바이너리 처리까지 검증했다고 주장하지 않는다. `FL_FILE_BASE`의 보호 범위도 해당 4종 파일 I/O이며 전체 언어의 샌드박스가 아니다.
