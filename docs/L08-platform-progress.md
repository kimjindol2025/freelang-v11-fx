# L08 플랫폼 통합 진행 보고

## 확인한 것

기준 fx `fd6ef97277451ec71e902afc794b9999c631c685`에서 `feat/blog-platform-completion` 가지를 만들었다. 기존 전체 앱의 C 변환 성공 후 네이티브 빌드 실패를 확인했다. 환경·인증·HTTP·fd 등 미연결 API와 클로저·catch 문법 문제가 있어 아직 앱 실행에 도달하지 못했다.

| 구분 | 커밋 SHA / 결과 |
|---|---|
| C18 `.fl`·정답 선행 고정 | `da71b207d218956b66df0be345f4d06be310fba1` |
| C18 구현·재현 가능한 검증 드라이버 | `d5f6c0145094ed32af77b81f8cf735fc22cafb97` |
| map 캡처·catch 후속 실패 시험 선행 고정 | `c3bb11e5bf3483fc24d2c619c2c9b3801f1bbfdb` |

C18은 v11의 환경 빈 값·기본값 평가, 오류 메시지 변환, 프로세스 종료, 암호학적 짧은 ID를 native dispatcher와 별칭에 연결했다. 기존 OpenSSL RAND_bytes를 사용하며 외부 의존성을 새로 추가하지 않았다. API 정답은 [소스 계약](L08-platform-contract.md)에 있다.

실행 명령(실제 경로는 가림):

```bash
CPATH=<가림> LIBRARY_PATH=<가림> L08_PLATFORM_OUT=<가림> bash scripts/verify-l08-platform-all.sh
```

전체 드라이버가 C18 자체 시험을 strace 아래 실행했다. 원문:

```text
environment_PROGRAM_EXIT=0
exit-default_PROGRAM_EXIT=0
exit-seven_PROGRAM_EXIT=7
L08_C18_PASS=3 FAIL=0
NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0
```

environment는 고정된 17개 출력과 일치하며 종료 시험은 종료 뒤 출력이 없음을 검사한다. strace 집계는 **완료한 C18 자체 단계**에 한정한다. 전체 회귀의 종료를 뜻하지 않는다. [수정 전 빌드 실패 원문](evidence/l08-platform/c18-before.txt), [수정 후 원문](evidence/l08-platform/c18-self.txt), [호출 감사](evidence/l08-platform/c18-trace-audit.txt).

전체 앱에서 발견한 map 내부 클로저 캡처·vector catch 문제도 새 최소 재현에서 `COMPILER_BLOCKERS_BEFORE_EXIT=1`로 확인했다. [실패 원문](evidence/l08-platform/compiler-blockers-before.txt). 아직 수정하지 않았다.

## 미확인

- 기존 L08 1~3차·L06·L07 회귀와 verify-native-bootstrap.sh는 같은 전체 드라이버에서 실행 중이며 최종 종료 코드·통과 결과는 아직 없다. 전체 검증 완료로 판정하지 않는다.
- C01·C02의 추가 컴파일 문제, C11~C17·C19~C20, blog1 전체 실행·실제 화면·데이터 이전·복구, PRD 필수 기능 완성은 남아 있다.
- 운영 데이터는 수정하지 않았고 서버 23·25 접속·합치기·배포는 하지 않았다. TS·수작업 JS·Python·fl-build.sh는 사용하지 않았다.
- 이번 C18 입력 범위 외 process_exit의 Node 전체 오류 코드 체계나 catch 객체·finally의 완전 호환을 주장하지 않는다. 기존 catch가 raw 값을 넘기는 차이는 별도 언어 시험으로 해결한다.

진행 상태는 잠정 보고다. 실행 중 검증 결과가 확정되면 원문·종료 코드·SHA를 추가한다.

## C18 원래 코드의 전체 검증 확정

위 ‘실행 중’은 최초 중간 보고 상태다. 동일 실행이 종료됐으며 원래 C18 소스 `d5f6c0145094ed32af77b81f8cf735fc22cafb97`의 전체 결과를 확정한다. 실행 중 runtime·self·검증 드라이버와 기존 회귀 입력은 변경하지 않았음을 Git diff로 확인했다. 별도 체크아웃의 후속 컴파일러 수정 결과와 구분한다.

```text
L08_C18_PASS=3 FAIL=0
L08_P3_PASS=11 FAIL=0
L08_P2_PASS=11 FAIL=0
L08_PASS=17 FAIL=0
L08_HTTP_PASS=7
L06_HTTP_PASS=110 FAIL=0
L06_TLS_PASS=110 FAIL=0
STAGE2_EQ_STAGE3=PASS
BOOTSTRAP_EXIT=0
NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0
L08_PLATFORM_CURRENT_GATES=PASS
PLATFORM_FULL_EXIT=0
```

[전체 결과 원문](evidence/l08-platform/c18-full.txt). bootstrap의 `STAGE0_EQ_CANONICAL=DIFFERENT`는 기존 검증 정책의 원문을 그대로 보존했다. STAGE2와 STAGE3의 SHA는 `0d776b758fb638e3aa96551a2f7bc1dcd3739e8ae77bfa9b674a6959b9064967`로 같다. 언어 검증 통과가 앱 전체 실행·필수 기능·운영 배포의 완료를 뜻하지 않는다.
