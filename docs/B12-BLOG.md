# B-12 정식 빌드·블로그 재개 검수

B12=닫기

정식 빌드 후보는 2GiB 안에서 생성 C 고정점까지 도달했지만, 기존 native 전체 회귀의 브라우저 생성 관문이 실패했다. B12의 닫기 조건에 따라 이번 런타임 변경과 미검증 블로그 앱 변경을 원복했다. 아래 빌드·통과 기록은 원복 전 후보의 실행 증거다. 원복된 최종 소스의 정식 빌드 완료로 계산하지 않는다.

## 기준과 실행 조건

- 시작 소스 HEAD: cccfca9a354557dae0da29bf8d2d0862d6792214. CLEAN 확인 후 feat/b12-blog-resume 가지를 만들었다.
- 검수 후보 HEAD: f3c88b89155dc030e827abb2bf86c4463adb38d5.
- 블로그 미검증 WIP: f8397a0504bf1c19c2506db0b7186a750faf9e64. 앱 실행 전에 원복했다.
- 고정 C seed 사용은 B12 보충으로 허용됐다. seed·기존 시험·기대값·제한시간·실행기는 변경하지 않았다.
- ps 확인 후 지정 공용 flock 아래 순차 실행했다. 임시 자료·TMPDIR·프로필·산출물은 작업 전용 영역 <가림>에만 생성했다.
- native 빌드는 주소 공간 2097152KiB와 cgroup 2147483648바이트, swap 0, core 0을 함께 적용했다. ASan/Chrome 회귀는 큰 가상 주소 영역이 필요하므로 전체 작업의 cgroup 2147483648바이트·swap 0·core 0으로 제한했다. bootstrap은 strace 없이 실행했다.

## 첫 정식 빌드 실패 원문

기존 scripts/verify-e10-bootstrap.sh를 수정 없이 실행했다.

```text
STAGE0_C_BUILD=PASS
FORMAL_RUNNER_EXIT=1
FreeLang: vector allocation failed
ELAPSED=18.44 MAX_RSS_KB=1531188 TIME_EXIT=1
MEMORY_PEAK_BYTES=1570254848
SEED_SHA=f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c / f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c
```

seed→stage1에서 멈췄다. 고정 seed에는 예전 불변 배열 push 누적이 남아 있어 B11의 .fl 렉서 수정이 소급 적용되지 않는다.

## 후보 보완과 정식 빌드 원문

후보는 runtime/collection.c에 이전 논리 길이를 보존하는 append 전용 저장 공간을 넣고, runtime/aliases.c·runtime.h에서 고정 seed가 출력하는 vec_builder/freeze_builder 이름과 빈 구분자 str_split을 기존 API에 연결했다. runtime/http.c에는 시험 서버의 loopback 제한 환경 설정을 추가했다. 임시 불변 배열 분기·성장·소유 snapshot 진단은 보완 전 출력과 9/9 일치했다. 이 후보 수정은 회귀 실패 후 모두 원복했다.

같은 정식 실행기의 다음 출력은 후보에서 얻었다.

```text
BOOTSTRAP_TRACER_PID=0
STAGE0_C_BUILD=PASS
STAGE0_TO_STAGE1=PASS
STAGE1_C_BUILD=PASS
STAGE1_TO_STAGE2=PASS
STAGE2_C_BUILD=PASS
STAGE2_TO_STAGE3=PASS
CMP_2_3=PASS
STAGE3_C_BUILD=PASS
STAGE3_TO_STAGE4=PASS
CMP_3_4=PASS
ARRAY_LOOP=PASS
STAGE1_SHA=489d9d20afe9bcb40e0b0caec31f07aeee1674e3ab7710e7cae6ef999742da7b
STAGE2_SHA=0dd2b662803aed1bc3392ac855a73e8c1066c07ec3bb46fee81ffa13a505f1aa
STAGE3_SHA=0dd2b662803aed1bc3392ac855a73e8c1066c07ec3bb46fee81ffa13a505f1aa
STAGE4_SHA=0dd2b662803aed1bc3392ac855a73e8c1066c07ec3bb46fee81ffa13a505f1aa
BOOTSTRAP_STATIC_AUDIT=PASS
BOOTSTRAP_DYNAMIC_EXEC_COUNTS=UNOBSERVED
BOOTSTRAP_STRACE=0
RESIDUAL_PROCESSES=0
FORMAL_RUNNER_EXIT=0
ELAPSED=141.44 MAX_RSS_KB=685600 TIME_EXIT=0
MEMORY_PEAK_BYTES=708509696
SEED_SHA=f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c / f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c
```

생성 C 고정점은 B11의 0dd2b662와 일치했다. 별도로 기록한 후보 stage4 compiler ELF SHA는 87d6aabdeed318a9016344fc5754b1fcc15abd951fa2957b98a57b4691088bd1이다. ELF 해시로 고정점을 판정하지 않았다.

## 기존 전체 회귀: 중단 지점

기존 scripts/verify-e08-completion.sh와 하위 실행기를 수정 없이 실행했다. 정식 stage4 C로 만든 후보 컴파일러를 사용했다. 회귀 준비 첫 링크 실패는 systemd가 셸 배열을 선행 확장해 빈 인자로 바꾼 것이었다. 두 번째 준비에서 같은 누락을 확인해 중단했고, --expand-environment=no로 실행 환경만 바로잡았다. 이 준비 실패와 실제 시험 결과를 구분한다.

실제 회귀 원문:

```text
SMALL_LEAKS_PASS=2 TOTAL=2 MODE=after
WORKER_CLEANUP=3/3
CLI_LIFETIME=4/4
L01_BROWSER_TIMER_PASS=20 FAIL=0
L01_BROWSER_SEMANTICS_PASS=37 FAIL=0
L01_BROWSER_EVENT_PASS=24 FAIL=0
L01_BROWSER_DOM_PASS=29 FAIL=0
L01_BROWSER_BLOCK_MODEL_PASS=21 FAIL=0
L07_SELF_PASS=221
REMAINING_LEAK_CASES=12/12
L08_PLATFORM_PASS=5 FAIL=0
L01_COMPILER_REJECT_PASS=10 FAIL=0
BROWSER_TARGET_NATIVE_GATE_EXIT=1
NATIVE_ALL_EXIT=1
FULL_EXIT=1
ELAPSED=1372.95 MAX_RSS_KB=321732 TIME_EXIT=1
```

E08 최상위 묶음은 통과 2·실패 1이다. FULL 내부에서 기록된 관문은 통과 16·실패 1이며, 같은 실패의 상하위 전파를 서로 다른 정답 실패로 합산하지 않는다. native 후속군은 중단으로 미실행이다. 회귀 중 cgroup peak 관측값은 646295552바이트였고, 서비스 자동 수거 뒤 최종 peak는 수집하지 못했다.

실제 첫 실패 원문:

```text
Uncaught error (line 3): [browser-semantics] [browser-case] BROWSER_SEMANTICS_ASSERT J_UTF16_LENGTH (at line 32, col 0)
```

이 native 하위 실행기는 고정 seed로 1단계 컴파일러를 따로 만든다. 이 컴파일러의 생성 C에서 원본 `한🙂` 리터럴은 `ed 95 9c ed a0 bd ed b9 82`였다. 앞서 통과한 stage4 컴파일러 생성 C에서는 `ed 95 9c f0 9f 99 82`였다. 전자는 서로 분리된 surrogate의 UTF-8 바이트를 담고 있어 정답 J_UTF16_LENGTH를 깨뜨렸다. 생성 C의 리터럴 손상은 확인했다. 빈 구분자 UTF-16 분리 호환과 1단계 문자열 합성 경로의 결합은 후속 조사 대상이며, 어느 한 함수가 원인이라고 추가 확정하지 않았다. 기대값 3과 시험은 유지했다.

## 감사·원복·제출

실제로 수집한 회귀 execve trace를 기존 감사기로 검사한 원문은 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다. JS는 별도 프로세스 실행 집계이며 Chrome 안의 생성 JS 실행과 구분한다. strace 없이 수행한 bootstrap의 동적 실행 수는 UNOBSERVED이며 이 0에 포함하지 않는다. bootstrap 정적 감사의 범위는 seed와 생성 compiler C translation unit이다.

닫기 조건에 따라 추가 무거운 시험과 앱 인수를 진행하지 않았다. 런타임 4개 파일을 시작 소스와 동일하게 원복했다. 블로그의 목록·발행·예약·분류 WIP도 앱·web·tests·tools를 시작 소스와 동일하게 원복했다. WIP와 실패 증거는 이 가지의 이력 및 임시 자료 <가림>에 보존한다. 띄웠던 시험 서비스는 종료됐다. 운영 배포·운영 DB 접근은 없다.

최종 범위는 보고서다. 커밋 전 민감정보 diff를 검사한다. 이번 제출은 B12 지시대로 GitHub만 사용하며 Forgejo에는 푸시하지 않는다. 원격 HEAD와 CLEAN의 최종 확인은 제출 응답에 기록한다.

```text
B12=닫기
정식빌드=후보 stage1/2/3/4 완료; 생성 C 489d9d20 / 0dd2b662 / 0dd2b662 / 0dd2b662; 회귀 실패 후 후보 소스 원복
회귀=E08 최상위 통과 2 / 실패 1; native 후속군 미실행
B8_SUMMARY=2/4
PRD_PASS=0/24
API=미실행 / 20
바뀐_파일=docs/B12-BLOG.md
HEAD=f3c88b89155dc030e827abb2bf86c4463adb38d5 (실행한 후보; 원복 기준 cccfca9a354557dae0da29bf8d2d0862d6792214)
```
