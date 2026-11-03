# V-13 — 34건 gate 조건 대조

RUN-ID `RUN-20261103-21`

## 기준과 비교 표

| 항목 | DB95-10의 34건 실행 | V-12의 4건 실행 | 이번 V-13 시도 |
|---|---|---|---|
| FX commit | `b8fdb29b94eec109db245172237c2d5a55e4c001` (요청 기준) | `b8fdb29b94eec109db245172237c2d5a55e4c001` | `b8fdb29b94eec109db245172237c2d5a55e4c001` |
| AFJ commit | 보고서가 속한 commit `f67661bae549204ada8250848f9897eb07ec7a1b`; 보고서 안의 실제 gate 기준 checkout은 `b60320b86de044179f284d2e7aa537a111e6da0f`로 기록됨 | `85424a5d3dacb829f4de921a7767cccec558eae0` | `f67661bae549204ada8250848f9897eb07ec7a1b` tree |
| gate 명령 | 보고서에는 runner `tests/native-release-gate.sh`만 있고, 34건 실행의 전체 명령/환경값은 기록 없음. V-10 보충에 적힌 명령은 병합본으로 시작한 부분 실행의 명령 | `FX_ROOT=<가림> FX_NATIVE_STAGE0=<가림> NATIVE_GATE_LOG_DIR=<가림> CPATH=<가림> LIBRARY_PATH=<가림> bash tests/native-release-gate.sh` | 위와 같은 변수 구성을 사용. 실제 값은 모두 `<가림>` |
| Stage2 compiler 실행 파일 및 SHA-256 | 경로 `<가림>`, 실행 파일 SHA-256 기록 없음 | 경로 `<가림>`, `46b9feb958c66864bf8db3056fb1be8781abf630ceaec9e2cae98da140ed69` | V-12와 같은 compiler SHA-256 `46b9feb958c66864bf8db3056fb1be8781abf630ceaec9e2cae98da140ed69` 사용 |
| Stage2 생성 소스 SHA-256 | `6a0d4e384a18834d4f0cb03c10460aade5bf05f999f5db981dd5dc275d313e7d` | `6a0d4e384a18834d4f0cb03c10460aade5bf05f999f5db981dd5dc275d313e7d` | `6a0d4e384a18834d4f0cb03c10460aade5bf05f999f5db981dd5dc275d313e7d` |
| 경로 관련 환경 변수 | `CPATH`/`LIBRARY_PATH`가 사후 gate에서 보정됐다고 설명하지만 값·최초 34건 실행의 전체 환경은 기록 없음 | `FX_ROOT`, `FX_NATIVE_STAGE0`, `NATIVE_GATE_LOG_DIR`, `CPATH`, `LIBRARY_PATH`; 값은 `<가림>` | 같은 변수명 사용; 값은 `<가림>` |
| 작업 폴더 | `<가림>`; 보고서에 구체 경로 기록 없음 | `<가림>` | `<가림>` |
| 시작 load average | 기록 없음 | `5.35, 5.52, 5.10` | `4.84, 6.56, 7.89` |

DB95-10의 `f67661bae549204ada8250848f9897eb07ec7a1b` commit은 `docs/DB95-10-regress-row.md`만 바꾼 문서 commit이다. `f67661bae549204ada8250848f9897eb07ec7a1b`와 V-12 AFJ `85424a5d3dacb829f4de921a7767cccec558eae0` 사이 변경도 문서 2개뿐이며 `tests`, `src`, `scripts` 차이는 없었다. 따라서 이 두 AFJ commit 사이의 시험 코드 차이는 관찰되지 않았다.

Gate 제한은 기본 case당 `120`초 그대로였다. 기대값, 임계값, runner, 소스는 수정하지 않았다. 무거운 실행은 `flock -o <가림>` 아래 수행했다.

V-13 gate 직전 `ps`에는 다른 native gate/compiler가 없었다. load average는 `4.84, 6.56, 7.89`였고 available memory는 `165Gi`였다. 실행 중 별도 DB95 bootstrap이 같은 잠금에서 대기하는 것을 확인했으며, 그 작업은 건드리지 않았다. Gate가 잠금을 놓은 뒤 진행할 수 있도록 기다리게 했다.

## 부하 분리: case 1 단독 1회

case `syntax: tcp-server`를 Stage2 compiler SHA-256 `46b9feb958c66864bf8db3056fb1be8781abf630ceaecbc9e2cae98da140ed69`로 1회 실행했다. 120초 안에 끝나지 않았다. timeout wrapper의 반환 코드가 저장되기 전에 실행 중단/정리 요청이 들어와 standalone RC 원문은 기록하지 못했다. 확인 당시 compiler 프로세스는 elapsed `02:28`, CPU `99.4%`였고, 제 단독 실행에서 생긴 것으로 확인한 프로세스만 종료했다.

단독 실행 도중 별도 `python3` 고부하 프로세스가 나타나 CPU를 약 `540–589%` 사용했다. 이를 종료하거나 조사하지 않았다. 따라서 이 1회 측정은 무부하 조건을 충족하지 못하며 부하와 compiler 동작을 분리하지 못한다.

## 71건 재현 시도

AFJ tree는 요청 commit `f67661bae549204ada8250848f9897eb07ec7a1b`에서 준비했다. 사용자 요청에 따라 임시 자료를 정리하면서 gate 실행을 중단했고, case 18 실행 중 종료되어 71건 완료 결과는 없다. 아래는 중단 전에 gate가 출력한 원문 case 줄이다.

```text
NATIVE_CASE=1 status=FAIL exit=124 label=syntax: tcp-server
NATIVE_CASE=2 status=PASS exit=0 label=syntax: afj-db95
NATIVE_CASE=3 status=FAIL exit=1 label=raw binary boundary
NATIVE_CASE=4 status=FAIL exit=1 label=MariaDB packet codec
NATIVE_CASE=5 status=FAIL exit=1 label=MariaDB charset contract
NATIVE_CASE=6 status=FAIL exit=124 label=COLLATE contract
NATIVE_CASE=7 status=FAIL exit=1 label=page/WAL smoke
NATIVE_CASE=8 status=FAIL exit=124 label=metrics contract
NATIVE_CASE=9 status=FAIL exit=127 label=local canary rollback
NATIVE_CASE=10 status=FAIL exit=124 label=large corpus
NATIVE_CASE=11 status=FAIL exit=124 label=TCP catalog pages
NATIVE_CASE=12 status=FAIL exit=124 label=TCP lazy catalog eviction
NATIVE_CASE=13 status=FAIL exit=1 label=persistent recursive B+Tree
NATIVE_CASE=14 status=FAIL exit=124 label=bounded B+Tree range
NATIVE_CASE=15 status=FAIL exit=124 label=compound unique index
NATIVE_CASE=16 status=FAIL exit=124 label=compound predicate range
NATIVE_CASE=17 status=FAIL exit=124 label=schema MVCC merge
```

완료된 17개 사례에서 계산한 부분 집계는 PASS `1`, FAIL `16`, TIMEOUT `10`, SKIP `0`이다. 이는 gate의 전체 요약 줄이 아니며 TOTAL 71 결과로 취급하지 않는다. case 18은 제한시간 종료 여부를 포함해 결과가 기록되지 않았다.

## 판정

현재 원인은 확정하지 못했다. AFJ f676과 V-12 AFJ 85424 사이의 test/source/runner 차이는 없고, Stage2 생성 소스 SHA-256도 같지만, DB95-10의 compiler 실행 파일 SHA-256·전체 실행 환경·시작 부하가 기록되지 않았다. 이번 partial gate에서도 서로 다른 입력의 compiler timeout이 관찰됐고, 단독 case에는 별도 고부하가 겹쳤다. 따라서 부하 원인과 compiler 실행물/환경 차이를 구분할 증거가 부족하며, FX 병합 변경을 원인으로 지목할 근거도 없다.

34건 기준은 현재 문서 기록만으로 누구나 같은 조건에서 재현할 수 있다고 확인되지 않았다. DB95-10의 compiler 실행 파일 hash, 전체 실행 명령 및 환경, 시작 부하가 빠져 있고 이번 재현도 71건 완료 전 중단됐다.

## 언어 성과

raw_tcp 30초 송신 제한과 DB95 통과 수의 관계는 전체 재현 gate가 끝나지 않아 판정할 수 없다.

V13_SUMMARY=3/4
