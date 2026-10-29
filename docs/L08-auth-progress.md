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
