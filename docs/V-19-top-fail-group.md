# V-19 최대 실패 묶음 원인 진단

- RUN-ID: `RUN-20261104-24`
- 기준 FX HEAD: `2002508cecfbac7836d8de4ee9cfb971178d7a25`
- 작업 가지: `fix/v19-top-group`
- 근거 보고서: `docs/V-18-fail-evidence.md`

## 묶음 순위 및 선택

V-18의 timeout이 아닌 실패 28건은 다음 순서다.

| 순위 | 오류 묶음 | 건수 |
|---:|---|---:|
| 1 | native gate의 `.js` 입력 미지원 | 14 |
| 2 | 런타임 함수 없음 (`buf-to-binary`, `buf-str`, `file_mtime`, `sleep_ms`) | 8 |
| 3 | 실행 환경에 Node 없음 | 3 |
| 4 | oversized WAL 거부 기대 불일치 | 2 |
| 5 | 성공 marker 비교 실패 | 1 |

가장 큰 묶음인 14건의 `.js` 입력 미지원을 골랐다. 대표 사례는 case 24 `TCP concurrency`다. V-18 원문 진단은 다음과 같다.

```text
[IR-ERR] literal bad type=unknown | kind=literal line=138
```

case 목록은 `24, 25, 35, 56, 60, 61, 62, 63, 64, 66, 67, 68, 69, 71`이다. 이들은 `.js` 입력이다.

## 원인 위치

AFJ DB95 `tests/native-release-gate.sh:38-42`의 `.js` 분기는 컴파일러를 호출하고 컴파일 성공 여부와 상관없이 다음 메시지를 내보낸 뒤 종료 코드 1로 끝난다.

```bash
*.js)
  "$compiler" "$input" "$case_dir/case.c" || true
  echo 'NATIVE_JS_SOURCE_NOT_PORTED'
  exit 1
  ;;
```

FX 쪽 `self/cgc-main.fl:2557-2563`은 FreeLang IR literal type이 허용 집합에 없으면 `literal bad type=...` 오류를 낸다. 대표 case의 JS 파일을 이 FreeLang 컴파일러에 입력한 진단은 이 동작과 일치한다. 그러나 컴파일러의 해당 진단만 수정해도 AFJ runner가 명시적으로 `exit 1`을 실행하므로 이 14건은 통과로 바뀌지 않는다.

## 범위 판정 및 중단 사유

확인된 직접 원인은 FX 제품 소스가 아니라 별도 AFJ DB95 테스트 runner의 JS 미지원 분기다. 요청된 작업 가지는 FX 저장소이며, Node 사용은 금지되어 있다. 현재 제약 안에서 가능한 변경은 다음 둘뿐인데 둘 다 이 지시의 목표를 정직하게 달성하지 못한다.

- FX compiler의 unknown literal 진단을 바꾸어도 `.js` 입력은 runner의 명시적 종료 코드 1 때문에 계속 실패한다.
- runner의 실패를 SKIP 또는 PASS로 바꾸면 시험 수행 여부와 gate 결과 의미가 바뀐다.

따라서 source diff와 재시험은 하지 않았다. 71건 gate도 실행하지 않았다. 결과 기대값이나 timeout도 건드리지 않았다. 최소 수정은 AFJ DB95 runner가 해당 JavaScript 시험을 실제로 실행할 수 있도록 하는 것이지만, 이는 현재 FX 가지 밖의 작업이며 실행을 위해 Node가 필요하다. 이 범위 변경이 승인되기 전에는 이 묶음을 수정했다고 보고할 수 없다.

## 기준표

| 기준 | 결과 |
|---|---|
| 1/4: 원인을 소스 줄과 원문으로 특정 | PASS — AFJ `tests/native-release-gate.sh:38-42`, 대표 case 원문 기록 |
| 2/4: 원인에만 한정된 diff / compiler fixpoint | 미완료 — 적절한 수정 대상이 FX 저장소에 없어 변경하지 않음 |
| 3/4: 묶음 재시험 및 다른 대표군 회귀 확인 | 미완료 — 유효한 수정 없이 시험하지 않음 |
| 4/4: 두 원격 push 및 HEAD 일치 | 미완료 — 사용자 범위 결정 전 push 보류 |


.js 14건 묶음은 이번 FX 수정 범위 밖, 별도 지시서 대기.

V19_SUMMARY=1/4
