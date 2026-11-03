# V-15 — 기준 Stage2로 DB95 gate 확인

RUN-20261104-03 / 코1

## 결과 요약

기준 compiler와 소스 해시를 시작과 중단 후 확인했다. gate는 71건 중 case 9까지 끝났고, case 10 실행 중 동일 원인의 compiler timeout이 세 번째 발생해 지시의 3회 중단 규칙에 따라 gate process group을 종료했다. 71건 전체는 완료되지 않아 기준선 34 재현 여부는 판정하지 않는다.

## 1. 시작 기록

시작 SHA-256 원문:

    9e08f102a579aed6ad77edac06997637e9f7b05dbe7197de1002107a6e905864  <가림>/stage2-compiler
    6a0d4e384a18834d4f0cb03c10460aade5bf05f999f5db981dd5dc275d313e7d  <가림>/stage2.c

사용 commit:

- FX test snapshot: b8fdb29b94eec109db245172237c2d5a55e4c001
- FX report branch before this report: 771d8fc4fa14a7ca43ee17e973519cfba43065ca
- AFJ compiler repo HEAD: 3667af015827123ed4ae79a17e41a4bc9a8ca379
- afj-db95 DB95-10 gate snapshot: b60320b86de044179f284d2e7aa537a111e6da0f
- afj-db95 current checkout HEAD at start: c07262d9cfb552788c86dabd64afe1f11543e033
- DB95-10 report commit: 203861dc075f4496f9cac040a9e74cc53b27f743

afj-db95 최신 checkout에는 미커밋 변경이 있어 시험 입력으로 쓰지 않았다. DB95-10 문서가 지목한 gate snapshot b60320b86de044179f284d2e7aa537a111e6da0f의 읽기 전용 archive를 사용했다. FX도 b8fdb29b94eec109db245172237c2d5a55e4c001 snapshot을 사용했다. FX_NATIVE_STAGE0 변수에는 이름과 달리 지정 Stage2 compiler만 전달했다. Stage0를 빌드하거나 실행하지 않았다.

시작 uptime 원문:

    00:39:44 up  1:58,  3 users,  load average: 5.33, 6.08, 6.28

시작 시 ps -eo pid,ppid,stat,etime,%cpu,%mem,comm,args --sort=-%cpu | head -15 상위 행(경로 가림):

| PID | PPID | STAT | ELAPSED | %CPU | COMMAND |
|---:|---:|---|---:|---:|---|
| 457896 | 457879 | R | 00:00 | 400 | ps |
| 457878 | 454012 | Rsl | 00:00 | 144 | node (기존 mcp191 프로세스) |
| 457870 | 454012 | Rsl | 00:00 | 125 | cloudflared |
| 457879 | 307537 | Ss | 00:00 | 44.4 | bash (V-15 준비 명령) |
| 2023 | 1 | Ssl | 01:58:22 | 42.6 | tailscaled |
| 454032 | 454012 | Ss | 00:56 | 41.8 | python3 (기존 doc-search 프로세스) |
| 5640 | 4195 | Ssl | 01:56:45 | 13.3 | gnome-shell |
| 457627 | 457602 | Ssl | 00:03 | 12.7 | node (기존 daemon) |
| 457628 | 457602 | Ssl | 00:03 | 12.4 | node (기존 kernel) |
| 457626 | 457602 | Ssl | 00:03 | 12.3 | node (기존 playground) |
| 2932 | 1 | Ssl | 01:58:16 | 12.3 | promtail |
| 457602 | 1 | Ssl | 00:03 | 12.1 | PM2 |
| 457629 | 457602 | Ssl | 00:03 | 12.1 | node (기존 dashboard) |
| 4706 | 4685 | Sl | 01:56:45 | 10.9 | next-server |

목록의 Node 프로세스들은 gate 시작 전부터 존재했으며 V-15가 실행하지 않았다. gate의 local canary rollback case는 제한된 PATH에서 node: command not found로 실패했다.

사용자 공간 지시에 따라 임시 파일과 잠금 파일은 /home/kim/tmp 아래에 두었다. bubblewrap 안에서만 /tmp를 /home/kim/tmp에 bind하여 flock -o /tmp/fl-heavy.lock을 실행했다. compiler 파일도 read-only bind했다.

## 2. 실행 조건과 중단

DB95-10 gate 스크립트 bash tests/native-release-gate.sh는 원본 그대로 사용했다. FX_NATIVE_STAGE0에는 제공 Stage2 compiler를 지정했다. curl 헤더/링커 경로는 DB95-10 기록 환경에 맞췄다. 원래 case 제한시간(일반 120초, 장기 360초)을 유지했고 기대값이나 source/test 파일은 바꾸지 않았다.

| 중단 규칙에 해당한 case | 결과 원문 | 관찰 |
|---|---|---|
| 1 syntax: tcp-server | NATIVE_CASE=1 status=FAIL exit=124 label=syntax: tcp-server | 120초 초과 |
| 6 COLLATE contract | NATIVE_CASE=6 status=FAIL exit=124 label=COLLATE contract | 120초 초과 |
| 8 metrics contract | NATIVE_CASE=8 status=FAIL exit=124 label=metrics contract | 120초 초과; 동일 timeout 세 번째 |

세 timeout의 worker가 호출한 Stage2 compiler 자식이 timeout --foreground 뒤에도 CPU를 사용하며 남는 것을 관찰했다. 세 번째 동일 원인 timeout 후 V-15 중단 규칙대로 V-15 bubblewrap process group만 종료했다. case 10은 실행 중이었지만 결과 줄을 내기 전에 종료됐다. 다른 프로세스나 코2 작업은 종료하지 않았다.

## 3. 완료된 case 결과

시간은 gate가 각 NATIVE_CASE 결과 줄을 출력한 시각 사이의 경과 초로 계산했다. 첫 case는 gate 시작부터 첫 결과 줄까지다.

| Case | 결과 | 종료 코드 | 경과(초) | Label |
|---:|---|---:|---:|---|
| 1 | FAIL / TIMEOUT | 124 | 120.120 | syntax: tcp-server |
| 2 | PASS | 0 | 8.942 | syntax: afj-db95 |
| 3 | FAIL | 1 | 13.070 | raw binary boundary |
| 4 | FAIL | 1 | 28.042 | MariaDB packet codec |
| 5 | FAIL | 1 | 28.931 | MariaDB charset contract |
| 6 | FAIL / TIMEOUT | 124 | 120.015 | COLLATE contract |
| 7 | FAIL | 1 | 23.886 | page/WAL smoke |
| 8 | FAIL / TIMEOUT | 124 | 120.016 | metrics contract |
| 9 | FAIL | 127 | 0.131 | local canary rollback |
| 10 | 중단 | — | 미측정 | large corpus 실행 중 중단, 결과 줄 없음 |
| 11–71 | 미실행 | — | — | — |

완료 9건은 PASS=1 FAIL=8 TIMEOUT=3이다. 이는 전체 gate summary가 아니다. gate는 NATIVE_GATE_PASS=... TOTAL=71 summary를 출력하기 전에 중단됐다.

처음 확인된 실패 case의 첫 오류 기록:

> case 1: 120초 timeout. output.log에 오류 진단 줄 없음.

> case 3: Uncaught error: [buf-to-binary] Function not found: buf-to-binary (at line 2, col 0)

> case 4: Uncaught error: [buf-str] Function not found: buf-str (at line 116, col 0)

> case 5: Uncaught error: [buf-str] Function not found: buf-str (at line 116, col 0)

> case 6: 120초 timeout. output.log에 오류 진단 줄 없음.

추가 완료 실패 원문:

> case 7: Uncaught error (line 455): [oversized-wal-smoke-test] oversized WAL was accepted (at line 486, col 0)

> case 8: 120초 timeout. output.log에 오류 진단 줄 없음.

> case 9: /<가림>/afj-db95/.freelang/smoke.sh: line 11: node: command not found

종료 후 SHA-256 원문:

    9e08f102a579aed6ad77edac06997637e9f7b05dbe7197de1002107a6e905864  <가림>/stage2-compiler
    6a0d4e384a18834d4f0cb03c10460aade5bf05f999f5db981dd5dc275d313e7d  <가림>/stage2.c

시작과 종료 시 두 해시는 각각 같았다. 제공 compiler 파일을 옮기거나 덮어쓰지 않았다.

## 4. 판정과 언어 성과

71건 전체가 끝나지 않아 판정 선택지 (가) 또는 (나)는 확정할 수 없다. 관찰된 9건 중 통과는 1건이며 이를 71건 통과 수로 일반화하지 않는다. 기준선 34의 재현 여부와 전체 FreeLang DB 통과 건수는 미판정이다.

중단 사유는 서로 다른 세 case에서 같은 120초 compiler timeout이 세 번 발생한 것이다. 추가로 timeout이 남긴 compiler 자식이 계속 실행되는 현상도 관찰했다. source, 기대값, 제한시간은 변경하지 않았다. 3회 중단 지시에 따라 전체 gate를 더 실행하지 않고 다음 지시를 기다린다.

## 푸시 확인

보고서만 커밋했다. Forgejo를 먼저, GitHub를 다음으로 푸시하고 branch ref를 검증했다.

V15_SUMMARY=1/4
