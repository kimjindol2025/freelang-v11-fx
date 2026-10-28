# L08 1차 구현·검증 보고 — C01·C02·C03

상태: 승인 범위의 구현·계약 시험 통과. 전체 v11 호환·blog1 이전 판정은 아래 미확인 항목과 구분한다.

## 확인한 것

### 기준·범위·결정

기준은 `2207136944ba925ccb15076b0d2abe30a0ff6934`이며, 그 위 `feat/l08-c01-c03`에서 작업했다. [지시서 6장](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/L08-compat-instruction-draft.md)에 사용자 결정을 기록했다. 구현은 `.fl` 컴파일러·C 런타임과 Bash 시험에 한정한다. C04 이후 작업, blog1 코드·실데이터 수정, 23·25 접속·작업, 합치기·배포·강제 푸시는 하지 않았다.

정답 기준은 [v11 소스 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4`](https://github.com/kimjindol2025/freelang-v11/tree/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4)다. TS·bootstrap JS는 기존 Git 객체를 텍스트로 읽었으며 실행하거나 새 파일로 작성하지 않았다. Node로 기준 출력을 만들지 않았다.

### 정답 선행과 구현 커밋

| 순서 | 전체 SHA | 내용 |
|---|---|---|
| 첫 정답 선행 | `00c67bd4ba75c68b4bed171b968e90b9ad66450c` | 이름 충돌·재정의·함수 값·get·load `.fl`와 정답 |
| 시험 범위 정정 | `6f8b54a5a4d6a32a0e0d48005f5aea8688779fa5`, `92b0537b84b3e90a990ea0805f080fa20ef5009b` | 배열 표시와 nil 표시의 C09 차이를 원소 값·nil? 검사로 분리. 언어 수정 전에 커밋 |
| 추가 정답 선행 | `5dadc222b8b59678f4e38a8241e0a1473f8d176c`까지 | 순환 load, 지역 함수·재귀·캡처, eager default/추가 인자, strict 오류 필드, variadic callback, push·잘못된 함수 값 |
| 주요 구현 | `c7a2549bdecd8ea585a9934ce17b2a48370d1f55` | 이름 바인딩·재정의·load·함수 값·swap!·get |
| 별칭 정답 선행 | `6cd057451cdf2b781ee99052907cd0fc2f4f9bfd`, `1018f9c0694a1de4c4aee991b243d5dd34c97a38` | v11의 정확한 이름 우선·함수 별칭과 db95 방식 server-start/HTTP server_start 공존 |
| 별칭 수정 | `e3888772fed1ccfa6d84b7f6c5b36c029cbc3ef3` | 정확한 사용자·네이티브 이름을 찾은 뒤에만 함수 별칭 조회. native 이름 metadata 검사 |
| nil 키 정답 선행 | `924026d3b54d027cd123f056dc7f645f3075bb81` | has-key?의 nil 키는 빈 문자열, contains?는 null 문자열이라는 소스 계약 |
| 마지막 구현 수정 | `52ffdb7e99cad43f9aeba641bafc3fe5c7727adf` | contains? nil 키의 변환 보존. 이후 구현 소스 변경 없음 |

초기 시험의 배열 문자열 `[2,3]`와 nil 출력 예상은 C09의 표시 형식까지 섞은 설계였다. 해당 부분을 구현에 맞춰 바꾸는 대신 C01~C03의 반환값을 직접 검사하도록 위 선행 커밋에서 수정했다. 이후 추가 경계 시험도 대응 수정 전에 정답을 커밋했다. 마지막 시험 커밋 이후 구현 수정으로 `.expected`를 바꾸지 않았다. [정답 17개 SHA-256](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/answers-sha256.md)을 보관한다. 이 중 하나는 HTTP 정답이며, 자체 시험의 누락 load 실패 판정은 구동부에 고정했다.

### 구현한 동작

| 항목 | 변경과 자체 시험 범위 |
|---|---|
| C01 | 소스 이름과 생성 C 식별자를 분리한다. 하이픈/밑줄/술어 이름 충돌, 사용자 함수의 native 이름 shadowing, 재정의 후 호출과 이전 함수 값, 지역 함수·재귀·캡처·scope를 검사한다. 실제 파일 정규 경로로 중복·링크·순환 load를 제거하고 없는 의존 파일은 비정상 종료한다. |
| C01 별칭 | v11 소스의 정확한 이름 우선·하이픈/밑줄 대체 규칙을 보존한다. 실제 native `server_start`보다 사용자 `server-start` 별칭을 먼저 고르는 우회는 막는다. [metadata 생성·검사](https://github.com/kimjindol2025/freelang-v11-fx/blob/52ffdb7e99cad43f9aeba641bafc3fe5c7727adf/scripts/refresh-native-symbols.sh)는 C 선언·컴파일러 분기에서 이름 목록을 재생성해 불일치를 실패로 처리한다. |
| C02 | blog1에서 함수 값으로 쓰는 assoc·dissoc·conj·update와 기존 연산자 경로를 연결한다. swap!의 함수 값·람다·추가 인자·한 번 평가·예외 전달, 지역 캡처, 여러 본문 식, 잘못된 함수 값의 catch, 기존 push 회귀를 검사한다. 임시 argv는 정상·예외 경로 모두 정리한다. |
| C02 HTTP | 문자열 함수 이름은 요청 시점의 이름 등록표에서 조회한다. 이전에 저장한 함수 값은 이전 구현을 유지한다. 재정의·나중 정의·define의 함수 값·하이픈/밑줄·native HTTP와 db95 이름 공존을 실제 loopback HTTP로 검사한다. 포트 0을 원자적으로 bind하며 자기 시험 과정만 종료한다. |
| C03 | get의 기본값, map의 저장된 nil과 누락, 배열 nil의 기본값, false/0/빈 문자열 보존, 글자 인덱스, 기본값·추가 인자의 선행 평가, 0/1/추가 인자 및 FL_STRICT nil 오류의 code/type/message/hint를 검사한다. has-key?·contains?는 nil 값의 키도 구분한다. seed용 기존 C get ABI와 L07 raw API는 유지한다. |

[바인딩 패스](https://github.com/kimjindol2025/freelang-v11-fx/blob/52ffdb7e99cad43f9aeba641bafc3fe5c7727adf/runtime/compiler-bindings.h), [컴파일러 연결](https://github.com/kimjindol2025/freelang-v11-fx/blob/52ffdb7e99cad43f9aeba641bafc3fe5c7727adf/self/cgc-main.fl), [get·호출 런타임](https://github.com/kimjindol2025/freelang-v11-fx/blob/52ffdb7e99cad43f9aeba641bafc3fe5c7727adf/runtime/collection.c), [함수 값](https://github.com/kimjindol2025/freelang-v11-fx/blob/52ffdb7e99cad43f9aeba641bafc3fe5c7727adf/runtime/aliases.c), [HTTP 호출](https://github.com/kimjindol2025/freelang-v11-fx/blob/52ffdb7e99cad43f9aeba641bafc3fe5c7727adf/runtime/http.c).

### 수정 전 실패 원문과 개발 중 실패

증거의 stdout/stderr는 로컬 절대 경로만 `<가림>`으로 치환했다. 날짜·시간·수·해시는 바꾸지 않았다. EXIT/DIFF 표시는 시험 구동부의 종료 코드이며, 프로그램이 0이어도 고정 정답과 다르면 시험 실패다.

| 증거 | 실제 결과 |
|---|---|
| [최초 실행](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/before-first.txt) | `foo_bar`, `foo_p`, `server_start` 재정의 컴파일 오류. 없는 load를 성공 처리. 최초 fixture 집합 `L08_PASS=0 FAIL=8`, `EXIT=1`. 위 출력 형식 정정 전 집계이며 최종 17개와 같은 집합이라고 주장하지 않는다. |
| [get 기본값](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/before-default.txt) | 기본값 7 대신 nil. 비strict nil 접근도 uncaught error, `EXIT=1` |
| [순환 load](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/before-cycle.txt), [람다 본문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/before-order.txt) | 각각 `0`을 두 번 출력, 마지막 식 99 대신 1. 프로그램 종료 0이지만 정답과 다름 |
| [지역 함수 생성 C](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/before-local.txt) | 캡처 변수 미선언. 보관한 수정 전 생성 C를 다시 검사해 `EXIT=1` |
| [get 인자 수](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/before-get-edge.txt) | 개발 중 엄격한 2/3인자 제한이 v11 소스와 다름을 확인해 제거 |
| [strict 오류 필드](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/before-strict-fields.txt) | code/type/message/hint 모두 nil, `PROGRAM_EXIT=0`, `EXPECTED_DIFF_EXIT=1` |
| [nil 키 변환](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/before-membership.txt) | contains?가 nil 키를 빈 문자열/nil 표기로 취급해 정답과 다름. `PROGRAM_EXIT=0`, `EXPECTED_DIFF_EXIT=1` |
| [함수 값 인자](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/before-callbacks.txt) | 여러 인자 덧셈이 3, 단항 함수 값 호출이 잘못된 값 출력. `EXPECTED_DIFF_EXIT=1` |
| [잘못된 함수 값](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/before-not-fn.txt) | catch하지 못하고 `error: not a fn`, `EXIT=1`. 고정 기준 소스의 별도 복사본과 보관 baseline compiler로 재확인 |
| [HTTP define](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/before-http.txt), [요청 시점 조회](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/before-http-late.txt) | define/나중 정의의 함수가 404. 재정의 전 함수가 호출됨. 각 `EXIT=1` |
| [GC 회귀](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/before-gc-regression.txt) | `push` 함수 값 연결 누락으로 기존 GC 시험 빌드 실패. `REGRESSIONS_EXIT=1`, bootstrap 미실행 |
| [bootstrap 별칭](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/before-bootstrap-alias.txt) | stage2에서 `json_keys` 미선언. `BOOTSTRAP_EXIT=1`. 컴파일러 내부 호출을 바꿔 숨기지 않고 v11 함수 별칭을 언어에 구현 |

중간 개발 실패는 보관한 개발 바이너리·생성 C의 보조 기록도 포함한다. 그 중간 상태 전체에 별도 소스 커밋 SHA가 있었다고 주장하지 않는다. 최초 기준 소스, 정답 선행 커밋, 최종 구현 커밋은 위 표로 구분했다.

### 최종 명령·결과

아래 명령의 경로는 `<가림>`이다. 기존 시스템 libcurl 8.9.1과 맞는 헤더·링크 디렉터리를 재사용했다. 새 실행기 설치나 sudo는 사용하지 않았다.

```bash
CPATH=<가림> LIBRARY_PATH=<가림> L08_OUT=<가림> bash scripts/verify-l08-all.sh
```

[전체 드라이버](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/scripts/verify-l08-all.sh)는 자체 시험 → 실제 HTTP → 기존 L06 전체 드라이버 순서다. L06 드라이버는 L06 HTTP/TLS → L07·기존 회귀 → `verify-native-bootstrap.sh` 순서를 유지한다. 테스트 정답, 기존 L06/L07 시험·드라이버, bootstrap 검증기와 고정 stage0 원본은 수정하지 않았다. bootstrap은 기존 L07 보관 도우미와 PID 격리를 사용하며 다른 과정은 종료하지 않는다.

출력 디렉터리는 실행마다 새로 만들었다. 마지막 구현 소스 `52ffdb7e99cad43f9aeba641bafc3fe5c7727adf`를 기준으로 아래 결과를 확인했다. 그 뒤 런타임·컴파일러·시험·스크립트의 변경은 없다.

| 시험 | 원문 결과 | 종료 코드·원문 |
|---|---|---|
| C01~C03 `.fl` 자체 시험 | `L08_PASS=17 FAIL=0` | `self_EXIT=0` · [출력](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/after-self.txt) |
| 실제 loopback HTTP | 7건 모두 `200`, `L08_HTTP_PASS=7` | `http_EXIT=0` · [출력](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/after-http.txt) |
| 기존 L06 HTTP | `L06_HTTP_PASS=110 FAIL=0` | HTTP/TLS 단계 `0` · [출력](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/after-l06-http.txt) |
| 기존 L06 TLS | `L06_TLS_PASS=110 FAIL=0` | 같은 단계 `0` · 같은 출력. 로컬 자체서명 시험이며 운영 인증서 시험은 아님 |
| 기존 시간 제한 | `L06_TIMEOUT_PASS=2 FAIL=0` | 같은 단계 `0` · 같은 출력 |
| 기존 L07 자체 시험 | `L07_SELF_PASS=221`, `L07_SELF_FAIL=0` | `REGRESSIONS_EXIT=0` · [출력](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/after-l07.txt) |
| 기존 C 런타임 | `PASS` 53건, `All tests PASS` | 같은 회귀 단계 `0` · [출력](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/after-runtime.txt) |
| 기존 JIT·GC | `결과: 10 PASS / 0 FAIL / 10 총` | 같은 회귀 단계 `0` · [출력](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/after-regressions.txt) |
| native bootstrap | `STAGE2_EQ_STAGE3=PASS`, `RESIDUAL_PROCESSES=0` | `BOOTSTRAP_EXIT=0` · [출력](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/after-bootstrap.txt) |
| 전체 드라이버 | `L08_C01_C03_GATES=PASS` | `ALL_EXIT=0` · [전체 원문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/full-run.txt) |

기존 검증기는 `STAGE0_EQ_CANONICAL=DIFFERENT`도 출력한다. 이 표기를 숨기거나 기대값을 바꾸지 않았다. 원래 검증기가 요구하는 stage0 SHA는 아래 값과 같으며, stage2/stage3 고정점까지 통과했다.

```text
STAGE0_SHA=f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c
STAGE1_SHA=fa3a40814936cf2922eccc12107391c510a0575e5ddabc60bce5e6250c49d9a1
STAGE2_SHA=33a7bfcdd4eaf94e9500f149bf882c67a8cd32be503d65a39a816f8125dd778f
STAGE3_SHA=33a7bfcdd4eaf94e9500f149bf882c67a8cd32be503d65a39a816f8125dd778f
```

### strace와 금지 실행기 검사

자체 시험·HTTP는 빌드부터 자식 실행까지 `strace -f -e trace=execve`로 추적했다. 기존 L06 드라이버와 bootstrap은 기존 추적·보관 경로를 그대로 사용했다. 원본 추적 전체와 디렉터리를 가린 호출행 발췌에 같은 감사기를 각각 적용했다.

```text
NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0
```

[감사 원문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/trace-audit.txt), [실행 파일 목록·호출 시도 수](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/executed-programs.txt), [전체 원본 추적 SHA-256](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/trace-sha256.md), [가린 execve 호출행 발췌 gzip](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l08-c01-c03/docs/evidence/l08-c01-c03/execve-redacted.txt.gz)을 보관했다. 호출 시도 수에는 PATH 검색의 ENOENT도 포함하며 성공한 과정 수와 다르다. 발췌는 모든 execve 호출행을 포함하고 디렉터리만 가려 실행 파일 이름·PID·인자·결과는 유지했다. strace가 원래 생략한 argv 문자열을 복원했다고 주장하지 않는다.

gzip SHA-256: `c8e6b4e3d11c4ef1ab7e4c14ef0b81249c26754174a458d4bab14a7980be9ef0`.

```bash
bash scripts/audit-l07-execve.sh <(gzip -dc docs/evidence/l08-c01-c03/execve-redacted.txt.gz)
```

마지막 실행 이후 정답 SHA 검사, native 이름 metadata 검사, 공백 오류·민감 정보·금지 확장자·로컬 절대 경로 검사를 수행했다. Forgejo → GitHub로 같은 가지를 정상 푸시하고 두 원격 HEAD를 재조회해 최종 응답으로 보고한다. 보고 커밋 자체 SHA는 자기 문서에 순환 기록하지 않는다.

## 미확인

- blog1 전체 프로그램의 fx 컴파일·실행·운영 복사본 이전은 수행하지 않았다. 이 결과를 L08 전체 완료나 배포 가능 판정으로 쓰지 않는다.
- C04 이후 항목, 기존 문자열 길이/slice의 전체 Unicode 계약, 텍스트 파일·이미지·정규식·날짜·저장소·인증·상주 작업·브라우저 출력은 이번 구현 범위가 아니다.
- native 전체 표준 함수의 모든 함수 값 조합, v11의 다른 평가 경로·preview flag 조합, JS Error의 raw 객체·stack 표현 동일성은 검증하지 않았다. 이번 get 정답은 지시한 v11 일반 소스 계약과 FL_STRICT 공개 필드를 기준으로 한다.
- 전체 키 타입의 동일성까지 완료한 것은 아니다. 비문자 map 키의 수→문자열 표현은 현재 native 변환에 의존하며, 부동소수의 최단 문자열 표현 등 C06/C09/C12와 공유하는 차이는 남는다. 이번 기본값·멤버 존재 시험 통과를 그 자료형·표시 계약의 완료로 대신하지 않는다.
- C20 atom의 장기 수명·동시성·WAL 내구성은 별도 과제다. 시험 통과를 그 기능의 완료 증거로 대신하지 않는다.
