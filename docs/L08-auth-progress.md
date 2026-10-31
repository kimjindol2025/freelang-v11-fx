# C13 인증 부분 구현 보고

## 확인한 것

Bearer 추출, password needs_rehash, CSPRNG hex token, 신규 scrypt password 생성, CSRF token/verify의 6개 공개 API를 native C에 연결했다. C12의 auth_sha256과 함께 기존 OpenSSL 의존성을 사용한다. 새 TS/JS/Python·Node·fl-build.sh는 사용하지 않았다.

`.fl`·정답과 RFC 기반 독립 fixture 소스를 먼저 커밋하고, native API 미연결로 `AUTH_BEFORE_BUILD_EXIT=1`임을 보존한 뒤 구현했다. Bearer의 nullish 우선순위·대소문자·빈 값, token 기본 32바이트·소수 버림·길이·중복 표본·잘못된 입력, rehash의 접두사 정책, 생성 salt/key 길이·새 salt, 한글·이모지·NUL password를 확인했다. 신규 password는 별도 C validator가 N=16384/r=8/p=1·salt16/key64·32 MiB maxmem으로 다시 계산해 검증했다. CSRF는 독립 HMAC fixture와 고정 clock으로 3600/3601초·미래·parseInt prefix를 검사하고, 실제 clock도 별도 검사했다. RNG 오류를 주입했을 때 token·password 생성이 실패함을 확인했다.

```bash
CPATH=<가림> LIBRARY_PATH=<가림> L08_AUTH_OUT=<가림> strace -f -e trace=execve -o <가림> bash scripts/verify-l08-auth-self.sh
```

```text
AUTH_BASICS_PASS
auth-basics_PROGRAM_EXIT=0
AUTH_CSRF_PASS
auth-csrf_PROGRAM_EXIT=0
AUTH_LIVE_CLOCK_PASS
auth-live_PROGRAM_EXIT=0
AUTH_RNG_FAILURE_PASS
auth-rng_PROGRAM_EXIT=0
PASSWORD_CREATE_FORMAT_AND_KDF=PASS
AUTH_PASSWORD_VERIFICATION=PENDING_USER_DECISION
L08_AUTH_PARTIAL_PASS=5 FAIL=0
AUTH_PARTIAL_SELF_EXIT=0
NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0
```

원문: [수정 전](evidence/l08-platform/auth-before.txt), [부분 자체 시험](evidence/l08-platform/auth-partial-after.txt), [strace 감사](evidence/l08-platform/auth-partial-trace-audit.txt). 최초 fixture build의 indentation warning은 문장을 나눠 보완했으며 정답·언어 구현은 변경하지 않았다. 원문 warning은 이력에서 지우지 않았다. 임시 난수 key·CSRF token·생성 password hash는 stdout·명령 인자·보고·Git에 출력하지 않는다. 공개 RFC 벡터 외 운영 계정·비밀번호·hash는 사용하지 않았다.

## 미확인 / 결정 대기

빈 scrypt 검증 key에 대한 원문 허용과 손상 거부 요구가 충돌해 사용자에게 질문한 상태다. 답변을 임의로 가정하지 않았다. **auth_verify_password는 아직 공개 연결하지 않았고 password verification fixture는 실행하지 않았다.** 이 부분을 stub으로 대체하거나 C13 전체 완료로 보고하지 않는다. 다른 인증 API·HTTP 작업은 계속 진행한다.

native random_token 상한은 1 MiB로 명시했다. source Node의 더 큰 모든 크기까지 같다고 주장하지 않는다. password 입력은 기존 UTF-16→UTF-8 replacement 계약을 따르고 KDF 임시 password/key 버퍼를 정리한다. auth 변경의 전체 회귀·bootstrap과 실제 blog1 로그인·권한·운영 데이터 호환은 아직 미확인이다. 현재 다른 checkout의 fd/JSON 전체 검증 결과를 이 auth 코드에 적용하지 않는다.

## 작업 가지 반영과 회귀 범위

부분 시험을 수행한 분리 checkout 구현 커밋은 `68e8628fccd7d113219d2461643c54fdef4f147f`다. 현재 작업 가지의 동일 소스 구현 커밋은 `24b2759a9814c1886296098c4fa97c4079e0aaa4`다. 반영 직후 runtime/self/scripts와 인증 시험 입력의 Git diff가 비어 있음을 확인했다(`AUTH_TESTED_SOURCE_EQ_BRANCH=PASS`).

전체 드라이버에 이 부분 자체 시험을 포함했다. 드라이버의 AUTH_PARTIAL 단계가 0이어도 **auth_verify_password는 미구현·미실행이며 C13 전체 완료가 아니다.** 이후 HTTP 구현과 운영 로그인 검증도 별도로 필요하다.

## 부분 인증 변경의 전체 회귀 확정

시험 대상 `6c53630309126d553c1eb60c67b2b8db3f4e7c21`의 전체 실행은 `AUTH_PARTIAL_PLATFORM_FULL_EXIT=0`으로 끝났다. 자체 `5`, fd basic `11`·extended `11`, JSON `13`, 인증 부분 `5`, L08 3차 `11`·2차 `11`·1차 `17`, HTTP `7`, L06 HTTP/TLS 각각 `110`, L07 `221` 통과다. 해당 묶음의 실패 수는 `0`이다. `BOOTSTRAP_EXIT=0`, `STAGE2_EQ_STAGE3=PASS`, `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`을 확인했다. stage2와 stage3 SHA는 `66d1ff3f63de632194c7a858cdf8150ecff0528d711d5eeb39adb2da09ecc3b7`이다.

명령은 `CPATH=<가림> LIBRARY_PATH=<가림> L08_PLATFORM_OUT=<가림> bash scripts/verify-l08-platform-all.sh`이며 하위 단계별 strace를 감사했다. [전체 출력 원문](evidence/l08-platform/auth-partial-full.txt). 이 실행에는 이후 C14 HTTP 변경이 포함되지 않는다. 비밀번호 검증 API·실제 블로그 로그인·앱 전체 실행은 여전히 미확인이다.

## D01 확정 뒤 실제 password 검증 재개

2026-10-31 사용자가 손상 값 거부를 선택했다. 정답 고정은 `9d71b9ca29509270be6b94abcdf6070892929666`, 구현은 `5859f61fd4e8fc310c2f021b5fedad1403de82d4`다. 기존 실행물에서 `fl_verify_password` 미연결 C 빌드 실패 `AUTH_VERIFY_BEFORE_EXIT=1`을 보존했다. 이후 RFC/legacy·비용 오류·손상/빈 key·Unicode/NUL·오답·잘못된 입력의 `AUTH_VERIFY_PASS=35 FAIL=0`, 기존 4개 인증 묶음+독립 생성 KDF 검증을 포함해 `L08_AUTH_SELF_PASS=6 FAIL=0`, `AUTH_VERIFY_SELF_EXIT=0`이다. 새 비용/입력 상한과 허용하는 Buffer 형식은 계약에 명시했다. 비교는 CRYPTO_memcmp이며 기존 OpenSSL만 사용한다.

[수정 전 원문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/blog-platform-completion/docs/evidence/l08-platform/auth-verify-before.txt), [수정 후 전체 자체 시험 원문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/blog-platform-completion/docs/evidence/l08-platform/auth-verify-self.txt). 감사는 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다. 정상 관리자 실제 화면·최신 전체 회귀/bootstrap은 별도 체크포인트이며 이 35개로 통과시키지 않는다. 운영 계정·자료는 사용하지 않았다. 사용자 중단 규칙에 따라 이번 연속 실행의 기대 실패 재현도 미통과 1회로 기록했다.

## Bootstrap 검증기의 필수 도구 누락 재현

후속 검증 호출에서 외부 strace와 검증기 내부 strace의 중첩으로 한 번 미통과했다. 외부 추적을 제거한 다음 실행은 PATH를 지나치게 제한해 rg가 없었다. stage2=stage3·array 결과가 맞고 프로세스 종료 0이어도 오염/금지 실행/잔여 과정 검사가 실행되지 않아 검증 통과로 인정하지 않았다. 실패 원문은 블로그 저장소의 store-adoption 증거에 보관한다. 사용자 3회 한도에 두 실행 모두 미통과로 포함했다.

새 작은 정답은 rg 없는 PATH에서 즉시 `REQUIRED_TOOL=FAIL:rg`, 종료 1로 끝나는 것이다. 빌드나 검증을 건너뛰어 성공을 표시하지 않아야 한다. 고정 stage0·bootstrap 기대 해시·컴파일/배열 시험·금지 실행 검사·동등성 기준은 변경하지 않는다. 원래 검증기는 내부 strace를 그대로 사용하고 호출자만 외부 중복 추적을 제거한다.

## 필수 도구 guard와 정상 bootstrap 확정

가드 정답 고정 `c8c3524` 뒤 수정 `7445dbd2ed71f3ecd1c51825b053006fe92540fe`에서 required tool을 먼저 확인하도록 보강했다. rg 없는 PATH의 자체 시험은 `BOOTSTRAP_REQUIRED_TOOLS=PASS`, 종료 `0`이다. stage0·bootstrap 기대 해시·검사 기준은 변경하지 않았다.

같은 소스의 원래 내부 strace와 필요한 rg 경로를 사용한 최종 전체 bootstrap은 `STORE_NATIVE_BOOTSTRAP_FINAL_EXIT=0`, `STAGE2_EQ_STAGE3=PASS`, `ARRAY_LOOP=PASS`, `RESIDUAL_PROCESSES=0`이다. stage2·stage3 SHA-256은 모두 `6f7c143734a8492fb6ae9ab77036f619c41915d0181a6f263e95985cf90ad423`이며 stage0는 기존 고정 `f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c`다. 오염·Node/npm/TS_JS·AFJ 접근 검사도 도구가 실제 존재하는 상태에서 실행했다. rg가 빠진 이전 종료 0을 통과로 사용하지 않았다. [성공/실패 원문·앱 DB 적용의 정확한 대상과 한계](https://github.com/kimjindol2025/blog1/blob/feat/blog-platform-completion/docs/native-store-adoption-progress.md).

이는 최신 전체 언어 자체 시험·L06/L07 등 전 회귀의 완료를 뜻하지 않는다. 그 전체 실행은 별도 미실행이다. 앱의 실제 로그인은 DB 변경 뒤 다시 화면 21/0·HTTP 8/0·독립 reader 2/0으로 통과했고 비밀번호/세션 값·운영 자료는 사용/기록하지 않았다.
