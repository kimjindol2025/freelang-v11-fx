# L08 3차 C04·C07·C10 구현·검증 보고

## 확인한 것

기준은 feat/l08-c05-c09의 `2828966042d56bcd7e083decdae40bd425d5177b`, 작업 가지는 `feat/l08-c04-c07-c10`이다. 구현 소스의 최종 커밋은 `a06b883d6d8a6eb17eacdb12f343b7e996e45f7c`이다. 이후 독립 clock 시험과 문서·증거를 추가했으며 런타임·컴파일러 구현은 변경하지 않았다. C04·C07·C10 외 C 번호에 착수하지 않았다.

### 소스 계약과 구현 범위

v11 비교 기준은 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4`이며 TS·JS 소스는 Git 텍스트로만 읽었다. [계약·정답 정정 이력](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c04-c07-c10/docs/L08-phase3-contract.md), [승인 기록 6장](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c04-c07-c10/docs/L08-compat-instruction-draft.md).

| 범위 | 구현·확인한 계약 |
|---|---|
| C04 | date_parse의 ISO 날짜/시각·epoch ms·오프셋·날짜만 UTC·offset 없는 시각의 로컬 TZ·잘못된 입력 예외. now_ms의 실제 호스트 ms 범위. format_date의 로컬 시간·첫 토큰 치환·음수 ms·NaN·NUL 이후 토큰. UTC·Asia/Seoul·America/New_York 및 DST gap/overlap |
| C07 | C 자체 parser/bytecode/backtracking VM. UTF-16·문자 클래스·반복/상한·lazy·캡처·선택·anchor·positive lookahead·공백 클래스·ignoreCase·global/first 치환·캡처 치환 기호·split·빈 매치. 실제 호출 59곳의 literal 34종과 동적 생성 2종을 고정하고 SQL keyword 44종·오류 코드 21종까지 검사 |
| C10 | v11 underscore 텍스트 read/write/append, 원본 byte copy, delete/exists/is_file/size, 부모 생성·디렉터리 생성/목록. fd 기준 경로 순회와 O_NOFOLLOW, copy 양쪽 경계·최종/중간/base symlink·NUL·`..` 차단, 권한 실패. PNG/JPEG/GIF의 copy·L07 raw 쓰기 후 cmp·SHA-256 |

[날짜 구현](https://github.com/kimjindol2025/freelang-v11-fx/blob/a06b883d6d8a6eb17eacdb12f343b7e996e45f7c/runtime/v11-time.h), [정규식 구현](https://github.com/kimjindol2025/freelang-v11-fx/blob/a06b883d6d8a6eb17eacdb12f343b7e996e45f7c/runtime/v11-regex.h), [파일 구현](https://github.com/kimjindol2025/freelang-v11-fx/blob/a06b883d6d8a6eb17eacdb12f343b7e996e45f7c/runtime/v11-file.h), [네이티브 연결](https://github.com/kimjindol2025/freelang-v11-fx/blob/a06b883d6d8a6eb17eacdb12f343b7e996e45f7c/runtime/v11-compat.h).

새 외부 정규식 라이브러리는 **0개**다. 기존 POSIX C ABI는 유지하고 실제 blog1의 underscore regex_* 호출은 자체 C 엔진으로 연결했다. 따라서 외부 정규식 라이브러리 승인 전 커밋은 없었다. [패턴 전수 목록·고정 SHA 링크](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c04-c07-c10/docs/L08-phase3-regex-inventory.md). app 추적 파일 22개와 실제 load한 db95 파일 5개를 조사했으며 중첩 호출도 포함했다. blog1 HEAD `013a5b339068412332ae12a7ef0c412963a3a880`, db95 HEAD `88b4aa2ca11e5355240a1d93ece8321315c669cc`와 조사 파일 내용 일치를 확인했다. 앱과 vendor는 수정하지 않았다.

텍스트 `file_read`는 UTF-8 대체 디코딩이며 다시 쓰면 원본 이미지 byte를 복구하지 못한다. 원본은 `file_copy`, L07의 kebab-case 및 checked result byte API로 처리한다. L07 read/write/append/copy 구현과 기존 C ABI를 유지했다. 새 C10 경로 보호는 underscore API에 적용하며 L07의 기존 FL_FILE_BASE·raw 정책을 다른 계약으로 바꾸지 않았다. 새로운 텍스트 상대 경로는 v11처럼 cwd 기준이다. FL_FILE_BASE 기준 디렉터리는 존재해야 하며 C10에서는 그 경로 요소도 symlink이면 거부한다. 한도는 64 MiB다.

### 커밋과 실패 기록

| 구분 | 커밋 SHA |
|---|---|
| 최초 `.fl`·정답·전수 목록 선행 고정 | `047383e3941dfe139d3c47eacf86611351c37434` |
| 최초 실패 보관·`$` 정답 정정 | `d64aaa403a7cef1ebc02cf9187f1a8bc4cbcc3a2` |
| fixture CLI 인자 선언 수정 | `8393f7b95a662972d787855e2b226e6de3839427` |
| 정규식/파일 경계 시험 추가 | `23ac3398d75d0505af312959fdef4a504d043c0c` |
| 예외 자체를 요구하도록 시험 강화 | `cc32de3a50d1708da0e39d3e11b8b53d6b9967ae` |
| DST gap/overlap 정답 추가 | `ff27cc041e068cd59d2477bdaf964ce720d1b8cb` |
| C04·C07·C10 구현 | `5d28c33d788125156475707cd81f6b18e74760de` |
| NUL 뒤 날짜 포맷 정답 선행 추가 | `9d928f82dd55d703de7239590d9c20b94710e239` |
| NUL 날짜 포맷 수정 | `a06b883d6d8a6eb17eacdb12f343b7e996e45f7c` |
| 독립 호스트 clock 시험 | `986d3050e5ff1a6bd2347e1ad7708d5a039484fc` |

정답 정정은 `$` 1건이다. m 없는 v11 RegExp에서 `regex_match "x\n" "x$"`는 false이며, 마지막 줄바꿈을 허용한다고 잘못 추론한 true를 C 엔진 구현 전에 정정했다. v11 RegExp 생성 원문과 고정 SHA의 ECMAScript Assertion 원문을 계약 문서에 연결했다. 구현 출력에 맞춘 정정은 아니다. 이후 시험은 추가·강화했으며 정답 변경 이력을 숨기지 않았다.

| 원문 증거 | 실제 결과 |
|---|---|
| [최초 수정 전](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c04-c07-c10/docs/evidence/l08-c04-c07-c10/before.txt) | `L08_P3_PASS=0 FAIL=10`, `BEFORE_EXIT=1`. 7개 빌드 실패 집계와 이미지 cmp 3개 실패이며 최종 11묶음과 같은 집계 단위가 아님 |
| [fixture 준비 오류](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c04-c07-c10/docs/evidence/l08-c04-c07-c10/before-fixture-setup.txt) | `L08_P3_PASS=6 FAIL=6`, `SELF_EXIT=1`. CLI 인자 선언 누락을 helper에서 수정했고 의미상 정답은 유지 |
| [정규식 진단 오류](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c04-c07-c10/docs/evidence/l08-c04-c07-c10/before-regex-diagnostics.txt) | `L08_P3_PASS=10 FAIL=1`, `SELF_EXIT=1`. 최초 unsupported group 오류를 후속 오류가 덮어쓰는 문제를 고침 |
| [NUL 포맷 수정 전](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c04-c07-c10/docs/evidence/l08-c04-c07-c10/before-date-nul.txt) | `expected=01 actual=MM`, `DATE_NUL_BEFORE_EXIT=1`. 새 시험을 먼저 커밋한 뒤 길이 기준 토큰 검색으로 수정 |

NUL 경계 시험을 추가하면서 [중간 전체 실행](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c04-c07-c10/docs/evidence/l08-c04-c07-c10/interrupted-run.txt)은 자기 시험 드라이버만 중단했다(`FULL_EXIT=143`). 다른 사용자의 과정을 종료하지 않았다. 이후 최종 소스에서 자체→기존 회귀→native bootstrap을 처음부터 실행했다.

### 최종 요청 시험 결과

[전체 드라이버](https://github.com/kimjindol2025/freelang-v11-fx/blob/a06b883d6d8a6eb17eacdb12f343b7e996e45f7c/scripts/verify-l08-phase3-all.sh)를 실행했다. 실제 경로는 보고에서 가렸다.

```bash
CPATH=<가림> LIBRARY_PATH=<가림> L08_P3_OUT=<가림> bash <가림>
```

자체 시험은 strace 아래에서 실행하고 기존 2차 드라이버를 그대로 호출했다. 그 안의 1차·L06·L07·부트스트랩 드라이버와 ptrace를 중첩하지 않았다. 기존 시험·정답·verify-native-bootstrap.sh는 변경하지 않았다.

| 단계 | 원문 결과 | 종료 코드 |
|---|---|---|
| C04·C07·C10 자체 | `L08_P3_PASS=11 FAIL=0`, 이미지 cmp 3종·외부 sentinel 보존 PASS | `PHASE3_SELF_EXIT=0` |
| L08 2차 자체 | `L08_P2_PASS=11 FAIL=0` | `PHASE2_SELF_EXIT=0` |
| L08 1차 자체 | `L08_PASS=17 FAIL=0` | `self_EXIT=0` |
| L08 실제 HTTP | 7건 모두 `200`, `L08_HTTP_PASS=7` | `http_EXIT=0` |
| L06 HTTP·TLS | `L06_HTTP_PASS=110 FAIL=0`, `L06_TLS_PASS=110 FAIL=0` | `HTTP_EXIT=0` |
| L06 시간 제한 | `L06_TIMEOUT_PASS=2 FAIL=0` | 같은 HTTP 단계 `0` |
| L07 | `L07_SELF_PASS=221`, `L07_SELF_FAIL=0`, 입력·외부 파일 해시 보존 | `REGRESSIONS_EXIT=0` |
| 기존 C 런타임 | PASS 53건, `All tests PASS` | 같은 회귀 단계 `0` |
| 기존 JIT·GC | `결과: 10 PASS / 0 FAIL / 10 총` | 같은 회귀 단계 `0` |
| native bootstrap | `STAGE2_EQ_STAGE3=PASS`, `RESIDUAL_PROCESSES=0` | `BOOTSTRAP_EXIT=0` |
| 전체 요청 드라이버 | `L08_C04_C07_C10_GATES=PASS` | `FULL_EXIT=0` |

묶음/프로그램/HTTP assertion의 집계 단위가 다르므로 위 숫자를 단순 합산한 전체 시험 수를 제시하지 않는다. TLS는 자체서명 인증서를 사용하는 로컬 시험이며 운영 TLS 확인이 아니다. 시험은 빈 포트 0을 할당받고 자기 시험 과정만 정리한다.

[전체 원문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c04-c07-c10/docs/evidence/l08-c04-c07-c10/full-run.txt), [자체 시험·이미지 SHA](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c04-c07-c10/docs/evidence/l08-c04-c07-c10/after-self.txt), [HTTP/TLS](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c04-c07-c10/docs/evidence/l08-c04-c07-c10/after-l06-http.txt), [L07](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c04-c07-c10/docs/evidence/l08-c04-c07-c10/after-l07.txt), [기존 회귀](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c04-c07-c10/docs/evidence/l08-c04-c07-c10/after-regressions.txt), [부트스트랩](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c04-c07-c10/docs/evidence/l08-c04-c07-c10/after-bootstrap.txt).

```text
STAGE0_SHA=f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c
STAGE1_SHA=f483feebaf74ea862cd9e5b3bd102a2445db763e17421126bdd140663c9b5e0f
STAGE2_SHA=0d776b758fb638e3aa96551a2f7bc1dcd3739e8ae77bfa9b674a6959b9064967
STAGE3_SHA=0d776b758fb638e3aa96551a2f7bc1dcd3739e8ae77bfa9b674a6959b9064967
BOOTSTRAP_EXIT=0
```

기존 검증기의 `STAGE0_EQ_CANONICAL=DIFFERENT`도 원문 그대로 보관했다. 고정 stage0와 검증기를 바꾸지 않고 고정점을 통과했다.

추가 독립 clock 시험은 세 시간대 모두 `CLOCK_RANGE=PASS`, `CLOCK_EXIT=0`이다. [호스트 하한·실제 FreeLang ms·호스트 상한 원문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c04-c07-c10/docs/evidence/l08-c04-c07-c10/clock.txt). 실제 현재 시각을 정답 상수로 만들지 않았다.

strace 원본 및 경로를 가린 gzip을 각각 감사했다. [감사 원문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c04-c07-c10/docs/evidence/l08-c04-c07-c10/trace-audit.txt), [원본 SHA-256](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c04-c07-c10/docs/evidence/l08-c04-c07-c10/trace-sha256.md), [추적 원문 gzip](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c04-c07-c10/docs/evidence/l08-c04-c07-c10/execve-redacted.txt.gz). 실행 대상 basename을 보존하며 JS 파일명을 언급한 검색 인자는 실행으로 세지 않는다.

```text
NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0
```

문서·증거 커밋 후 같은 작업 가지를 Forgejo → GitHub 순서로 일반 푸시하고, 두 ref를 조회한 HEAD는 최종 응답에 보고한다. 보고서 자신의 SHA를 자기 참조로 꾸며 넣지 않는다.

[고정 정답 SHA-256](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c04-c07-c10/docs/evidence/l08-c04-c07-c10/answers-sha256.md), [최종 diff·정답·민감 정보 검사](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c04-c07-c10/docs/evidence/l08-c04-c07-c10/final-checks.txt)를 보관했다. 추가 clock 시험은 별도로 실행했으며, 기본 런타임·컴파일러·검증 드라이버와 기존 회귀 시험은 검증 시작 후 바꾸지 않았다. 민감 정보 검사는 변경 내용 검토와 주요 키/토큰 형식 검색이며 모든 비밀을 자동 판별한다는 보증은 아니다.

## 미확인·미실행·제한

- **추가 ASan/UBSan 검사는 미실행이다.** GCC `14.3.1`의 libasan/libubsan 런타임이 없어 링크가 종료 `1`로 실패했다. sanitizer 프로그램은 실행되지 않았으며 통과로 세지 않는다. [링크 원문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c04-c07-c10/docs/evidence/l08-c04-c07-c10/sanitizer-link.txt), [의존성 확인](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c04-c07-c10/docs/evidence/l08-c04-c07-c10/sanitizer-dependencies.txt). 위 `FULL_EXIT=0`은 요청된 자체·회귀·bootstrap 드라이버의 결과다. 추가 검사까지 전부 완료했다고 보고하지 않는다.
- 정규식은 실제 blog1 패턴부터 지원하는 부분집합이다. backreference·lookbehind·class 내 complement union 등 범위 밖 구문은 명시 오류이며 모든 ECMAScript RegExp를 구현했다고 주장하지 않는다. 자원 상한은 계약 문서에 명시했다. 큰 입력의 성능·부하 및 범위 밖 패턴 호환은 미검증이다.
- 날짜는 ISO 계약을 구현했다. RFC/지역별 비ISO Date.parse 입력 전체, 다른 OS/다른 TZ 데이터 버전·모든 역사적 시간대는 미검증이다. format_date는 숫자 timestamp와 문자열 template 계약이다.
- 파일은 일반 파일·신뢰할 수 있는 기존 base 디렉터리 계약이다. 64 MiB 초과·streaming·fd 내구성(C11)·OS별 오류 메시지 전체 동일성은 이번 범위가 아니다. 디렉터리 목록 순서는 보장하지 않는다. 기존 raw/legacy C ABI 정책과 새 underscore 보호 정책을 구분한다.
- blog1 전체 컴파일·실행·운영 WAL/이미지/설정/계정 호환·화면·운영 TLS는 검증하지 않았다. L08 전체 완료나 운영 이전 가능 판정은 하지 않는다.
- 서버 23·25 접속·작업, 운영 데이터 수정, 합치기·배포·강제 푸시는 하지 않았다.
