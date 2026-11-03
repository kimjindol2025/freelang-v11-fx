# V-12 — 71건 gate 부하/코드 구분

RUN-ID `RUN-20261103-19`

## 실행 기준

- FX 기준본: `b8fdb29b94eec109db245172237c2d5a55e4c001`
- FX 작업 가지 기준: `fix/raw-tcp-review` HEAD `33e78a84d5139667292db280e024c3776bb4a77e`
- DB95: `native-run` HEAD `85424a5d3dacb829f4de921a7767cccec558eae0`
- 기준본 Stage2 생성 소스 SHA-256: `6a0d4e384a18834d4f0cb03c10460aade5bf05f999f5db981dd5dc275d313e7d`
- 사용 Stage2 실행 파일 SHA-256: `46b9feb958c66864bf8db3056fb1be8781abf630ceaecbc9e2cae98da140ed69`
- `FX_NATIVE_STAGE0` 환경 변수에 gate가 요구하는 Stage2 실행 파일을 전달했다. Stage0 실행 파일은 gate에 사용하지 않았다.
- 사용한 gate 명령은 `docs/DB95-10-regress-row.md`의 명령과 같으며 주소·경로는 가렸다.

```sh
FX_ROOT=<가림> FX_NATIVE_STAGE0=<가림> NATIVE_GATE_LOG_DIR=<가림> CPATH=<가림> LIBRARY_PATH=<가림> bash tests/native-release-gate.sh
```

Gate script의 기본 per-case 제한은 `120`초다. script 자체가 `TCP 5m performance envelope`에 `360`초를 지정하지만 이 사례는 `exit=1`로 끝났고 TIMEOUT으로 집계되지 않았다. 스크립트·제한시간·기대값은 바꾸지 않았다.

## 시작 전 부하·메모리·프로세스

최초 점검에서는 load average `2.75, 2.93, 3.37`, available memory `167Gi`였고 native gate/컴파일러는 보이지 않았다. 이후 별도 native bootstrap 작업이 heavy lock을 기다리는 것을 발견해, 부분 gate를 중단하고 그 작업들이 끝날 때까지 기다렸다. 재측정 후 시작했다.

완전한 기준 gate 시작 직전 기록:

```text
20:10:38 up 5 days, 18:50,  4 users,  load average: 5.35, 5.52, 5.10
Mem: 188Gi total, 21Gi used, 71Gi free, 99Gi buff/cache, 167Gi available
Swap: 31Gi total, 2.9Gi used, 29Gi free
```

시작 시 `ps` 상위 목록에는 `grok-bot` 26.0%, `promtail` 11.2%, `orca-ide` 8.1%, `chrome` 4.8%, `grok` 3.9%, `node` 프로세스 각각 2.2–2.6% 등이 있었다. 별도 native gate/컴파일러는 없었다. 프로세스 이름만으로 코2 작업인지 식별할 수 없었다.

V-12 중 잘못된 `FX_ROOT` 설정으로 시작한 첫 시도는 `NATIVE_FX_CHECKOUT_MISSING`을 출력하고 사례 0건에서 끝나 무효 처리했다. 다음 부분 시도는 5/71 뒤 별도 native bootstrap이 시작되어 중단했다. 이 두 실행은 아래 gate 결과에 포함하지 않았다. bootstrap 프로세스를 종료하거나 방해하지 않았고, 종료 후 ps·부하·메모리를 다시 확인해 기준 gate를 한 번 완주했다.

## V-11 당시 ps 증거

V-11 full gate 시작 전 보존된 ps 상위 기록:

```text
    PID    PPID STAT     ELAPSED %CPU COMMAND
   2678       1 Ssl   5-16:46:20 11.2 promtail
1553899 1553839 Sl    3-09:57:09 25.8 grok-bot
3987905 3833576 Ss         00:00 17.6 bash
3987925    3807 Rsl        00:00  108 node
3987950 3526463 Rl         00:00  142 node
```

당시 `node`와 `grok-bot` 등 부하 프로세스가 관찰됐다. 이들이 코2 작업인지는 확인되지 않았다. V-11 전체 gate 결과는 보고된 대로 `NATIVE_GATE_PASS=4 FAIL=67 TIMEOUT=39 SKIP=0 TOTAL=71`이었다.

## 기준본 71건 gate

무거운 실행 전 `ps`를 확인하고 `flock -o /tmp/fl-heavy.lock` 아래 실행했다. 실행 중 load average를 1분마다 기록했다. 전체 원문 요약:

```text
NATIVE_GATE_PASS=4 FAIL=67 TIMEOUT=39 SKIP=0 TOTAL=71
```

PASS 4건은 다음과 같다.

```text
NATIVE_CASE=2 status=PASS exit=0 label=syntax: afj-db95
NATIVE_CASE=40 status=PASS exit=0 label=row-version MVCC core
NATIVE_CASE=49 status=PASS exit=0 label=auth lockout
NATIVE_CASE=51 status=PASS exit=0 label=auth state recovery
```

TIMEOUT 사례 목록. 각 항목은 gate가 `exit=124`로 기록했고 case 전체 제한 `120`초를 소진했다. Gate는 compiler-only 시간을 별도 출력하지 않으므로 아래 시간은 별도 compiler phase 측정값이 아니라 실행 제한시간이다.

| Case | 시험 | 기록된 시간 |
|---:|---|---:|
| 1 | syntax: tcp-server | 120초 제한 소진 (`exit=124`) |
| 6 | COLLATE contract | 120초 제한 소진 (`exit=124`) |
| 8 | metrics contract | 120초 제한 소진 (`exit=124`) |
| 10 | large corpus | 120초 제한 소진 (`exit=124`) |
| 11 | TCP catalog pages | 120초 제한 소진 (`exit=124`) |
| 12 | TCP lazy catalog eviction | 120초 제한 소진 (`exit=124`) |
| 14 | bounded B+Tree range | 120초 제한 소진 (`exit=124`) |
| 15 | compound unique index | 120초 제한 소진 (`exit=124`) |
| 16 | compound predicate range | 120초 제한 소진 (`exit=124`) |
| 17 | schema MVCC merge | 120초 제한 소진 (`exit=124`) |
| 18 | composite type order | 120초 제한 소진 (`exit=124`) |
| 19 | composite NULL order | 120초 제한 소진 (`exit=124`) |
| 20 | schema-aware composite order | 120초 제한 소진 (`exit=124`) |
| 21 | mixed declared composite types | 120초 제한 소진 (`exit=124`) |
| 22 | composite reopen rebuild | 120초 제한 소진 (`exit=124`) |
| 23 | schema-aware composite range | 120초 제한 소진 (`exit=124`) |
| 29 | AFJ error token mapping | 120초 제한 소진 (`exit=124`) |
| 30 | MariaDB remediation regression | 120초 제한 소진 (`exit=124`) |
| 31 | MariaDB unsupported contract | 120초 제한 소진 (`exit=124`) |
| 32 | TCP database session scope | 120초 제한 소진 (`exit=124`) |
| 33 | WAL format | 120초 제한 소진 (`exit=124`) |
| 36 | transaction isolation | 120초 제한 소진 (`exit=124`) |
| 37 | isolation levels | 120초 제한 소진 (`exit=124`) |
| 38 | MVCC conflicts | 120초 제한 소진 (`exit=124`) |
| 39 | range predicate conflict | 120초 제한 소진 (`exit=124`) |
| 41 | row-version WAL recovery | 120초 제한 소진 (`exit=124`) |
| 42 | row-version catalog checkpoint | 120초 제한 소진 (`exit=124`) |
| 43 | savepoint | 120초 제한 소진 (`exit=124`) |
| 44 | deadlock | 120초 제한 소진 (`exit=124`) |
| 45 | lock timeout | 120초 제한 소진 (`exit=124`) |
| 46 | prepared JSON adapter | 120초 제한 소진 (`exit=124`) |
| 47 | request limits | 120초 제한 소진 (`exit=124`) |
| 48 | slow request timeout | 120초 제한 소진 (`exit=124`) |
| 52 | session cleanup | 120초 제한 소진 (`exit=124`) |
| 53 | session TTL | 120초 제한 소진 (`exit=124`) |
| 54 | connection rate limit | 120초 제한 소진 (`exit=124`) |
| 55 | peer IP rate limit | 120초 제한 소진 (`exit=124`) |
| 57 | fragmented TCP requests | 120초 제한 소진 (`exit=124`) |
| 58 | SQL error mapping | 120초 제한 소진 (`exit=124`) |

다른 32개 실패는 gate 원문상 `exit=1` 또는 `exit=127`이며 timeout이 아니다. 한 예외인 `TCP 5m performance envelope`도 script상 `360`초 제한이나 `exit=1`이었다.

### 부하 상승 중 ps

21:17경 상위 ps에서 별도 `python3` 프로세스가 약 `646%` CPU를 사용하고 있었다. 뒤이은 관측에서는 약 `730–760%`였다. 명령 인자나 실행 경로는 조사하지 않았고, 해당 프로세스에 손대지 않았다. ps만으로 코2 작업인지 식별할 수 없다.

```text
PID    PPID   STAT  ELAPSED  %CPU  %MEM  COMMAND
264064 832662 Rsl   00:23     646   0.6   python3
```

이때 load average는 21:16 `10.35, 7.15, 6.27`, 21:17 `11.99, 8.04, 6.62`, 21:18 `12.54, 8.85, 6.99`, 최고 관측 21:28 `15.24, 12.76, 9.85`였다. 1분별 uptime 원문 기록을 아래에 붙인다.

```text
 20:11:38 up 5 days, 18:51,  4 users,  load average: 5.37, 5.49, 5.11
 20:12:38 up 5 days, 18:52,  4 users,  load average: 5.77, 5.57, 5.16
 20:13:38 up 5 days, 18:53,  4 users,  load average: 5.75, 5.67, 5.23
 20:14:38 up 5 days, 18:54,  4 users,  load average: 5.87, 5.84, 5.32
 20:15:38 up 5 days, 18:55,  4 users,  load average: 5.76, 5.76, 5.33
 20:16:38 up 5 days, 18:56,  4 users,  load average: 5.43, 5.69, 5.33
 20:17:38 up 5 days, 18:57,  4 users,  load average: 5.28, 5.65, 5.34
 20:18:38 up 5 days, 18:58,  4 users,  load average: 5.32, 5.60, 5.35
 20:19:38 up 5 days, 18:59,  4 users,  load average: 6.58, 5.86, 5.45
 20:20:38 up 5 days, 19:00,  4 users,  load average: 5.48, 5.69, 5.42
 20:21:38 up 5 days, 19:01,  4 users,  load average: 4.93, 5.55, 5.39
 20:22:38 up 5 days, 19:02,  4 users,  load average: 4.78, 5.40, 5.35
 20:23:39 up 5 days, 19:03,  4 users,  load average: 3.98, 5.07, 5.23
 20:24:39 up 5 days, 19:04,  4 users,  load average: 3.45, 4.72, 5.10
 20:25:39 up 5 days, 19:05,  4 users,  load average: 3.48, 4.49, 5.00
 20:26:39 up 5 days, 19:06,  4 users,  load average: 3.67, 4.36, 4.92
 20:27:39 up 5 days, 19:07,  4 users,  load average: 5.55, 4.86, 5.06
 20:28:39 up 5 days, 19:08,  4 users,  load average: 5.79, 5.10, 5.13
 20:29:39 up 5 days, 19:09,  4 users,  load average: 4.87, 5.02, 5.11
 20:30:39 up 5 days, 19:10,  4 users,  load average: 3.80, 4.68, 4.98
 20:31:39 up 5 days, 19:11,  4 users,  load average: 3.47, 4.41, 4.87
 20:32:39 up 5 days, 19:12,  4 users,  load average: 3.83, 4.29, 4.80
 20:33:39 up 5 days, 19:13,  4 users,  load average: 3.59, 4.17, 4.72
 20:34:39 up 5 days, 19:14,  4 users,  load average: 4.45, 4.27, 4.72
 20:35:39 up 5 days, 19:15,  4 users,  load average: 5.23, 4.56, 4.80
 20:36:39 up 5 days, 19:16,  4 users,  load average: 7.47, 5.32, 5.04
 20:37:39 up 5 days, 19:17,  4 users,  load average: 6.25, 5.38, 5.08
 20:38:39 up 5 days, 19:18,  4 users,  load average: 5.88, 5.39, 5.10
 20:39:39 up 5 days, 19:19,  4 users,  load average: 5.00, 5.24, 5.07
 20:40:39 up 5 days, 19:20,  4 users,  load average: 5.14, 5.25, 5.08
 20:41:39 up 5 days, 19:21,  4 users,  load average: 5.32, 5.28, 5.10
 20:42:40 up 5 days, 19:22,  4 users,  load average: 4.67, 5.12, 5.06
 20:43:40 up 5 days, 19:23,  4 users,  load average: 4.44, 4.98, 5.02
 20:44:40 up 5 days, 19:24,  4 users,  load average: 4.79, 4.96, 5.01
 20:45:40 up 5 days, 19:25,  4 users,  load average: 5.06, 4.99, 5.01
 20:46:40 up 5 days, 19:26,  4 users,  load average: 5.87, 5.24, 5.09
 20:47:40 up 5 days, 19:27,  4 users,  load average: 5.67, 5.30, 5.12
 20:48:40 up 5 days, 19:28,  4 users,  load average: 6.54, 5.62, 5.25
 20:49:40 up 5 days, 19:29,  4 users,  load average: 7.12, 5.94, 5.38
 20:50:40 up 5 days, 19:30,  4 users,  load average: 6.86, 6.09, 5.46
 20:51:40 up 5 days, 19:31,  4 users,  load average: 7.93, 6.54, 5.66
 20:52:40 up 5 days, 19:32,  4 users,  load average: 7.09, 6.71, 5.78
 20:53:40 up 5 days, 19:33,  4 users,  load average: 6.34, 6.55, 5.78
 20:54:40 up 5 days, 19:34,  4 users,  load average: 5.39, 6.32, 5.76
 20:55:40 up 5 days, 19:35,  4 users,  load average: 5.97, 6.29, 5.78
 20:56:40 up 5 days, 19:36,  4 users,  load average: 5.38, 6.02, 5.72
 20:57:40 up 5 days, 19:37,  4 users,  load average: 4.69, 5.72, 5.63
 20:58:40 up 5 days, 19:38,  4 users,  load average: 7.40, 6.35, 5.86
 20:59:40 up 5 days, 19:39,  4 users,  load average: 5.98, 6.11, 5.81
 21:00:41 up 5 days, 19:40,  4 users,  load average: 5.43, 5.91, 5.76
 21:01:41 up 5 days, 19:41,  4 users,  load average: 4.74, 5.60, 5.65
 21:02:41 up 5 days, 19:42,  4 users,  load average: 5.23, 5.57, 5.64
 21:03:41 up 5 days, 19:43,  4 users,  load average: 6.15, 5.79, 5.71
 21:04:41 up 5 days, 19:44,  4 users,  load average: 6.02, 5.82, 5.73
 21:05:41 up 5 days, 19:45,  4 users,  load average: 6.40, 6.05, 5.82
 21:06:41 up 5 days, 19:46,  4 users,  load average: 7.19, 6.26, 5.90
 21:07:41 up 5 days, 19:47,  4 users,  load average: 7.15, 6.34, 5.95
 21:08:41 up 5 days, 19:48,  4 users,  load average: 6.15, 6.27, 5.95
 21:09:41 up 5 days, 19:49,  4 users,  load average: 6.29, 6.32, 5.99
 21:10:41 up 5 days, 19:50,  4 users,  load average: 5.40, 6.11, 5.94
 21:11:41 up 5 days, 19:51,  4 users,  load average: 5.00, 5.86, 5.87
 21:12:41 up 5 days, 19:52,  4 users,  load average: 4.87, 5.65, 5.79
 21:13:41 up 5 days, 19:53,  4 users,  load average: 4.46, 5.43, 5.71
 21:14:41 up 5 days, 19:54,  4 users,  load average: 4.96, 5.41, 5.69
 21:15:41 up 5 days, 19:55,  4 users,  load average: 8.47, 6.11, 5.90
 21:16:41 up 5 days, 19:56,  4 users,  load average: 10.35, 7.15, 6.27
 21:17:41 up 5 days, 19:57,  4 users,  load average: 11.99, 8.04, 6.62
 21:18:41 up 5 days, 19:58,  4 users,  load average: 12.54, 8.85, 6.99
 21:19:42 up 5 days, 19:59,  4 users,  load average: 13.58, 9.84, 7.45
 21:20:42 up 5 days, 20:00,  4 users,  load average: 13.37, 10.54, 7.84
 21:21:42 up 5 days, 20:01,  4 users,  load average: 9.10, 9.89, 7.79
 21:22:42 up 5 days, 20:02,  4 users,  load average: 11.98, 10.43, 8.11
 21:23:42 up 5 days, 20:03,  4 users,  load average: 12.47, 10.96, 8.44
 21:24:42 up 5 days, 20:04,  4 users,  load average: 12.97, 11.27, 8.70
 21:25:42 up 5 days, 20:05,  4 users,  load average: 13.25, 11.55, 8.95
 21:26:42 up 5 days, 20:06,  4 users,  load average: 13.53, 11.92, 9.24
 21:27:42 up 5 days, 20:07,  4 users,  load average: 13.22, 12.00, 9.43
 21:28:42 up 5 days, 20:08,  4 users,  load average: 15.24, 12.76, 9.85
 21:29:42 up 5 days, 20:09,  4 users,  load average: 14.45, 13.06, 10.14
 21:30:42 up 5 days, 20:10,  4 users,  load average: 12.72, 12.80, 10.24
 21:31:42 up 5 days, 20:11,  4 users,  load average: 12.56, 12.76, 10.39
 21:32:42 up 5 days, 20:12,  4 users,  load average: 13.03, 12.76, 10.53
 21:33:42 up 5 days, 20:13,  4 users,  load average: 13.02, 12.82, 10.69
 21:34:42 up 5 days, 20:14,  4 users,  load average: 12.26, 12.78, 10.82
```

## 분기 및 판정

기준본 PASS `4`는 기준 `34`보다 낮다. 따라서 지시서 4번의 분기에 따라 병합본 gate를 실행하지 않았다. `fix/raw-tcp-review 33e78a84d5139667292db280e024c3776bb4a77e`와 코2 최신 HEAD의 임시 병합도 만들지 않았다. 병합 차이 비교나 최신 코2 HEAD 해시는 이 분기에서 필요하지 않아 조사하지 않았다.

판정은 **(다) 기준본 자체가 34를 못 냄**이다. 증거는 기준 FX `b8fdb29b94eec109db245172237c2d5a55e4c001`에서 실행한 gate 결과 `PASS=4`, V-11의 기존 gate 결과도 `PASS=4`, 그리고 기준본 gate 중 concurrent `python3` 고부하가 나타난 ps/load 기록이다. 부하가 시간 초과에 어느 정도 기여했는지는 이 단일 완주만으로 분리할 수 없으나, 기준본이 목표 통과 수를 내지 못했다는 사실은 확인됐다. raw_tcp 병합 변경 원인은 이번 분기에서 비교하지 않았다.

언어 성과: **기준본이 4/34 PASS에 그쳐 병합 비교를 생략했으므로, raw_tcp 30초 송신 제한이 db95 통과 수를 떨어뜨리지 않는지는 이번 실행으로 판정할 수 없다.**

코드·JS runner·기대값·제한시간은 수정하지 않았다. DB95 checkout은 `85424a5d3dacb829f4de921a7767cccec558eae0` 그대로이며 clean이다.

V12_SUMMARY=4/4
