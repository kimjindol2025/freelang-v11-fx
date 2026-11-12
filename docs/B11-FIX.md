# B-11 복사 비용과 속도 조사

B11=계속

입력 두 배의 최대 RSS 비율은 1.7604배·1.9151배·1.9336배로 모두 2.2배 이하다. 8192바이트는 0.50초로 1초 이하다. 2GiB 안에서 stage4 생성·빌드를 완료했고 생성 C 고정점과 API 20/20이 유지됐다.

출발 HEAD는 4f8a7f779de18dc9a2e11cbac9c1cd1999c26ec1이며 시작 작업 트리는 CLEAN이었다. fix/b11-copy-speed를 별도 작업 트리에 만들었다. 속도 원인의 프로파일·대조는 소스 수정 전에 완료했고, 추가 복사 경로도 해당 구현을 바꾸기 전에 확인했다. 임시 경로·주소·소유자는 <가림>으로 표시한다.

## 수정 전 프로파일과 비교

perf는 없고 gprof가 있어 B-10의 생성 C stage4.c와 변경 전 런타임을 gcc -O2 -pg로 빌드했다. 8192바이트 원본 입력을 2GiB 주소 공간 상한에서 실행했다. 프로파일은 계측 빌드이며 성능 판정용 일반 빌드의 경과 시간과 구분한다. gprof flat profile의 자기 시간 비율 상위 5개 원문:

| 함수 | 비율 |
| --- | --- |
| t_unit | 72.89% |
| t_put | 10.64% |
| t_cp | 6.22% |
| t_encode | 2.81% |
| _init | 2.21% |

각 표본은 0.01초다. gprof 자기 시간 합은 4.98초이며 계측 실행의 실제 경과는 23.93초, 최대 RSS는 335996KiB였다. _init 항목은 gprof가 자발 진입으로 분류한 것으로 호출 원인을 더 구체적으로 단정하지 않는다. 프로파일의 비율을 전체 경과 시간 비율로 바꿔 보고하지 않는다.

호출 그래프에는 t_equal → t_unit, fl_v11_text_get → t_unit, fl_v11_builtin → t_unit이 있다. v11 문자열 비교·길이·문자 조회는 UTF-16 변환을 반복하며, 생성 코드의 문자열 누적도 점점 긴 문자열을 다시 인코딩한다. t_unit 자체의 용량은 이미 두 배 확장이고, 매 항목마다 용량을 한 칸만 늘리는 구현은 아니었다.

느려진원인=B-10 세 파일 변경 자체가 7~8배 저하의 주원인이라는 증거는 없다. B-9의 보존 stage1과 B-10의 정식 자기 컴파일 stage4를 비교하면서 현재 코드 생성의 범용 v11 문자열 처리 비용이 드러났다. 같은 코드 생성 방식·같은 B-10 런타임에서 B-10 이전 렉서로 되돌린 임시 비교 빌드는 8192바이트에 4.99초, RSS 341788KiB였고 결과 C도 같았다. B-10의 최종 4.91초와 비슷하다. 보존 stage1에 런타임 두 파일 변경만 적용한 기존 대조는 0.57초에서 0.21초였다.

16384바이트의 B-10 추가 기준 측정은 exit 0, 18.98초, RSS 1049428KiB였다. 기존 2048·4096·8192바이트 입력은 원본을 그대로 복사했고, 16384바이트 입력은 8192바이트 입력을 두 번 이어 만들었다. 기존 시험·정답·실행기를 변경하지 않았다.

수정 전 확인한 복사 위치:

- self/cgc-main.fl:632: parse-args의 불변 배열 append 누적.
- self/cgc-main.fl:714: parse-all → p-append-ast의 전체 AST 배열 누적. 실제 append는 436줄.
- self/cgc-main.fl:2254, 2258: 함수·명령문 전체 문자열의 반복 재누적.
- self/cgc-main.fl:2587, 2620: append-all 및 expand-loads의 불변 배열 push 누적.
- runtime/v11-compat.h:1007: 문자열 분리의 불변 벡터 push 누적. 빈 구분자로 문자 배열을 만들 때도 같은 비용이 발생한다.
- runtime/compiler-bindings.h:129, 142, 248-264: 바인딩 정규화의 본문·자식 목록·전체 프로그램 배열 누적. 앞선 임시 구현에서 문자열 비용을 줄인 뒤에도 남아 있던 이 경로를 확인하고 함께 수정했다.

위 줄 번호는 출발 커밋의 수정 전 위치다. 이후 구현과 최종 검증은 다음과 같다.

## 구현

- make-state에서 기존 str-split의 빈 구분자 의미를 사용해 UTF-16 문자 배열을 한 번 만든다. atom/deref로 소유된 불변 배열을 얻어 함수 반환 스냅샷이 전체 소스를 다시 복사하지 않도록 하고, peek-at은 이 배열을 get으로 읽는다. UTF-16 단위·빈 문자열·경계 의미를 바꾸지 않았다.
- str-split·str-lines·str-replace의 공통 분리 경로는 기존 fl_vec_builder_new/push/freeze를 사용한다. 분리 순서와 limit·join 처리는 그대로 유지한다. 새 분리기나 새 이어붙이기 도구를 만들지 않았다.
- parse-args·parse-all의 누적 배열, append-all·expand-loads의 누적 배열을 지역 builder로 만든다. 기존 시작 배열은 한 번 복사하고 결과를 반환할 때 한 번 고정한다. builder가 함수 간 상태 맵으로 넘어가지 않도록 유지했다.
- cgc-top-level은 함수·명령문 조각을 지역 builder에 넣고 마지막에 기존 str-join으로 합친다. 기존 접두 문자열과 hoisted 함수의 순서·개행을 보존했다.
- compiler-bindings.h의 바인딩·매개변수·자식 목록·전체 프로그램 누적도 기존 builder를 사용하고, AST에 넣거나 반환하기 전에 고정한다.

B-10의 runtime/collection.c와 runtime/aliases.c는 수정하지 않았다. 할당 실패 검사·메시지·종료 처리는 유지한다. 시험·기대값·제한시간·실행기·seed도 수정하지 않았다.

## 실행 조건

무거운 빌드·측정 직전에 ps를 확인하고 지정 공용 잠금에서 flock -o로 직렬 실행했다. native 실행과 gcc 빌드에 주소 공간 2097152KiB 상한과 core 0을 적용했다. TMPDIR은 작업 전용 디렉터리 <가림>으로 전달했고 산출물도 그 안에만 보존했다. 설치·Stage0·8GiB 실행은 하지 않았다. 자기 컴파일은 strace 없이 실행했다.

시작 native 컴파일러는 B-10에서도 사용한 보존 stage1.c를 현재 런타임과 재링크한 것이다. 보존 C를 고치거나 seed를 교체하지 않았다.

```text
ARCHIVED_STAGE1_C_SHA=b4cdbc346b43708d9f218301e2c84ea49934edd3d688c1b0f5814c9680314448
SEED_C_SHA=f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c
```

## 네 크기: 최종 stage4 측정

일반 gcc -O2 빌드로 측정했다. gprof 계측 빌드의 시간을 이 표에 섞지 않았다. B-10의 앞 3건은 이전 원문이고, 16384바이트는 이번 소스 수정 전에 B-10 최종 컴파일러로 추가 측정했다. RSS의 KB는 GNU time 형식이며 KiB 단위다. 모든 행의 shell exit는 0이다.

| 입력 바이트 | B-10 초 | B-10 RSS(KB) | B-11 초 | B-11 RSS(KB) |
| --- | --- | --- | --- | --- |
| 2048 | 0.43 | 45060 | 0.16 | 29628 |
| 4096 | 1.38 | 119488 | 0.25 | 52160 |
| 8192 | 4.91 | 335464 | 0.50 | 99896 |
| 16384 | 18.98 | 1049428 | 1.11 | 193164 |

```text
FINAL_BYTES=2048 ELAPSED=0.16 MAX_RSS_KB=29628 EXIT=0
FINAL_GENERATED_C_IDENTICAL=1
FINAL_BYTES=4096 ELAPSED=0.25 MAX_RSS_KB=52160 EXIT=0
FINAL_GENERATED_C_IDENTICAL=1
FINAL_BYTES=8192 ELAPSED=0.50 MAX_RSS_KB=99896 EXIT=0
FINAL_GENERATED_C_IDENTICAL=1
FINAL_BYTES=16384 ELAPSED=1.11 MAX_RSS_KB=193164 EXIT=0
FINAL_GENERATED_C_IDENTICAL=1
```

각 생성 C를 기존 컴파일러의 같은 입력 생성 C와 cmp로 비교했다. 네 결과 모두 같다. 메모리 배율은 소수점 아래 4자리에서 버린 값으로 1.7604·1.9151·1.9336이며, 판정은 버린 숫자가 아니라 원래 정수 RSS의 `현재×10 <= 이전×22`로 확인했다. 8192바이트 시간도 원문 0.50초로 확인했다. 문턱값을 조정하지 않았다.

## native 단계와 생성 C 고정점

```text
STAGE1_BUILD=PASS ELAPSED=16.54 MAX_RSS_KB=161492 EXIT=0
STAGE1_TO_STAGE2=PASS ELAPSED=3.06 MAX_RSS_KB=505340 EXIT=0
STAGE2_BUILD=PASS ELAPSED=33.67 MAX_RSS_KB=485724 EXIT=0
STAGE2_TO_STAGE3=PASS ELAPSED=13.27 MAX_RSS_KB=684464 EXIT=0
STAGE3_BUILD=PASS ELAPSED=32.53 MAX_RSS_KB=485840 EXIT=0
STAGE3_TO_STAGE4=PASS ELAPSED=13.13 MAX_RSS_KB=685036 EXIT=0
STAGE4_BUILD=PASS ELAPSED=33.17 MAX_RSS_KB=485824 EXIT=0
GENERATED_C_FIXED_POINT=PASS
```

stage완료=stage2 완료 / stage3 완료 / stage4 완료. 자기 컴파일 최대 RSS는 685036KiB다. 생성 C의 sha256sum 원문:

```text
C_STAGE2=0dd2b662803aed1bc3392ac855a73e8c1066c07ec3bb46fee81ffa13a505f1aa
C_STAGE3=0dd2b662803aed1bc3392ac855a73e8c1066c07ec3bb46fee81ffa13a505f1aa
C_STAGE4=0dd2b662803aed1bc3392ac855a73e8c1066c07ec3bb46fee81ffa13a505f1aa
```

고정점=0dd2b662 / 0dd2b662 / 0dd2b662, 일치. C 파일을 cmp로도 비교했다. .fl 컴파일러 소스 변경에 따라 B-10의 고정점과는 다르며, 이번 세 단계 사이의 동일성을 판정했다.

ELF 해시는 별도 식별값으로만 기록한다. 서로 다른 ELF 해시를 생성 C 고정점 불일치로 판정하거나 ELF 해시로 고정점을 대신 판정하지 않았다.

```text
ELF_STAGE1=f92c81d365eaf59d6926005802a0f08febb58fadde41699fd349e00d79266d39
ELF_STAGE2=d7dff202b7a1c1a0998d7c6c3bad4220a870f1aea0c5d904f861529be0963232
ELF_STAGE3=598cadebe2301adcf6f2219914f3fd2b6a7236618b000ca17eefde518b750128
ELF_STAGE4=1cb80979e66fce943e9f6595c7db9da4c88e05eeed9d2d87bb46106616c9e70c
```

## 기존 의미와 API

문자열 분리·빈 입력·인접/끝 구분자·한글과 비 BMP 문자·분리 limit·줄 분리·치환의 임시 진단 7건은 수정 전에 기존 런타임의 출력을 보존하고, 같은 생성 C를 새 런타임과 링크해 바이트 비교했다. 기존 native atom-snapshot 시험도 원본·기존 expected·기존 90초 제한으로 실행했다.

```text
STRING_DIAGNOSTIC=7/7_IDENTICAL
EXISTING_NATIVE_SNAPSHOT=7/7
API_JS_GENERATED_IDENTICAL=1
ELAPSED=0.69 MAX_RSS_KB=135592 EXIT=0
```

마지막 시간·RSS는 stage4의 browser-storage.fl → JS 생성 측정이다. 이 JS도 이전 개발용 JS와 cmp로 일치했다. 위 추가 진단 14건을 필수 API 20개의 통과 수에 합산하지 않았다.

브라우저 검사는 기존 browser-storage.fl·HTML·20개 정답을 그대로 사용했다. 기존 로컬 시험 서버를 새 임시 루트에서 실행하고 Chrome headless dump-dom·screenshot으로 확인했다. 실제 계정·운영 DB·운영 서버에는 접근하지 않았다. 서버와 Chrome에는 같은 user cgroup의 MemoryMax=2147483648, MemorySwapMax=0을 적용했다. V8의 주소 공간 예약 때문에 Chrome에는 native의 RLIMIT_AS 대신 이 실제 메모리 상한을 적용했다. native 컴파일러의 2GiB 주소 공간 상한은 그대로 유지했다.

```text
API_RUN_EXIT=0
API_GROUP_MEMORY_MAX=2147483648
API_GROUP_SWAP_MAX=0
API_SERVER_LOOPBACK_ONLY=1
API=20/20
STORAGE_MISSING=PASS
STORAGE_WRITE_RESULT=PASS
STORAGE_ROUND_TRIP=PASS
STORAGE_EMPTY_VALUE=PASS
STORAGE_UNICODE=PASS
STORAGE_OVERWRITE=PASS
STORAGE_REMOVE_RESULT=PASS
STORAGE_REMOVED=PASS
STORAGE_REMOVE_IDEMPOTENT=PASS
STORAGE_OTHER_KEY=PASS
STORAGE_READ_KEY_TYPE=PASS
STORAGE_WRITE_KEY_TYPE=PASS
STORAGE_REMOVE_KEY_TYPE=PASS
STORAGE_EMPTY_KEY=PASS
STORAGE_VALUE_TYPE=PASS
STORAGE_READ_ARITY=PASS
STORAGE_WRITE_ARITY=PASS
STORAGE_REMOVE_ARITY=PASS
STORAGE_ACCESS_DENIED=PASS
STORAGE_QUOTA_PRESERVES_OLD=PASS
API_GROUP_MEMORY_PEAK_BYTES=258420736
```

```text
CHROME: ELAPSED=1.42 MAX_RSS_KB=254220 EXIT=0
B8_STDERR_CHECK=PASS
API_ORACLE_AND_RESULT_SHA=70fb285b6777bf480a616bea547cf5bcb6c25fe99f55e826af1c489da61bc1f9
```

## 중간 실패 보존

첫 임시 소스에서 expand-loads의 닫는 괄호 하나가 빠졌다. 기존 bash 괄호 검사 결과는 PARENS_FILES=1, PARENS_ERRORS=1이었다. 검사 종료 코드를 제대로 확인하지 않아 빌드가 진행됐다. 그 임시 빌드의 native 실행은 exit 0이었지만 결과 C가 없어 기존 결과 비교에 실패했다(0.00초, RSS 5420KiB). 성공 측정으로 쓰지 않고 소스·C·실행 파일·로그를 별도 보존했다. 괄호를 고친 뒤 기존 검사는 PARENS_FILES=1, PARENS_ERRORS=0, exit 0이었고 이후 위의 실제 생성물 비교·고정점을 확인했다.

첫 브라우저 준비에서는 임시 tmp를 755 권한으로 만들었다. 20개 단언과 원본 정답은 일치했지만 기존 서버가 `UPLOAD_STORAGE_UNAVAILABLE` 진단을 내서 기존 stderr 검사가 실패했다. 서버의 기존 요구는 해당 디렉터리 700 권한이었다. 실패 로그를 보존하고 새 700 권한 디렉터리에서 같은 JS·HTML·서버·검사로 다시 실행해 위의 종료 0과 stderr 통과를 확인했다. 서버·필터·러너·정답을 바꾸거나 진단을 숨기지 않았다.

같은 원인으로 세 번 실패한 뒤 재실행하지 않았다. 원문·DOM·스크린샷·프로파일과 중간 결과는 작업 전용 임시 디렉터리 <가림>에 보존한다.

## 전달

B11=계속. 두 성능 문턱과 stage4 완료·생성 C 고정점·API 20/20을 모두 충족했다.

바뀐_파일=self/cgc-main.fl, runtime/v11-compat.h, runtime/compiler-bindings.h, docs/B11-FIX.md

민감정보 diff를 확인한 뒤 이 네 파일만 커밋하고 GitHub fix/b11-copy-speed에만 푸시한다. Forgejo는 보류한다. 최종 커밋 HEAD·원격 HEAD 일치·CLEAN 여부는 푸시 후 최종 응답에 기록한다.
