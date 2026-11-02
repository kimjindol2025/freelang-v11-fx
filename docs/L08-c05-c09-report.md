# L08 2차 C05·C06·C08·C09 구현 보고

## 확인한 것

작업 가지는 `feat/l08-c05-c09`, 시작점은 `feat/l08-c01-c03`의 `ec9200cc25c6c918b79b5c626bccfed6f4699e16`이다. 6장에 사용자 결정을 먼저 기록하고 실패 재현 `.fl`·정답을 커밋한 뒤 구현했다. 승인 범위 외 C 항목은 착수하지 않았다.

### 기준과 구현

v11 기준은 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4`의 일반 인터프리터 경로다. 같은 저장소 안의 opt-in VM·fast helper·다른 fallback 경로를 섞지 않았다. v11 TS·JS는 Git 소스를 텍스트로 읽었으며 실행하지 않았다. [정답 근거](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c05-c09/tests/l08-phase2/contract.md).

| 범위 | 변경과 확인 대상 |
|---|---|
| C05 | join의 두 인자 순서, 끝 생략·음수 slice, find/starts/ends/lines, literal 치환, split 조각 수, URI 한글·이모지·NUL·잘못된 percent/UTF-8 |
| C06 | 기존 문자열 연산은 UTF-16 코드 단위. 한글·이모지·단독 surrogate·잘못된 UTF-8·NUL, Unicode 공백·대소문자·final sigma, parseFloat 계약과 HTML `&#x27;` 처리 |
| C08 | 배열 append/conj/concat·한 단계 flatten·구조 값 unique·안정 정렬·원본 보존·map/filter/reduce. unique의 비유한 수와 -0 정규화, 정렬의 저장된 null/누락 키 구분 |
| C09 | 지수 리터럴·IEEE 754 수·나눗셈·숫자 타입 오류·비교·구조 동등성·출력·인자 선행 평가·and/or 단락 평가·loop/recur. 유한 range 시작/끝/증분, take/drop 계약 |
| 1차 잔여 | 숫자 map 키·소수 shortest 표기·고정/지수 표기 경계·음의 0·최소 비정규 수·최대 유한 수 |

[네이티브 호환 구현](https://github.com/kimjindol2025/freelang-v11-fx/blob/87763c7a3bdb3095406599a3d16c5799fbbad2e8/runtime/v11-compat.h), [컴파일러 연결](https://github.com/kimjindol2025/freelang-v11-fx/blob/87763c7a3bdb3095406599a3d16c5799fbbad2e8/self/cgc-main.fl). 텍스트와 raw 바이트의 내부 표시를 구분하며 L07 API의 파일 바이트를 변환하지 않는다. seed와 기존 C 호출자의 ABI를 유지하고 FreeLang 호출을 호환 구현에 연결한다. 내장 함수 값은 요청 arena 문자열 대신 정수 opcode만 캡처한다. 컴파일러의 cond 코드 생성 순서를 명시해 생성 중 부작용의 C 인자 평가 순서 의존을 없앴다. str-replace-in의 replaceAll 치환 기호는 C 문자열 처리로 구현했으며 C07 정규식 엔진에는 착수하지 않았다. 배열 append의 문자열 spread는 v11처럼 코드 포인트로 나누며, 문자열 인덱스의 UTF-16 단위와 구분한다.

숫자 표기는 고정 SHA의 Ryu C 구현과 라이선스를 포함했다. Unicode 대소문자 표는 Unicode 15.0.0 텍스트 자료에서 Bash/awk로 생성했다. Perl 자료 파일은 텍스트로만 읽는다. 새 TS·JS·Python 파일과 fl-build.sh 사용은 없다. [Ryu 출처](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c05-c09/runtime/ryu/README.md), [Unicode 생성기](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c05-c09/scripts/generate-unicode15.sh).

### 커밋 순서

| 구분 | 커밋 SHA |
|---|---|
| 결정 기록·첫 8개 시험 묶음과 정답 | `190e46d6a2ccf62c4e0a69f5b14b2d90a44dfd48` |
| 수정 전 실패 보관·음의 0 시험 추가 | `6f1e0d648e926ecb6e4eb8b491f64f8d4f60f46f` |
| 경계 시험·정답 추가 | `db6bb955ce825def7f46e90097820192e0378ae7` |
| 산술 오류·배열 계약 시험·정답 추가 | `aa09224bf760e9245e01d836508fc3ab87cb2978` |
| 네이티브 구현 | `5f11c21aa5281facbd05d7f2dabd97ca24d4fb7c` |
| num-to-str 정답 정정·추가 소스 경계 시험 | `49cf74629a025867c22f2313e9dc42ab4d6d6846` |
| null/누락 키 비교 정답의 추론 정정 | `e5806691cb4b2a084a8b2e87d4e372a52ecafea2` |
| substring·fallback slice 정답 추가 | `b0682cd20a9f9c6f891e688931448da9b751ed31` |
| 최종 소스 계약 수정 | `87763c7a3bdb3095406599a3d16c5799fbbad2e8` |

추가 시험은 해당 수정 전에 정답을 고정했다. 최초 8개와 최종 11개 시험 집합의 수가 다르다. 정답 정정은 2건이다. num-to-str nil을 보조 평가기 동작과 혼동해 빈 문자열로 잘못 고정했으며 일반 평가기 원문 String(args[0])에 따라 `"null"`로 정정했다. 서로 다른 nil 키 map 비교를 true로 잘못 추론했으나 flDeepEq의 null/undefined strict 비교와 다음 null 검사에 따라 false로 정정했다. 후자는 구현이 원래 false였으며 코드를 true로 바꾸지 않았다. 정답을 처음부터 전부 불변으로 유지했다고 보고하지 않는다. 정정 근거와 이력은 정답 근거 문서에 남겼다. 경계 시험까지의 기존 9개 manifest는 유지되며, 최종 11개 정답 SHA-256 표를 별도 보관한다.

num-to-str 정정 뒤에는 [기존 실행물과 정정한 소스 정답의 차이](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c05-c09/docs/evidence/l08-c05-c09/before-oracle-correction.txt)를 별도로 남겼다. 프로그램 종료 `0`, diff 종료 `1`이며, 잘못 구현한 빈 문자열 `[]`를 원문 기준 `[null]`로 고친 뒤 최종 시험을 수행했다.

### 수정 전 실패

최초 실행은 `L08_P2_PASS=0 FAIL=9`, 종료 `1`이다. 당시 8개 묶음과 raw 파일 비교 실패를 합친 실패 수이며 최종 묶음 수와 혼동하지 않는다. 컴파일 인자 수 오류·없는 문자열 함수·잘못된 출력·문자열 단위 차이를 [수정 전 원문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c05-c09/docs/evidence/l08-c05-c09/before.txt)에 보관했다.

추가 경계 시험은 프로그램 종료 `0`, 정답 diff 종료 `1`이었다. 산술·배열 계약 시험은 프로그램 종료 `1`, 정답 diff 종료 `1`이었다. [경계 원문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c05-c09/docs/evidence/l08-c05-c09/before-edges.txt), [산술·배열 원문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c05-c09/docs/evidence/l08-c05-c09/before-contracts.txt). 두 보관 실행물의 재실행도 strace로 확인했다. 원문에서 로컬 절대 경로만 `<가림>` 처리하며, 수·해시·CR·빈 줄은 유지한다.

경계 원문에는 URI 처리 오류로 출력한 실제 NUL이 있어 Git에서 binary로 분류된다. 원본을 보존하고 [바이트 덤프](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c05-c09/docs/evidence/l08-c05-c09/before-edges-bytes.txt)를 함께 보관했다. diff에 찍힌 로컬 파일 이름도 가렸다.

[소스 경계 수정 전 실행물](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c05-c09/docs/evidence/l08-c05-c09/before-source-edge.txt)은 처음 12개 항목의 보관 실행물이며, 최신 16개 정답과의 diff는 이후 추가한 4개 출력의 누락도 포함한다. 프로그램 종료 `0`, diff 종료 `1`이다. [substring 4개 차이](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c05-c09/docs/evidence/l08-c05-c09/before-substring.txt)는 나머지 12개 수정 후 따로 재현했으며 종료 `1`이다. 개발 중 전체 드라이버는 추가 소스 수정 전에 자기 시험 작업만 중단했다(`FULL_EXIT=143`). 다른 사용자의 과정은 종료하지 않았다. 이 중간 실행을 최종 전체 통과로 세지 않는다.

### 최종 시험·strace

최종 구현 SHA `87763c7a3bdb3095406599a3d16c5799fbbad2e8`에서 [전체 시험 드라이버](https://github.com/kimjindol2025/freelang-v11-fx/blob/87763c7a3bdb3095406599a3d16c5799fbbad2e8/scripts/verify-l08-phase2-all.sh)를 실행했다. 실행 형태는 다음과 같다. `<가림>`은 실제 실행 때 사용한 비공개 로컬 경로를 가린 표시이며 실제 명령에 이 문자열을 넣어 실행한 것이 아니다.

```bash
CPATH=<가림> LIBRARY_PATH=<가림> L08_P2_OUT=<가림> bash <가림>
```

2차 자체 시험은 strace 아래에서 실행하고, 기존 L08 1차 드라이버를 그대로 호출해 자체·HTTP·L06·L07·기존 회귀·부트스트랩을 순서대로 검사한다. 기존 드라이버 내부의 strace와 중첩 ptrace를 만들지 않는다. 포트는 loopback의 0으로 원자적 할당하고 시험이 만든 과정만 종료한다. TLS는 로컬 자체서명 인증서 시험이며 운영 인증서 확인이 아니다.

| 시험 | 확인 결과 | 종료 코드 |
|---|---|---|
| 2차 `.fl` 자체 시험 | `L08_P2_PASS=11 FAIL=0`, `RAW_L07_BYTES=PASS` | `PHASE2_SELF_EXIT=0` |
| L08 1차 자체 시험 | `L08_PASS=17 FAIL=0` | `self_EXIT=0` |
| L08 실제 HTTP | 7건 모두 `200`, `L08_HTTP_PASS=7` | `http_EXIT=0` |
| L06 폼 본문 재현 | `HTTP=200`, `FORM_MAP=PASS` | `REPRO_EXIT=0` |
| L06 HTTP/TLS | `L06_HTTP_PASS=110 FAIL=0`, `L06_TLS_PASS=110 FAIL=0` | `HTTP_EXIT=0` |
| L06 시간 제한 | `L06_TIMEOUT_PASS=2 FAIL=0` | 같은 HTTP 단계 `0` |
| L07 자체 시험 | `L07_SELF_PASS=221`, `L07_SELF_FAIL=0` | `REGRESSIONS_EXIT=0` |
| 기존 C 런타임 | `PASS` 53건, `All tests PASS` | 같은 회귀 단계 `0` |
| 기존 JIT·GC | `결과: 10 PASS / 0 FAIL / 10 총` | 같은 회귀 단계 `0` |
| 회귀 후 native bootstrap | `STAGE2_EQ_STAGE3=PASS`, `RESIDUAL_PROCESSES=0` | `BOOTSTRAP_EXIT=0` |
| 전체 드라이버 | `L08_C05_C06_C08_C09_GATES=PASS` | `FULL_EXIT=0` |

승인 범위 C05·C06·C08·C09의 자체 시험·회귀·native bootstrap 검증을 통과했다. 위 묶음별 집계는 서로 단위가 달라 단순 합산한 “전체 시험 수”로 꾸미지 않는다. 같은 최종 소스로 수행한 [부트스트랩 사전 진단 원문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c05-c09/docs/evidence/l08-c05-c09/bootstrap-diagnostic.txt)도 `STAGE2_EQ_STAGE3=PASS`, `BOOTSTRAP_DIAGNOSTIC_EXIT=0`이었다. 최종 표는 회귀 뒤에 실행한 검증 결과다.

[전체 원문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c05-c09/docs/evidence/l08-c05-c09/full-run.txt), [2차 자체 시험](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c05-c09/docs/evidence/l08-c05-c09/after-self.txt), [1차 자체 시험](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c05-c09/docs/evidence/l08-c05-c09/after-phase1-self.txt), [HTTP 7건](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c05-c09/docs/evidence/l08-c05-c09/after-http.txt), [L06 HTTP/TLS](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c05-c09/docs/evidence/l08-c05-c09/after-l06-http.txt), [L07](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c05-c09/docs/evidence/l08-c05-c09/after-l07.txt), [C 런타임](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c05-c09/docs/evidence/l08-c05-c09/after-runtime.txt), [기존 회귀](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c05-c09/docs/evidence/l08-c05-c09/after-regressions.txt), [최종 부트스트랩](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c05-c09/docs/evidence/l08-c05-c09/after-bootstrap.txt).

최종 부트스트랩 원문:

```text
STAGE1_SHA=f483feebaf74ea862cd9e5b3bd102a2445db763e17421126bdd140663c9b5e0f
STAGE2_SHA=0d776b758fb638e3aa96551a2f7bc1dcd3739e8ae77bfa9b674a6959b9064967
STAGE3_SHA=0d776b758fb638e3aa96551a2f7bc1dcd3739e8ae77bfa9b674a6959b9064967
BOOTSTRAP_EXIT=0
```

strace 원본과 경로를 가린 gzip을 각각 검사했다. 실행물 basename을 보존하며 execve의 실행 대상만 검사하고, JS 파일명을 언급한 검색 인자를 실행으로 세지 않는다. [감사 원문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c05-c09/docs/evidence/l08-c05-c09/trace-audit.txt), [원본 SHA-256·보관 방법](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c05-c09/docs/evidence/l08-c05-c09/trace-sha256.md), [경로를 가린 추적 원문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c05-c09/docs/evidence/l08-c05-c09/execve-redacted.txt.gz), [정답 SHA-256](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c05-c09/docs/evidence/l08-c05-c09/answers-sha256.md).

```text
NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0
```

기존 검증기의 `STAGE0_EQ_CANONICAL=DIFFERENT` 표기는 그대로 보관한다. 기존에 고정한 stage0 SHA는 `f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c`이며 이를 변경하지 않았다. 기존 native bootstrap 검증기 SHA-256은 `2899b57106ddbfceb611a54fc3af34fdf998961effe6d67e5b6289bcc1b769cb`이다.

최종 보고·증거를 커밋한 뒤 같은 작업 가지를 Forgejo → GitHub 순서로 일반 푸시하고, 각 원격 ref를 조회해 최종 응답에 두 HEAD SHA를 원문 보고한다. 보고서 자신의 커밋 SHA를 보고서 안에 자기 참조로 꾸며 넣지 않는다.

[최종 소스·정답·민감 정보 diff 검사 결과](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c05-c09/docs/evidence/l08-c05-c09/final-checks.txt)에 `TESTED_SOURCE_AND_TEST_DIFF=0`, `PREVIOUS_TESTS_AND_BOOTSTRAP_DIFF=0`, `TS_JS_PYTHON_CHANGED_FILES=0`, `SENSITIVE_DIFF_SCAN=PASS`를 보관했다. 민감 정보 검사는 추가 diff와 gzip 해제 내용의 개인 키·주요 토큰 형식 검색 및 변경 내용 검토이며 모든 비밀 형식을 자동 판별한다는 보증은 아니다.

## 미확인

- L08 전체 완료·blog1 이전 가능·운영 호환을 판정하지 않는다. 날짜·정규식·파일 text alias 등 다른 C 항목은 이번에 착수하지 않았다.
- blog1 앱 컴파일·실행, 운영 WAL·이미지·설정·계정 호환, 브라우저 화면은 검증하지 않았다.
- v11의 모든 host 객체·순환 객체·이름 있는 함수 출력·무한 lazy sequence·모든 locale 동작의 완전한 호환을 주장하지 않는다. Unicode 15.0.0보다 새 문자 데이터와 운영 v11의 실제 Unicode 버전도 미확인이다.
- 정렬은 안정 삽입 정렬이다. 큰 배열 성능과 비교 규칙이 일관되지 않은 누락 키 혼합 입력에서 모든 JS 엔진과 동일한 순서는 검증하지 않았다.
- 대규모 문자열·배열과 전체 컴파일 성능을 최적화하거나 부하 시험하지 않았다. GC 파일 변경은 문자열 복사 때 내부 텍스트 표식을 유지하는 1줄이며 C20 수명·GC 개편에 착수한 것이 아니다.
- 서버 23·25 접속·작업, 운영 자료 변경, 합치기·배포·강제 푸시는 하지 않았다.
