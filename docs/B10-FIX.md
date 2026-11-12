# B-10 렉서 builder와 할당 실패 처리

B10=보류

2GiB 안에서 stage4 생성·빌드와 Stage2=3=4 고정점, API 20/20, 명시적 할당 실패 종료를 확인했다. 그러나 같은 입력의 전체 컴파일 RSS가 두 배 입력당 2.6517배·2.8075배로 늘어, ‘계속’ 조건인 대략 비례까지는 충족하지 못했다. 렉서 누적은 기존 builder로 바꿨지만 파서·코드 생성의 다른 누적 복사는 남아 있다. API 실패나 기존 결과물 변화는 관측되지 않아 되돌림 조건은 발생하지 않았다.

출발 HEAD: `85712082e647fb6278bd54e8e2e4184f8c14ed2a`. 시작 작업 트리는 CLEAN이었다. 이 커밋에서 별도 작업 트리와 `fix/b10-lexer-builder` 가지를 만들었다. 임시 입력·빌드·로그 경로와 소유자 정보는 <가림>으로 표시한다.

## 수정 범위

- self/cgc-main.fl: 토큰 숲의 concat 병합을 없애고, 기존 vec-builder / append / freeze-builder를 사용한다. builder는 lex-loop 지역 변수에만 둔다. 함수 사이를 오가는 상태 맵에는 마지막 토큰 하나만 담아 반환값 스냅샷 경로로 누적 builder를 보내지 않는다. 모든 토큰을 순서대로 추가한 뒤 한 번 고정한다.
- runtime/collection.c: 불변 push의 구조체·데이터 할당을 검사한다. builder 생성·realloc 및 freeze에서 쓰는 fl_vec_from에도 검사를 넣었다. realloc 성공 전에는 기존 포인터와 용량을 덮어쓰지 않는다. 실패 시 FL 오류 값을 추가 할당하지 않고 `FreeLang: vector allocation failed`를 stderr에 출력한 뒤 EXIT_FAILURE로 종료한다. 원소 수·용량의 uint32_t 넘침도 거부한다.
- runtime/aliases.c: 벡터 concat의 반복 불변 push를 기존 fl_vec_builder_new/push/freeze로 바꿨다. 보존된 stage1 C에는 예전 토큰 합치기 호출이 들어 있으므로, .fl만 고치면 첫 stage2 생성이 B-9의 복사 경로를 그대로 지난다. 이 초기 자기 컴파일을 위해 필요한 수정이다. 입력 두 벡터의 순서와 불변 결과 계약은 유지한다.

새 이어붙이기 도구나 builder API는 만들지 않았다. 기존 시험·기대값·시간 제한·실행기는 수정하지 않았다. seed 교체·Stage0 실행·설치·8GiB 실행도 하지 않았다.

## 메모리 제한과 stage1 식별

무거운 빌드·측정 전에 ps를 확인하고 지정된 공용 잠금에서 `flock -o`로 직렬 실행했다. native 빌드와 실행에는 `ulimit -v 2097152`, `ulimit -c 0`을 적용했다. 주소 공간 상한은 2147483648바이트다. `TMPDIR=<가림>`을 전달하고 작업 전용 임시 디렉터리에만 산출물을 만들었다.

B-9의 보존 stage1.c를 수정 없이 새 런타임과 링크했다. 기존 bootstrap 실행기를 호출하지 않고 동일한 런타임 소스 목록·gcc -O2 -w·링크 옵션을 사용했다.

```text
ARCHIVED_STAGE1_C_SHA=b4cdbc346b43708d9f218301e2c84ea49934edd3d688c1b0f5814c9680314448
SEED_C_SHA=f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c
STAGE1_BUILD=PASS
ELAPSED=17.44 MAX_RSS_KB=173364 EXIT=0
```

## 같은 입력 3건: stage1 재측정

B-9의 입력 원본을 그대로 재사용했다. 각 줄은 `(+ 1 2)`와 LF이며 256·512·1024줄이다. 이 stage1은 보존 C에 새 런타임을 링크한 것으로, .fl의 새 렉서가 실행되는 최종 단계 측정과 구분한다. RSS의 KB는 GNU time 형식이며 KiB 단위다.

| 입력 바이트 | B-9 시간(초) | B-9 최대 RSS(KB) | B-10 stage1 시간(초) | B-10 stage1 최대 RSS(KB) |
| --- | --- | --- | --- | --- |
| 2048 | 0.07 | 56760 | 0.03 | 23284 |
| 4096 | 0.17 | 195460 | 0.08 | 63640 |
| 8192 | 0.57 | 731724 | 0.21 | 211640 |

3건 모두 shell exit 0이고, 생성 C는 각각 B-9 원본과 cmp로 일치했다. 이 단계만으로 대략 비례한다고 판정하지 않는다. AST 누적 불변 append와 코드 생성 문자열 누적 경로는 여전히 남아 있다.

## 중간 시도 보존

첫 구현은 builder를 상태 맵 안에 넣었다. 새 stage2 생성과 빌드는 완료됐지만 stage3 생성 중, 함수 반환값의 snapshot_copy가 builder의 가변 표식을 보존하지 않는 경로를 정적으로 확인했다. builder를 지역 변수로 한정하도록 수정하기 위해 해당 실행만 TERM으로 종료했다. 그 시도는 stage3 통과나 메모리 한도 실패로 보고하지 않는다.

```text
Command terminated by signal 15
ELAPSED=231.54 MAX_RSS_KB=344072 EXIT=0
ATTEMPT1_TERMINATED_BY_OPERATOR=143
```

첫 시도의 C·실행 파일·로그는 임시 작업 디렉터리 <가림>에 따로 보존했다. 다음 시도에는 지역 builder 수정본을 사용했고 아래의 전체 단계가 완료됐다. 같은 점검을 세 번 실패한 뒤 재실행한 사실은 없다.

## 최종 stage4로 같은 입력 재측정

이 표는 수정된 .fl 렉서를 실제로 실행하는 최종 stage4 기준이다. 앞의 보존 stage1 재링크 결과와 구분한다. 입력 원본·컴파일 명령의 입출력 방식·GNU time 측정 방식·주소 공간 상한은 같다. 각 실행의 exit는 0이다.

| 입력 바이트 | B-9 시간(초) | B-9 최대 RSS(KB) | B-10 stage4 시간(초) | B-10 stage4 최대 RSS(KB) |
| --- | --- | --- | --- | --- |
| 2048 | 0.07 | 56760 | 0.43 | 45060 |
| 4096 | 0.17 | 195460 | 1.38 | 119488 |
| 8192 | 0.57 | 731724 | 4.91 | 335464 |

```text
FINAL_FORMS=256 ELAPSED=0.43 MAX_RSS_KB=45060 EXIT=0
FINAL_C_IDENTICAL=1
FINAL_FORMS=512 ELAPSED=1.38 MAX_RSS_KB=119488 EXIT=0
FINAL_C_IDENTICAL=1
FINAL_FORMS=1024 ELAPSED=4.91 MAX_RSS_KB=335464 EXIT=0
FINAL_C_IDENTICAL=1
```

최종 생성 C 3개 모두 B-9의 같은 입력 생성물과 cmp로 일치했다. 시간은 표의 실제 값을 그대로 기록했으며 개선됐다고 주장하지 않는다. 보존 stage1은 예전 형태의 생성 C이고, stage4는 현재 언어의 정규화·함수 호출 처리가 들어간 자기 컴파일 결과다.

증가모양=2제곱 이상(남아 있는 누적 복사 경로의 점근 비용). 실제 RSS 배율은 소수점 아래 4자리에서 버린 2.6517배·2.8075배이며, 실측 4배 이상을 주장하는 뜻이 아니다. self/cgc-main.fl:436의 AST 불변 append, self/cgc-main.fl:2258의 전체 명령문 문자열 재누적이 남아 있다. 특히 반복마다 길어지는 문자열을 새로 만들어 이전 arena 값을 유지하는 구조는 누적 비용이 제곱이다. 이들 경로의 기여율을 별도 계측한 것은 아니며, 이번에는 렉서 외 파서·코드 생성의 구현을 추가로 바꾸지 않았다. 실측 비례 증가가 충족됐다고 판정을 완화하지 않는다.

## native 자기 컴파일과 고정점

전체 단계는 strace 없이 실행했다. Stage0를 실행하지 않고 보존 stage1에서 시작했다. 2GiB 주소 공간 제한을 생성과 gcc 빌드에 모두 유지했다.

```text
STAGE1_TO_STAGE2=PASS ELAPSED=2.62 MAX_RSS_KB=511684 EXIT=0
STAGE2_BUILD=PASS ELAPSED=32.36 MAX_RSS_KB=532456 EXIT=0
STAGE2_TO_STAGE3=PASS ELAPSED=383.16 MAX_RSS_KB=820384 EXIT=0
STAGE3_BUILD=PASS ELAPSED=34.23 MAX_RSS_KB=486188 EXIT=0
STAGE3_TO_STAGE4=PASS ELAPSED=380.18 MAX_RSS_KB=819608 EXIT=0
STAGE4_BUILD=PASS ELAPSED=34.96 MAX_RSS_KB=486044 EXIT=0
STAGE2_EQ_3_EQ_4=PASS
```

stage완료=stage2 완료 / stage3 완료 / stage4 완료. 생성 C와 native 실행 파일을 모두 만들었다.

```text
STAGE2=82f2bdff39641449d830f158f5e0f386d646c6546432bcd015c86f1bb4b659ab
STAGE3=82f2bdff39641449d830f158f5e0f386d646c6546432bcd015c86f1bb4b659ab
STAGE4=82f2bdff39641449d830f158f5e0f386d646c6546432bcd015c86f1bb4b659ab
```

고정점=82f2bdff / 82f2bdff / 82f2bdff, 일치. 세 C 파일을 cmp로도 비교했다. B-9에서는 stage2 생성이 완료되지 않았으므로 이 값은 이번에 확인한 고정점이다. 렉서 소스 변경에 따라 컴파일러 생성 C가 달라졌으며, 이전 실패 실행의 파일을 고정점 기준으로 재사용하지 않았다.

## 할당 실패 처리 확인

최종 stage4로 임시 진단 .fl 두 개를 C로 생성하고 같은 런타임과 링크했다. 기존 시험 파일을 바꾸거나 그 기대값을 조정한 것이 아니다. 진단 실행만 65536KiB 주소 공간 상한으로 낮춰 실제 할당 실패를 유도했다. 이 값도 지정한 최대 2GiB 이하이며 기존 시험의 한도·시간 제한은 변경하지 않았다.

- 불변 경로: `(repeat 1048576 1)`로 fl_vec_push의 새 배열 할당 실패를 유도했다.
- builder 사용 경로: 지역 builder에 4194304개 항목을 추가하는 loop에서 메모리 부족을 유도했다. 이 경로에는 호출 인자 벡터 할당도 있으므로 realloc 실패 지점만 따로 검증한 것으로 보고하지 않는다.

```text
OOM_immutable_SHELL_EXIT=1
Command exited with non-zero status 1
ELAPSED=0.03 MAX_RSS_KB=44604 TIME_EXIT=1
FreeLang: vector allocation failed
OOM_builder_SHELL_EXIT=1
Command exited with non-zero status 1
ELAPSED=0.35 MAX_RSS_KB=50892 TIME_EXIT=1
FreeLang: vector allocation failed
OOM_PROBES=2/2
```

두 경로 모두 stderr를 정확한 오류 메시지와 비교했고 shell exit 1을 확인했다. 종료 139나 SIGSEGV로 통과 처리하지 않았다. 이 검증은 해당 벡터 경로에 대한 것이며 모든 런타임 할당 실패를 검증했다고 주장하지 않는다.

## 기존 개발용 API 20개

기존 tests/l01/browser-storage.fl, browser-storage.html, browser-storage.expected를 그대로 사용했다. 최종 stage4의 --browser-js 생성물은 이전 개발용 JS와 cmp로도 일치했다. self/browser-core.fl 및 내장 데이터의 --check도 통과했다.

```text
BROWSER_API_GENERATION=PASS
ELAPSED=14.53 MAX_RSS_KB=143004 EXIT=0
BROWSER_API_JS_IDENTICAL=1
BROWSER_CORE_EMBED_CHECK=PASS
```

기존 로컬 시험 서버 실행 파일만 새 임시 루트에서 사용했다. 실제 계정·운영 DB·운영 서버에는 접근하지 않았다. Chrome은 기존 HTML을 headless dump-dom·screenshot으로 실행했다. V8의 가상 주소 공간 예약을 허용하면서 실제 메모리 상한을 지키도록 검사용 서버와 Chrome을 같은 user systemd cgroup의 MemoryMax=2147483648, MemorySwapMax=0 아래 실행했다. native 컴파일러의 주소 공간 2GiB 제한은 별도로 유지했다. 이 환경 설정은 저장소의 시험·실행기 수정이 아니다.

첫 브라우저 준비 시도는 서비스 PATH에 rg가 없어 브라우저 실행 전에 127로 끝났다. 그 로그를 보존하고 현재 PATH를 환경으로 전달한 두 번째 시도에서 기존 정답을 비교했다. 시험·정답·대기 횟수·시간 제한은 바꾸지 않았다.

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
API_GROUP_MEMORY_PEAK_BYTES=305209344
```

```text
CHROME: ELAPSED=1.34 MAX_RSS_KB=255444 EXIT=0
B8_STDERR_CHECK=PASS
API_ORACLE_AND_RESULT_SHA=70fb285b6777bf480a616bea547cf5bcb6c25fe99f55e826af1c489da61bc1f9
```

API 결과 원문·DOM·스크린샷·메모리 로그는 임시 디렉터리 <가림>에 보존했다. 검사 종료 후 이번에 띄운 서버와 native 컴파일러·진단 실행 파일의 실행 중 프로세스는 0이었다. 기존 다른 작업은 종료하지 않았다.

## 최종 판정과 전달

B10=보류. stage4 완료·고정점 일치·API 20/20·명시적 할당 실패 종료는 통과했지만 전체 컴파일 메모리의 비례 증가가 남은 조건이다. 다음 검토 대상은 AST 불변 누적과 코드 생성 문자열 누적이며, 이번 변경 범위에서 추가 수정하지 않았다.

바뀐_파일=self/cgc-main.fl, runtime/collection.c, runtime/aliases.c, docs/B10-FIX.md

민감정보 diff와 문법상 diff 검사를 거쳐 이 네 파일만 커밋한다. GitHub fix/b10-lexer-builder에만 푸시하며 Forgejo는 보류한다. 최종 HEAD·GitHub 원격 HEAD 일치·CLEAN 여부는 푸시 후 최종 응답에 기록한다.
