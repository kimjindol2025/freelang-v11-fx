# V-21 Node-free 검사 결과

기준 갈래 `fix/v20-buffer`의 `7e3e776d72f876578e85f3de77622df814882299`에서 `chore/v21-node-free`를 만들었다. 검사 스크립트는 Bash로 작성했고 `bash -n` 구문 검사를 통과했다. 스크립트와 결과표 외에는 저장소 파일을 수정하지 않았다.

## 검사 방식과 결과

- 글자 검색은 `scripts/`, `tests/`, `self/`, `runtime/`, `Makefile`에서 `node `, `npm`, `npx`, `.js`, `.ts`, `fl-cli`, `package.json`을 찾았다. FX 저장소에 Makefile은 없었다. 알려진 외부 AFJ 사례를 확인하기 위해 `AFJ_DB95_ROOT`가 가리키는 저장소의 `scripts/`, `tests/`도 포함했다.
- FX 저장소에 `install.sh`가 없어 설치 실행은 건너뛰었다. PATH에서 node/npm/npx 등을 제외한 채 `tests/run-tests.sh`를 실행했고 결과는 `10 PASS / 0 FAIL / 10 총`, 종료 코드 `0`이었다.
- 같은 단위 시험 실행을 `strace -f -e trace=execve`로 추적했다. 해당 실행에서 node/npm/npx/python 실행은 관찰되지 않았다. 실행 당시 시스템 ps에는 시험과 무관한 기존 프로세스도 보였지만, 추적 대상은 PATH 제한 아래 시작한 단위 시험뿐이다.
- `NODE_FREE=293`은 검사 대상 파일에서 나온 고유한 글자 검색 행 수다. 실행·추적 행은 별도 표기하며 이 숫자에 포함하지 않는다. 이는 실행된 의존 프로세스 수가 아니다.

## 알려진 사례와 재현성

`tests/native-release-gate.sh:38-42`의 `*.js)` 분기가 탐지됐고, `tests/release-gate.sh`의 `.js` 입력은 `14`건으로 탐지됐다. 최종 두 차례 실행 모두 `NODE_FREE=293`, `KNOWN_GATE_FOUND=1`, `KNOWN_JS_CASES=14`, `PATH_UNIT_RC=0`이었다. 생성된 표도 두 실행에서 같았으며 SHA-256은 `fc9d69416b0e1ba535a177efec595a78c364a59550000f985fed35a6f2582a3a`였다.

세부 행과 대체 제안은 [node-free-table.md](node-free-table.md)에 있다.

언어 성과: 검사한 범위에서 문자열 의존 후보는 `293`개이며, PATH 제한 실행과 추적에서 새로 실행된 node·npm·npx·python 프로세스는 `0`개다.

V21_SUMMARY=4/4, NODE_FREE=293
