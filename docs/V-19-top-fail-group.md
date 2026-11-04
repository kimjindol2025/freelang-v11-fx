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


## 보충 1: 남은 묶음 조사

시간 초과 39건과 `.js` 14건을 제외한 V-18 묶음 순위는 다음과 같다.

| 순위 | 묶음 | 건수 |
|---:|---|---:|
| 1 | 누락된 runtime 함수 호출 | 8 |
| 2 | Node 실행 파일이 필요한 shell 시험 | 3 |
| 3 | oversized WAL이 거부되지 않음 | 2 |
| 4 | gate `PASS` marker 비교 실패 | 1 |

최다인 8건 묶음을 골랐다. 구성은 case 3 `buf-to-binary` 1건, case 4·5·65·70 `buf-str` 4건, case 26·27 `file_mtime` 2건, case 50 `sleep_ms` 1건이다.

대표 case 3의 V-18 오류 원문:

```text
Uncaught error: [buf-to-binary] Function not found: buf-to-binary (at line 2, col 0)
```

AFJ `tests/binary-raw.fls:1-5`는 `AP+AQUI=`가 raw bytes로 왕복되어야 한다고 명시하고, 줄 2에서 `buf-to-binary`를 호출한다. AFJ `src/mariadb-wire.fls:5-11`은 binary buffer 계약을 사용하며 `buf-str`로 문자열을 인코딩한다. 그 모듈은 추가로 `buf-u8`, `buf-concat`, `buf-len`, `buf-read-str`, `buf-read-u8`, `buf-slice`, `buf-from-binary`를 호출한다.

FX `self/cgc-main.fl:2792-2798`은 별도 lowering이 없는 호출을 `fl_v11_builtin`으로 넘긴다. 이 native lookup에는 위 `buf-*` 이름이 없다. 기존 `fl_base64_decode`는 `runtime/aliases.c:1830-1843`에서 decode 결과를 `fl_str_val(out)`로 만들기 때문에 NUL 이후 bytes를 표현하지 못한다. 단순히 `buf-to-binary`를 `fl_base64_decode` 별칭으로 바꾸는 것만으로 raw binary case의 기대 동작을 만족시킬 수 없다. 필요한 buffer API 묶음은 FX에 현재 없는 binary-safe 변환·결합·slice·read 동작을 제공해야 한다.

이 묶음은 test runner 문제만으로 설명되지 않으며, 8건을 통과시키려면 새 binary-safe runtime API 또는 그에 준하는 API 확장이 필요하다. 지시서의 중단 조건에 따라 수정 및 재시험 전에 멈췄다. 묶음 case 재실행과 다른 묶음 대표 재실행은 하지 않았고, 71건 전체 gate도 실행하지 않았다.

다음 세 묶음도 FX 소스가 아니라 시험/대상 범위 밖으로 확인했다. Node 3건은 test shell의 외부 실행 의존성이다. oversized WAL 2건은 AFJ `src/afj-db95.fls:438-455`의 자체 smoke check이며, FX 저장소에는 그 WAL parser/assertion 구현이 없다. PASS marker 1건은 AFJ native runner의 출력 검사다. 이들은 FX 가지에서 고치지 않았다.

`.js` 진단 초안은 `1b2a0b25502328b691c7cd6ba3a1fc61f0b2e2a5`에 커밋했다. 보충 조사와 API 중단 사실은 이 보고서의 후속 기록이다. API 범위 승인 전에는 소스 diff나 remote push를 만들지 않았다.

## 현재 기준표

| 기준 | 결과 |
|---|---|
| 1/4: `.js` 진단 커밋 및 남은 묶음의 원인 근거 | PARTIAL — `.js` 진단 커밋 완료, 최다 runtime API 묶음의 원인·중단 사유 기록 |
| 2/4: 원인 한정 diff / compiler fixpoint | 보류 — binary-safe API 확장 승인 필요 |
| 3/4: 대상 묶음 통과 및 다른 대표 회귀 | 미실행 — API 승인 전 중단 |
| 4/4: Forgejo→GitHub push와 HEAD 일치 | 보류 — 최종 보고 및 소스 변경 전 |

현재 묶음 기준 통과 증가는 `0`건이며, V-18 전체 gate 기준 통과 수 `4`건은 변경되지 않았다.

V19_SUMMARY=1/4
