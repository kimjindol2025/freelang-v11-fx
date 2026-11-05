# V-24 실행 경로 부분 이관

## 이어받기 재확인

| 대상 | 시작 상태 / HEAD | 미커밋 변경 |
|---|---|---|
| FX 로컬 chore/v21-node-free | aa949bfcb378521b261002aaa403817716369624, commit 확인 | 없음 |
| Forgejo chore/v21-node-free | aa949bfcb378521b261002aaa403817716369624 | 원격 V-24 가지 없음 |
| GitHub chore/v21-node-free | aa949bfcb378521b261002aaa403817716369624 | 원격 V-24 가지 없음 |
| AFJ 기존 V-24 복제본 | 438bc61, chore/v24-fl-gate | 없음, 원본 작업 트리 보존 |
| AFJ 별도 기존 복제본 | review/db95-191-native-fixes | 미추적 문서 3개와 tmp 보존 |

FX는 aa949bfc에서 chore/v24-fl-gate를 만들었다. AFJ 검증은 별도 깨끗한 worktree에서 수행했고, 전달 변경은 기존 AFJ V-24 HEAD의 별도 로컬 복제본에서 같은 이름의 가지에 커밋했다. 기존 복제본의 원격 주소와 작업 트리를 바꾸지 않았다. ERP 등 금지 저장소에는 명령을 실행하지 않았다.

## 완료된 호출 3건

AFJ 커밋 dc00fcc, 659805a, 769022f가 호출을 각각 하나씩 바꾼다. 실행 준비와 자세한 시험 결과는 AFJ 커밋 035d430의 docs/V24-native-invocations.md에 기록했다. AFJ native 실행 파일은 기존 FreeLang 입력을 FX aa949bfc 런타임과 기존 Stage3로 빌드했다. Stage0·컴파일러·런타임·러너·판정·기대값·제한시간은 바꾸지 않았다.

| 항목 | 원본의 기대 결과 | native 실행 | 전후 실측 동등성 |
|---|---|---|---|
| production TLS boundary | TLS 없는 외부 bind 거부, PASS | rc=0, PASS | 이전 Node 실측 없음 |
| forced-kill writer | READY 뒤 실제 강제 종료 | 복구 시험 전체 rc=0, PASS | 이전 Node 실측 없음 |
| forced-kill recovery | committed 행만 복구, PASS | rc=0, PASS | 이전 Node 실측 없음 |

첫 writer 커밋은 두 호출을 native로 바꾼 임시 사본으로 전체 계약을 확인했다. 두 번째 커밋은 변경된 정식 스크립트로 확인했다. writer는 stdout 파일 리다이렉션에서도 READY를 전달하도록 호출에 stdbuf -oL을 사용했다. 옵션 없이 두 번 실패한 로그 및 exec 추적은 별도 임시 검증 폴더에 보존했다. 옵션 적용 뒤 정식 시험이 PASS했다.

curl 개발 헤더가 기본 탐색 경로에 없어 첫 native 링크는 실패했다. 기존 임시 보관본의 curl 8.9.1 헤더와 설치된 libcurl.so.4에 대한 임시 링크를 빌드 환경에서만 지정해 해결했다. 시스템 파일이나 저장소 런타임 소스는 바꾸지 않았다.

기존 Stage3 SHA-256: bb6b1a355e4e47360607a648b9d13ab24f43d056d5f58d87ec78fa8f657510f3.

## 진행을 막는 실제 시험 실패

| 기존 FreeLang 시험 | native 결과 | 관찰 |
|---|---|---|
| silent-batch-index | rc=1, INDEX_MISMATCH | commit=true, index=0, scan=1, hash 불일치 |
| silent-wal-recovery | rc=1, oversized WAL was accepted | oversized WAL 거부 계약 실패 |

실패한 호출은 변경하지 않았다. 위 두 결과는 기존 native 빌드·실행으로 재현했으며 기대값이나 판정을 바꿔 통과시키지 않았다. 이 원인 해결은 호출만 변경하는 현재 범위 밖이다. 남은 허용 호출과 gate 호출의 이관은 완료하지 않았다. JavaScript 14건도 그대로다.

## 같은 범위로 재계수

AFJ_DB95_ROOT에 전달용 AFJ 복제본을 지정하고 기존 scripts/verify-node-free.sh --exclude-docs를 실행했다. 검사 스크립트는 변경하지 않았다.

| 지표 | 시작 | 현재 |
|---|---:|---:|
| NODE_FREE | 293 | 290 |
| CLASS_EXECUTION / NODE_REAL | 78 | 75 |
| CLASS_BUILD | 0 | 0 |
| CLASS_OTHER | 0 | 0 |
| KNOWN_GATE_FOUND | 1 | 1 |
| KNOWN_JS_CASES | 14 | 14 |
| PATH_UNIT_RC | 0 | 0 |
| 단위 시험 | 10/10 | 10/10 |

node/npm/npx/python 등을 제외한 PATH에서 단위 시험 10 PASS / 0 FAIL을 유지했다. strace의 execve 기록에서 금지 런타임 실행을 찾지 못했다. 생성된 탐지표도 현재 결과로 갱신했다. FX와 AFJ를 함께 검사한 결과이며 FX 변경만으로 75가 되는 것은 아니다.

허용 10건 중 3건 완료. gate의 FreeLang 호출 변경은 0건. NODE_REAL 60 이하, gate 전후 결과 동등성, 전체 허용 항목 통과를 달성하지 못했으므로 부분 완료다.

V24_SUMMARY=0/3, NODE_REAL=75
