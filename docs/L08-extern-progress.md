# L08 native C 확장 연결 회귀 보완

## 확인한 것

구현 소스 `796d6ac65ebcb6c25db09b9113eee47ca276a4f2`. 최소 `.fl`·정답·실제 C 구현과 계약을 먼저 `b7d318eaa24ec178676f23b445eeb6c7a62d17f6`에 고정하고, 수정 전 실행 실패를 `a9ff22242568c93298d4bcb8eed40587d43e0cb5`에 보존했다. [계약](../tests/l08-platform/extern-c-contract.md), [최소 실패](evidence/l08-platform/extern-before.txt).

이전 `09bf29622098199f1136a58df1915ed865419b6b` 전체 회귀는 `SEQUENCE_PLATFORM_FULL_EXIT=1`이다. 실제 C helper가 link돼 있지만 지연 함수 조회가 `fd-test-barrier`를 registry에서 찾아 실패했다. `L08_FD_EXTENDED_PASS=10 FAIL=2`, `CONCURRENT_LINES=0`이며 이 실행의 뒤쪽 회귀·bootstrap 완료를 주장하지 않는다. [전체 실패 원문](evidence/l08-platform/sequence-full-extern-before.txt).

컴파일러에 반복 가능한 prefix `--extern SOURCE_NAME`을 추가했다. ASCII 이름만 허용하며 해당 compiler 과정에서만 직접 C 호출로 처리한다. lexical binding은 우선하고, 선언하지 않은 함수의 동적 조회는 유지한다. 실제 prototype과 C 구현을 별도로 제공해야 한다. C 함수를 stub으로 만들거나 시험 이름을 언어에 하드코딩하지 않았다. 예:

```bash
<가림>/compiler --extern native-ext-value --extern native-ext-zero <가림>/input.fl <가림>/output.c
```

정상 구현을 link했을 때 `23 → 123`, `-7 → 93`, 인자 없는 함수 `17`의 3개 고정 출력이 통과했다. 선언 없는 compiler 별도 과정은 전후 모두 `Function not found: native-ext-value`, 종료 `1`이다. 잘못된 이름·누락된 이름은 compiler 종료 `1`, C 구현 누락은 link 종료 `1`이다. 기존 `--ast` 출력도 확인했다. 첫 sanitizer link는 검색 경로 누락으로 실패해 최초 전체 자체 driver 종료는 `1`이다. 이를 숨기지 않고 [환경 실패 원문](evidence/l08-platform/extern-sanitizer-config-before.txt)에 보존했다. `-L`에 시험용 native sanitizer 라이브러리를 명시한 별도 재실행은 `EXTERN_SANITIZER_BUILD_EXIT=0`, `EXTERN_SANITIZER_EXIT=0`, 같은 3개 출력·stderr `0`바이트다. `detect_leaks=0`이며 누수 검사는 미실행이다. [정상·오류·sanitizer 원문](evidence/l08-platform/extern-after.txt).

기존 fd driver에는 실제 C helper 두 이름의 선언만 추가했다. `fd-concurrent.fl`과 정답은 변경하지 않았다. `L08_FD_FAULT_PASS=8 FAIL=0`, `fd-concurrent_PROGRAM_EXIT=0`, `CONCURRENT_LINES=800`, `CONCURRENT_RECORDS=PASS`, `OUTSIDE_SENTINEL=PASS`, `L08_FD_EXTENDED_PASS=11 FAIL=0`, `EXTERN_FD_REGRESSION_EXIT=0`이다. 실제 8-thread/800-record 및 오류 주입·경계 검사를 유지했다. [fd 회귀 원문](evidence/l08-platform/extern-fd-after.txt).

시험 명령은 다음과 같으며 실행은 strace 아래에서 확인했다. 위 두 원문 각각 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다.

```bash
L08_EXTERN_COMPILER=<가림>/compiler L08_EXTERN_OUT=<가림> L08_EXTERN_SANITIZE=1 strace -f -e trace=execve -o <가림>/execve.log bash scripts/verify-l08-extern-self.sh
L08_FD_FAULT_OUT=<가림> strace -f -e trace=execve,clone,clone3 -o <가림>/execve.log bash scripts/verify-l08-fd-faults.sh
```

수정한 sanitizer 경로 설정을 포함하는 전체 자체 driver를 별도 새 root에서 재실행해 `L08_EXTERN_SELF=PASS`, `EXTERN_CONFIGURED_SELF_EXIT=0`, `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`을 확인했다. [driver 전체 재실행 원문](evidence/l08-platform/extern-configured-after.txt). 명령에는 `L08_EXTERN_SANITIZER_LIBDIR=<가림>`를 추가했다.

## 미확인

소스 `796d6ac65ebcb6c25db09b9113eee47ca276a4f2` 고정 checkout에서 `scripts/verify-l08-platform-all.sh`를 실행 중이다. 이후 C header를 주입하는 모든 빌드 호출부를 조사해 L06·L07의 실제 C helper도 같은 명시 선언이 필요한 것을 확인했다. `cc7d112078a5fd29c8564f06993294eabcd6d5b7`에서 두 빌드 명령에만 선언을 추가했다. 원래 `.fl`·C helper·정답은 유지했으며 실제 multipart HTTP/TLS와 L07 자체 회귀를 별도로 실행 중이다. 실행 중인 고정 전체 checkout은 이 두 빌드 명령 변경을 포함하지 않으므로 최신 작업 가지 전체 통과 증거로 사용할 수 없다. 기존 회귀·L06·L07·native bootstrap 최종 결과가 나올 때까지 최신 전체 통과로 집계하지 않는다.

C20 atom 원자적 갱신·DB 동시 거래·누수, 비밀번호 검증, 브라우저 J안·PRD 필수 기능·운영 자료 호환·복구·부하 검증은 남아 있다. 서버 23·25 접속·운영 자료 변경·합치기·배포는 하지 않았다.
