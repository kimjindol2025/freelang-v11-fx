# L08 C20 부분 진행: atom 갱신과 정렬 문맥

## 확인한 것

atom 구현은 `471d9f4ef2286267519b0953637284ec6bf6e09c`, 정렬 문맥 구현은 `35a6ec853aaa535fdf284207045043ef7d4ebc37`다. 기존 앱·vendor·기대값·운영 자료를 변경하지 않았다. `.fl`·정답·C 시험 helper를 먼저 커밋하고 실패 원문을 고정한 뒤 runtime을 보완했다.

### atom: 콜백 한 번과 유실 갱신 방지

정답 근거는 v11 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4` src/eval-builtins.ts의 swap! 소스와 C20의 동시 갱신 요구사항이다. Node로 실행하지 않았다. [계약·예외·중첩 의미](../tests/l08-platform/atom-update-contract.md).

| 순서 | 커밋 | 증거 |
|---|---|---|
| `.fl`·정답 먼저 | `6a03bfc1de969b2b561feded720d32107184a5bf` | 8 실제 thread ×100회 증가, callback 횟수 800, map·중첩·오류·독립 atom 진행 |
| 실패 고정 | `d3b27f86c2d956fa88865e627fee193657c45a8d` | 기대 `800`, 실제 `100`, `ATOM_UPDATE_BEFORE_EXIT=1` |
| 별칭 시험 먼저 | `e72fea861f40cfa1f3d49b8a326d47e069146dc2` | swap_bang의 별도 실제 8-thread 재현 |
| 별칭 실패 고정 | `9ce023665dd02879cfcc5161cb597867b5b0cd04` | 기대 `800`, 실제 `100`, `ATOM_ALIAS_BEFORE_EXIT=1` |

[일반 호출 실패](evidence/l08-platform/atom-update-before.txt), [별칭 실패](evidence/l08-platform/atom-alias-before.txt).

실제 atom에 재진입 가능한 독립 mutex를 보관하고 swap!의 읽기·callback 한 번·최종 저장을 같은 잠금으로 보호한다. callback의 같은 atom reset!/swap!과 오류 전 이미 발생한 callback 효과를 유지한다. callback throw를 포착해 잠금을 풀고 다시 전달한다. 별칭도 같은 갱신 경로로 연결했다. 이전 64개 버킷에 충돌하는 두 atom을 사용한 독립 진행 시험으로 전역/버킷 잠금을 callback 전체에 거는 해법을 거부한다. 일반 non-atom vector의 기존 내부 fallback 잠금은 별도로 유지한다.

일반 실행에서 본 시험의 고정 출력 `20`개와 별칭 출력 `3`개가 통과했다. [본 시험](evidence/l08-platform/atom-update-after.txt), [별칭](evidence/l08-platform/atom-alias-after.txt). 두 fixture를 함께 실행한 ASan/UBSan driver는 `L08_ATOM_UPDATE_PASS=2 FAIL=0`, `ATOM_SUITE_SANITIZER_EXIT=0`이다. 실제 `8`개 worker와 callback 호출 `800`회를 확인했으며 numeric/map 최종 값도 각각 `800`이다. [결합 sanitizer 원문](evidence/l08-platform/atom-suite-sanitized-after.txt). `detect_leaks=0`이며 누수 검사는 미실행이다.

### 정렬: 호출별 callback과 예외 cleanup

시험·정답 `e840cce0992a700bdc30e3fdc1fc7a2eaa1a58b1` 이후, 실패 원문을 `be561536307556b5536c5af58cf0c55a4ccf898f`에 고정했다. 중첩 정렬에서 기대 `[1,2,3,4]`, 실제 `[3,2,1,4]`, `SORT_CONTEXT_BEFORE_EXIT=1`이다. [실패](evidence/l08-platform/sort-context-before.txt), [v11 소스 계약·범위](../tests/l08-platform/sort-context-contract.md).

전역 callback 변수를 제거하고 기존 C 표준 라이브러리의 GNU qsort_r에 호출별 문맥을 전달한다. 새 외부 라이브러리를 추가하지 않았다. callback 예외는 comparator에서 포착하고, libc 정렬이 정상 반환해 내부 cleanup을 수행한 뒤 임시 배열을 해제하고 전파한다. 빈 배열의 NULL-source zero-length memcpy도 피했다.

중첩·입력 보존·중첩 예외 catch·외부 예외 뒤 회복·빈 배열·실제 `8`개 thread의 반대 방향 정렬 `20`회가 일반 실행과 ASan/UBSan 모두 통과했다. 고정 출력은 `8`개다. 각각 `SORT_CONTEXT_NORMAL_EXIT=0`, `SORT_CONTEXT_SANITIZED_EXIT=0`, `L08_SORT_CONTEXT_PASS=1 FAIL=0`이다. [일반 원문](evidence/l08-platform/sort-context-normal-after.txt), [sanitizer 원문](evidence/l08-platform/sort-context-sanitized-after.txt). GNU/Linux native 대상의 기존 libc API를 사용하며 다른 OS의 qsort_r ABI는 검증하지 않았다.

### 저장소 재생 회귀

atom 구현 `471d9f4ef2286267519b0953637284ec6bf6e09c` 고정 checkout과 승인된 source-only blog 복사본에서 원래 writer 1회·별도 reader 10회를 ASan/UBSan으로 실행했다. 모두 종료 `0`, `DB95_NATIVE_STORE_PASS=11 FAIL=0`, `ATOM_STORE_SANITIZER_EXIT=0`, `DB95_NATIVE_WAL_READ_ONLY=PASS`다. 해당 합성 WAL SHA-256은 `3b9bd996b64ad4fae8e09269a0cdb756de8d46fa7a07539be03fd126190f5ebd`다. 원래 fixture·거래 기록을 빼거나 정답을 낮추지 않았다. 동시 writer 시험이나 운영 자료 검증으로 확대하지 않는다. [원문](evidence/l08-platform/atom-store-sanitized-after.txt).

위 시험의 execve 감사는 각각 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다. 사용 명령 형식은 다음과 같다. 실제 source-only 복사본·실행물·출력·라이브러리 위치는 `<가림>` 처리한다.

```bash
L08_ATOM_UPDATE_OUT=<가림> L08_ATOM_UPDATE_COMPILER=<가림>/compiler L08_ATOM_UPDATE_SANITIZER_LIBDIR=<가림> strace -f -e trace=execve,clone,clone3 -o <가림>/execve.log bash scripts/verify-l08-atom-update.sh
L08_SORT_CONTEXT_OUT=<가림> L08_SORT_CONTEXT_COMPILER=<가림>/compiler L08_SORT_CONTEXT_SANITIZER_LIBDIR=<가림> strace -f -e trace=execve,clone,clone3 -o <가림>/execve.log bash scripts/verify-l08-sort-context.sh
L08_BLOG_SOURCE_ROOT=<가림> L08_BLOG_NATIVE_COMPILER=<가림>/compiler L08_BLOG_STORE_OUT=<가림> L08_BLOG_STORE_SANITIZER_LIBDIR=<가림> strace -f -e trace=execve -o <가림>/execve.log bash scripts/verify-l08-blog-store-native.sh
```

## 미확인

- 두 구현을 포함한 최신 고정 `35a6ec853aaa535fdf284207045043ef7d4ebc37`에서 전체 회귀·L06/L07·native bootstrap을 실행 중이다. 이전 C 확장 고정 `fe6bb4f4f38775404685ccf96e69f8e147d3633c` 전체 실행도 종료 결과를 계속 관찰하며 이 두 소스를 혼합해 최신 통과라고 집계하지 않는다.
- 최신 전체 blog native 빌드·editor HTTP/재시작도 source-only 복사본에서 재검증 중이다. 합성 세션 시험이며 비밀번호 로그인·실제 편집기 조작 완료 증거가 아니다.
- HTTP 요청 전체의 상태 수명, DB 동시 거래·WAL 일관성·crash/복구·부하 목표 검증은 남아 있다. atom 컨테이너는 기존처럼 영구 수명이어서 임시 atom·RC cycle·누수 해결이 필요하다. 이번 mutex 추가로 이 문제를 해결했다고 주장하지 않는다. 여러 callback이 여러 atom을 순환 대기하는 사용자 lock 의존성을 자동 해결한다고 주장하지 않는다.
- sort-by 전체의 필드 이름 key·문자열 locale·동점 안정성·잘못된 인자 계약은 이 숫자/문맥 fixture의 검증 범위 밖이다.
- 비밀번호 검증 결정·브라우저 J안·이미지 처리·댓글·다중 작성자·나머지 PRD·운영 자료 호환·백업/복구·성능은 남아 있다. C20 전체·L08 전체·플랫폼 전체 완료가 아니다. 서버 23·25 접속·운영 자료 변경·합치기·배포는 하지 않았다.
