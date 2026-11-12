# B-8 브라우저 저장소 API: 구현과 부분 검증

2026-11-12 시작, 2026-11-13 정리. 사용자가 언어 API 구현을 승인했다. 제품의 영속 초안 저장 활성화와 D04 보관 정책은 보류한다. **개발용 native 컴파일러에서 Chrome 20/20을 확인했지만 정식 소스 빌드·전체 회귀·bootstrap은 미완료이므로 WIP다.**

최신 상태: 승인된 8 GiB 재개까지 종료 `139`로 끝나 세 번째 실패 중단 규칙을 발동했다. 아래 초기 미실행 설명은 재개 전 기록이며, 마지막 절에 승인 후 실행과 중단 결과를 구분했다.

## 출발점과 선행 커밋

- FX 기준: `02176817d44f4d998a3f0b8ac04cec4373e04a7f`.
- blog1 기준: `cd48c91947f09836ea781f46d66683d1ffcb9259`.
- 다른 DB 검수의 작업 트리는 수정하지 않았다. 별도 `feat/browser-storage-b8` 작업 트리를 만들고 CLEAN을 확인했다.
- FX의 고정 정답 20개 선커밋: `c20dc641b0fb067e343cba9647cd9c801bbf27b8`.
- 구현 WIP 커밋: `4a42b601b1038117d205a65331c6047c83c0fc0a`.

기준 브라우저 작업 가지와 원격 기본 가지에서 저장소 API 중복 구현이 없는 것을 확인했다. 정답은 블로그 준비본과 동일하며 기존 정답·문턱값·시간제한을 변경하지 않았다.

## 언어에서 추가된 것

`browser-storage-read`, `browser-storage-write!`, `browser-storage-remove!`를 연결했다. 인자 수·키·값 검사와 오류 분류는 `self/browser-core.fl`에 구현했고 C 프린터는 표준 localStorage 접근과 getItem/setItem/removeItem 호출만 출력한다. 생성 내장 데이터도 갱신하고 embedding 검사를 통과했다. 별도 JSON 해석기·자동 삭제·일괄 clear API는 만들지 않았다.

없는 키는 nil, 빈 문자열은 빈 문자열로 구분한다. SecurityError·QuotaExceededError·기타 저장소 실패를 각각 명시적 오류 문자열로 드러내며 키/본문/원래 host 예외 메시지는 노출하지 않는다. 계약은 `B8-browser-storage-contract.md`에 있다.

## 실제 Chrome 20개

고정 compiler의 수정 전 진단은 아래와 같다. 이는 기존 개발 컴파일러에서 새 API가 없음을 보인 증거이며 정식 소스 빌드 성공 증거가 아니다.

```text
FIXED_COMPILER_STORAGE_BEFORE_EXIT=1
Uncaught error (line 440): browser target: unsupported operation: browser-storage-read
```

seed 기반의 소스 빌드가 아래 메모리 제한 실행에서 종료돼, 기존 검증된 컴파일러 C 생성물을 현재 runtime과 다시 연결한 개발용 바이너리로 API를 검증했다. 이 개발용 C 생성물은 현재 native compiler .fl 원본의 재생성 결과가 아니다. seed나 source 파일을 이 생성물로 교체하지 않았다.

```text
DEVELOPMENT_COMPILER_RELINK=PASS
STORAGE_GENERATION_REPRODUCIBLE=PASS
BROWSER_STORAGE=20/20
```

실제 Chrome `--headless --dump-dom --screenshot --window-size=1280,1000`에서 고정 20개가 전부 PASS였다. 생성 JS를 손으로 고치지 않았고 두 번 생성해 byte 일치를 확인했다. 실제 localStorage를 사용하며 접근 거부·quota는 두 합성 키에서 표준 DOMException을 주입한다. 실제 저장소나 메모리를 가득 채우는 시험은 하지 않았다. 값/키/Unicode/다른 키 보존과 잘못된 인자·실패 노출·실패 전 값 보존을 확인했다.

처음 두 Chrome 실행에서도 20개 값은 PASS였지만 시험 서버의 stderr에 `UPLOAD_STORAGE_UNAVAILABLE`가 남아 실행기를 실패로 기록했다. 첫 준비에서 빠진 임시 폴더를 만든 것만으로 해결되지 않아, runtime 계약인 `TMPDIR=FL_FILE_BASE/tmp`까지 맞춘 세 번째 준비에서 stderr 검사도 통과했다. 진단을 허용 목록에 추가하거나 숨기지 않았다. 두 실패 자료는 따로 보존했다.

## 기존 정답의 부분 회귀

```text
BROWSER_REGRESSION_EXIT=2
semantics PASS=37 FAIL=0 MISSING=0
dom PASS=29 FAIL=0 MISSING=2
events PASS=24 FAIL=0 MISSING=2
timers PASS=20 FAIL=0 MISSING=0
http PASS=20 FAIL=0 MISSING=0
cancel PASS=20 FAIL=0 MISSING=0
BROWSER_REGRESSION=PARTIAL_CDP_OBSERVATIONS_UNEXECUTED
CLI_FIXTURES=4/4
NATIVE_SEMANTICS=37/37
COMPILER_REJECTIONS=10/10
NATIVE_JSON=13/13
NATIVE_PARTS_ONLY=PASS FULL_REGRESSION=UNEXECUTED
```

브라우저는 150개 통과·실패 0·기존 CDP 관측 4개 미실행이다. 기존 금지인 CDP 클라이언트 빌드를 하지 않았으며 HTML 관측을 그 4개 통과로 대체하지 않았다. 첫 회귀 준비는 앞선 서버의 FL_FILE_BASE 환경이 다음 compiler 실행에 남아 다음 입력 읽기에 실패했다. 준비에서 환경을 분리한 재실행만 위 결과로 기록했다.

native 보조 검수는 `tests/l01/browser-storage-native-parts.sh`로 실행했다. 기존 CLI 네 사례의 sanitizer·기대 종료·90초 한도·stdout/stderr 정답, native 의미 정답 37개, 기존 compiler 거부 10개, JSON 원래 입력/정답·WAL 10회 재생·30초 한도를 유지했다. 기존 전체 러너를 변경하거나 이 보조 검수를 전체 회귀로 표시하지 않았다. 처음에는 sanitizer 링크 대상이 없어 빌드가 실패했고, 기존 캐시 라이브러리를 다시 연결한 재실행에서 위 결과를 얻었다.

native 보조 검수의 GCC 빌드 execve 로그만 Bash로 감사한 원문은 `BUILD_TRACE_NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다. 이 수치는 전체 작업이나 bootstrap의 동적 실행 횟수가 아니다. Chrome 내부에서는 생성 JS와 시험 HTML의 브라우저 스크립트가 실행됐다. 기존 전체 러너의 awk 사용과 C CDP 클라이언트 빌드는 B-8 금지 범위여서 실행하지 않았다.

## 정식 빌드와 bootstrap의 미완료

| 실행 | 보호용 가상 메모리 상한 KiB | 최대 RSS kbytes | 실제 셸 종료 |
|---|---:|---:|---:|
| 구현 전 seed → 현재 compiler 소스 생성 | 2097152 | 1531808 | 139 |
| 구현 후 seed → 현재 compiler 소스 생성 | 4194304 | 3273036 | 139 |
| 구현 후 개발 compiler → 현재 compiler 소스 생성 | 4194304 | 3952140 | 139 |

메모리 보호 상한을 둔 실행에서 SIGSEGV가 나 로그를 보존했다. 원인 분석/정상 완료 전에 이를 성공이나 단순한 OOM 진단으로 바꾸지 않는다. 마지막 실행의 GNU time은 신호 종료와 `Exit status: 0`을 함께 기록했지만 실제 셸 종료는 `139`다. 개발용 재링크 성공을 정식 소스 빌드 통과로 초기화하지 않는다. 더 큰 메모리로 재시도하지 않았다.

구현 후 소스 생성 실패는 보수적으로 같은 관문의 `2`회로 관리한다. 다음 같은 관문의 실패가 세 번째면 즉시 중단한다. 전체 브라우저 회귀도 환경 준비 실패와 미실행 포함 종료 `2`를 보수적으로 `2`회 미통과로 관리하며, CDP 관측 해결 없이 반복하지 않는다. storage 20개 자체의 최종 실행은 통과했다.

정식 `verify-e10-bootstrap.sh`는 실행하지 않았고 stage2/3/4 생성 결과가 없다. 따라서 현재 해시는 UNOBSERVED다. 이전 J-1에 기록된 `d98af73ee3b1aa9439864ab10ef044ca21242c86bf4269b0beb094e5ad149039`는 과거 기준값이며 이번 결과로 복사하지 않는다.

seed C SHA-256은 원문 `f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c`다. `bootstrap/stage0.c`, `.sha256`, `self/cgc-main.fl`은 기준 커밋과 byte 동일하다. 기존 seed를 교체하지 않았고 native compiler .fl도 수정하지 않았다.

첫 제한 빌드 때 OS core 수집기가 잘린 `4M` 진단을 기본 위치에 남긴 것을 확인했다. 이후 `ulimit -c 0`을 지정했고 다음 seed 실패의 Storage는 `none`이었다. OS 수집기 위치는 바꾸지 않았으며 진단 경로는 `<가림>`이다. 작업 산출물과 실행 로그는 승인된 작업 임시 영역 `<가림>`에 보존했다.

## 출처·자원·다음 승인 범위

| 대상 | SHA-256 |
|---|---|
| 개발 compiler의 재사용 C 생성물 | `e697df04aa11dffc72dd53b48658c55b146df9ef63de9adba432980440b1b635` |
| 현재 runtime으로 재연결한 개발 compiler | `c0f92a0204debc0654756385fa639bfff20a75c64c46033e3e421043be35f34e` |
| 변경된 browser core | `80e6862372836d3923897625614cbbbed53fc3386155c317ecb4d9fc5a705d5a` |
| 동일 재생성 storage JS | `47c10adcfd6e4ab3e1642290cd5244abb2a600760b649942a1e207e56a7a648d` |

개발 재링크와 Chrome 생성의 최대 RSS는 `512340 kbytes`, storage Chrome은 `252196 kbytes`, native 보조 검수 묶음은 `506028 kbytes`였다. 값은 개별 GNU time 기록이며 전체 동시 프로세스 합계가 아니다. ps 확인 후 공통 flock 아래 순차 실행했다. 기존 개발 헤더·시스템 libcurl과 sanitizer 캐시를 사용했으며 새 라이브러리를 설치하지 않았다.

기존 러너·정답은 유지했다. bootstrap의 env -i 두 곳에 TMPDIR 전달을 추가하는 제안만 비공개 작업 영역에 준비했으며 실제 러너에는 적용하지 않았다. 정식 검수 재개에는 빌드 메모리 예산과 기존 전체 러너가 사용하는 awk/C CDP 검수 도구에 대한 범위 확인이 필요하다. 원격·주소·경로·키는 공개 보고에서 가렸다.

공개 증거는 `docs/evidence/b8-storage/`의 화면·20개 원문·부분 회귀·보조 native 결과·제한 빌드 resource 기록이다. source 빌드/전체 회귀/bootstrap을 닫기 전까지 WIP를 유지하고 제품 영속 저장은 켜지 않는다.

## 2026-11-13 승인된 8 GiB 재개와 중단

사용자가 8 GiB 상한의 추가 확인 한 번과 검수용 awk/C CDP 도구·환경 수정 예외를 승인했다. HEAD와 CLEAN, ps·메모리 상태를 확인했고 새 산출물은 같은 작업 임시 영역에 만들었다. `verify-e10-bootstrap.sh`의 env -i 두 곳에 TMPDIR 전달만 추가했다. seed·정답·판정·시간제한은 변경하지 않았다.

정식 bootstrap을 단일 공통 flock 아래 `ulimit -v 8388608`, `ulimit -c 0`으로 한 번 실행했다. strace는 사용하지 않았다. 시작 시 MemAvailable 원문은 `176739024 kB`였다.

```text
BOOTSTRAP_8G_EXIT=139
BOOTSTRAP_TRACER_PID=0
STAGE0_GENERATED_SOURCE_CONTAMINATION=0
STAGE0_SHA=f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c
STAGE0_EQ_CANONICAL=DIFFERENT
STAGE0_C_BUILD=PASS
STAGE0_TO_STAGE1=PASS
STAGE0_NATIVE_EXEC=PASS
STAGE1_C_BUILD=PASS
NATIVE_SOURCE_POST_FIX_FAILURES=3
STOP_RULE=TRIGGERED
```

stage1→stage2 생성에서 SIGSEGV·종료 `139`가 발생했다. 최대 RSS는 `7988188 kbytes`, 경과 시간은 `0:46.39`다. OS 수집기의 Storage는 `none`이었다. 원인을 확정하기 전에 이를 단순 OOM으로 판정하지 않는다.

```text
STAGE1_SHA=b4cdbc346b43708d9f218301e2c84ea49934edd3d688c1b0f5814c9680314448
STAGE2_SHA=UNOBSERVED
STAGE3_SHA=UNOBSERVED
STAGE4_SHA=UNOBSERVED
```

stage1은 관측한 중간 생성물이며 새 고정점이 아니다. stage2=3=4 비교·배열 실행·정적 감사에는 도달하지 못했다. 앞 단계의 부분 통과로 같은 소스 생성/정식 관문의 미통과 누계를 초기화하지 않았다. 세 번째 실패 중단 규칙을 발동했고 이후 코드 수정·재시험·전체 회귀·CDP 실행을 진행하지 않았다. 로그 보존·보고·제출만 수행한다.

승인 전의 개발용 Chrome 20/20과 부분 회귀는 해당 과거 실행 범위로 유지한다. 이번 결과로 정식 빌드나 bootstrap 통과를 선언하지 않는다. 예외 승인은 남아 있지만 추가 실행은 별도 사용자 지시 전까지 중단한다. 원문과 가림 처리된 stderr/resource는 `docs/evidence/b8-storage/retry8/`에 보존했다.
