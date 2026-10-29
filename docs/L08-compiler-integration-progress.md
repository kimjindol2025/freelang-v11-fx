# 전체 블로그 컴파일에서 드러난 언어 문제 보완

## 확인한 것

C18 변경의 전체 회귀와 bootstrap은 원래 체크아웃에서 계속 실행했다. 원래 소스에 영향을 주지 않도록 별도 체크아웃에서 C01·C02의 추가 문제를 수정했다. 앱·vendor 수정이나 운영 데이터 변경은 하지 않았다.

- Map 블록과 try/throw 본문에서 lambda가 외부 변수를 캡처하도록 수집기를 보완했다.
- scalar/vector catch 변수를 바인딩하고 여러 handler 표현식을 평가한다. finally만 있는 try도 인식한다.
- 정상 완료·본문 오류·handler 오류 후 finally를 실행하고, 처리되지 않은 오류를 상위로 전달한다. setjmp 이후 사용하는 결과·미처리 오류는 volatile로 보존한다.
- `.fl throw`는 v11처럼 Error를 만들며 catch에는 message/type/raw를 제공한다. native Error와 일반 map의 차이는 내부 object brand로 구분하고 atom heap 복사에서도 brand를 보존한다. native Error의 code/hint 등 기존 필드도 보존한다.
- v11 evalThrow/evalTryBlock을 근거로 정답 정정 이력을 남겼다. 기존 메시지 검사 입력은 error_message를 사용한다. 정답을 현재 출력에 맞춘 변경은 아니다.

## 실제 결과

| 시험 | 원문 |
|---|---|
| 최초 전체 앱 최소 재현 | `COMPILER_BLOCKERS_BEFORE_EXIT=1` |
| throw 경로 정정 후, Error 포장 전 후보 | `CORRECTED_ORACLE_BEFORE_EXIT=1`, `expected=sample actual=null` |
| 수정 후 캡처·catch 재현 | `compiler-blockers_PROGRAM_EXIT=0`, 고정 출력 6개 일치 |
| 수정 후 중첩 오류·finally·범위·정정 메시지 | `try-scope_PROGRAM_EXIT=0`, 고정 출력 14개 일치 |
| 두 자체 시험의 strace | `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0` |
| 선택 회귀 contracts/source-edge/strict-fields/regex-guards | 각각 종료 코드 `0`, 고정 출력 일치 |
| 전체 앱 C 변환 | `BLOG_TRANSLATE_EXIT=0` |
| 전체 앱 C 구문 검사 | `BLOG_C_SYNTAX_EXIT=1`, `UNDECLARED_BINDINGS_OR_CATCH=0` |

최종 전체 앱 진단은 Error 포장 보완 직전의 캡처·catch 바인딩 후보에서 수집했다. 따라서 앱 통합 성공이나 최종 HEAD 빌드 통과의 증거로 사용하지 않는다. 진단 파일은 generated C의 긴 본문·HTML·경로를 싣지 않고 오류·경고 행을 발췌했다. 나머지 stdout/stderr는 시험 부산물로 보관한다.

## 미확인

- 새 구현의 전체 회귀·native bootstrap은 아직 결과가 없다. C18 원래 소스 검증과 혼합하지 않는다.
- 인증·HTTP·fd·JSON·타이머/TCP 등의 API가 남아 전체 앱 빌드는 아직 실패한다. L08 전체 완료가 아니다.
- PRD 필수 기능·실제 브라우저·데이터 호환·복구·성능 검증은 계속 필요하다.

근거와 정답 정정 이력은 L08-platform-contract.md, 수정 전후 원문은 docs/evidence/l08-platform에 보존한다. 새 외부 정규식 의존성은 없으며 기존 C 자체 엔진을 사용한다.
