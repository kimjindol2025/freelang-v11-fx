# V-23 node 검색 결과 분류와 제거 순서

## 기타 184건 재분류

V-21 표의 검색 범위와 원본 293행을 그대로 유지하고 분류 규칙만 세분화했다. 실행 명령과 시험 입력, 문서·주석, 빌드 설정은 기존 판정 순서를 유지했다. 나머지는 경로 세그먼트로 보관본·생성물을 먼저 찾고, 검색어가 파일 확장자 문자열인지 식별자·문자열인지 분리했다.

| 분류 | 건수 | 판정 기준 |
|---|---:|---|
| 실행 경로 | 78 | 셸 스크립트에서 런타임을 명령으로 호출 |
| 시험 입력 | 4 | `.js` 시험 파일 내부의 검색 행 |
| 빌드 설정 | 0 | Makefile 또는 package.json 설정 파일 행 |
| 문서·주석 | 27 | 문서 파일 또는 주석으로 시작하는 행 |
| 보관본 경로 | 0 | `archive`, `archived`, `legacy`, `backup`, `old`, `deprecated`, `history` 경로 세그먼트 |
| 생성물 경로 | 0 | `generated`, `build`, `dist`, `artifacts`, `vendor`, `coverage` 경로 세그먼트 |
| 확장자 문자열 | 74 | 실행이 아닌 `.js`·`.ts` 문자열 검색 행 |
| 식별자·문자열 | 110 | 실행이 아닌 `node `, `npm`, `npx` 검색 행 |
| 기타 | 0 | 위 규칙으로 분류되지 않은 행 |
| 합계 | 293 | 모든 행을 한 번씩만 분류 |

보관본·생성물 분류는 경로 조각이 아니라 전체 경로의 폴더 세그먼트와 일치할 때만 적용한다. 따라서 코드나 문자열 안에 우연히 `old` 같은 철자가 들어간 행은 보관본으로 오인하지 않는다. 실제 파일 시스템의 보관 여부나 생성 여부를 추가로 확인한 결과는 아니며, 현재 검색 표에서 일치한 행은 각각 0건이다.

## 실행 경로 78건 제거 순서

파일별 행 수 합계는 `1 + 2 + 3 + 4 + 68 = 78`이다. 순서는 호출 수가 적고 FreeLang 입력을 바로 native 실행으로 옮기기 쉬운 항목부터, 광범위한 회귀 게이트와 JavaScript 입력 이관이 필요한 항목 순으로 정했다. 이 순서는 제안이며 이번 회차에서는 실행 코드를 변경하지 않았다.

| 순서 | 파일 | 줄 수 | 하는 일 | 대체 방법 |
|---:|---|---:|---|---|
| 1 | `AFJ_DB95/tests/production-tls-boundary.sh` | 1 | TLS 경계 시험용 FreeLang 서버 기동 | 프리랭으로 바꾸기 |
| 2 | `AFJ_DB95/tests/fault-injection-restart.sh` | 2 | 강제 종료 뒤 WAL 복구 시험과 결과 확인 | 프리랭으로 바꾸기 |
| 3 | `AFJ_DB95/tests/silent-fix-three-pass.sh` | 3 | 세 복구 회귀를 반복 실행 | 프리랭으로 바꾸기 |
| 4 | `AFJ_DB95/tests/mariadb-afj-differential-runner.sh` | 4 | FreeLang DB와 MariaDB 실행 결과 비교 | 프리랭으로 바꾸기 |
| 5 | `AFJ_DB95/tests/release-gate.sh` | 68 | 전체 기능 회귀. 54개 FreeLang 입력과 14개 `.js` 입력 호출 포함 | 막힘: `.js` 시험 이관 범위와 전체 게이트 전환을 별도 설계해야 함 |
| | **합계** | **78** | | |

## 재확인

기본 모드에서 `NODE_FREE=293`을 유지했고 결과표 SHA-256은 `fc9d69416b0e1ba535a177efec595a78c364a59550000f985fed35a6f2582a3a`였다. `--exclude-docs` 분류 출력은 `CLASS_EXECUTION=78`, `CLASS_INPUT=4`, `CLASS_BUILD=0`, `CLASS_DOCS=27`, `CLASS_ARCHIVE=0`, `CLASS_GENERATED=0`, `CLASS_EXTENSION_TEXT=74`, `CLASS_IDENTIFIER_STRING=110`, `CLASS_OTHER=0`, `CLASS_SUM=293`, `NODE_REAL=78`이었다. 기본 실행은 `NODE_FREE=293`, `KNOWN_GATE_FOUND=1`, `KNOWN_JS_CASES=14`, `PATH_UNIT_RC=0`을 출력했다.

V23_SUMMARY=2/3, NODE_REAL=78, OTHER=0
