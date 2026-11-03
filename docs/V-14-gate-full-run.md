# V-14 — 71건 gate 전체 실행 기록

RUN-20261103-22 / 코1

## 판정 요약

이번 회차에서는 71건 gate를 실행하지 않았다. 사용할 수 있는 기준 Stage2 컴파일러를 확보하지 못했다. 따라서 gate 결과, case별 결과표, 첫 오류 상위 5건은 없다. 기존에 기록된 `NATIVE_GATE_PASS=34 FAIL=37 TIMEOUT=0 SKIP=0 TOTAL=71`은 DB95-10의 과거 결과이며 이번 실행 결과로 취급하지 않는다.

판정은 **(나) 환경이나 버전 차이**로 한정한다. 이 회차는 gate를 실행할 준비 단계에서 컴파일러가 부재/비호환임을 확인했을 뿐이므로, “기준선 34 자체가 재현 불가” 또는 부하 원인으로 확정하지 않는다.

## 1. 시작 환경 기록

FX 기준본:

```text
b8fdb29b94eec109db245172237c2d5a55e4c001
```

DB95-10 문서가 기록한 gate checkout:

```text
b60320b86de044179f284d2e7aa537a111e6da0f
```

DB95-10 문서의 AFJ 보고 커밋:

```text
203861dc075f4496f9cac040a9e74cc53b27f743
```

현재 로컬 afj-db95 checkout HEAD (읽기만 수행):

```text
da882593aae09049fbfe7a4232cd58ffcf2aa6fd
```

컴파일러 후보 기록:

```text
sha256 (freelang-tools cgc-native 후보): b9681984c7b92d581da56bf73af84053817efc5a618192ab5c9586a58c91e814
file: ELF 64-bit LSB executable, x86-64
실행 및 기준 Stage2 생성: 실패
```

FX 작업본에 포함된 `bin/cgc-bin`은 없었다. 로컬에 있던 `bin/cgc-bin` 후보의 SHA-256은 `65b615fc07ddce7fa0e40b7842a59acbfb88e771e82af4343a6d9af8cce2ab7e`이고, `file` 결과는 `ELF 64-bit LSB pie executable, ARM aarch64`였다. 이 x86-64 환경에서는 `Exec format error`로 실행할 수 없었다. 컴파일러 SHA 기록이지 gate에 사용한 compiler라는 뜻은 아니다.

시작 시 부하:

```text
 00:11:55 up  1:30,  3 users,  load average: 4.32, 4.59, 5.28
```

시작 시 상위 프로세스 목록은 `ps -eo pid,ppid,stat,etime,%cpu,%mem,comm,args --sort=-%cpu | head -15`로 확인했다. 상위에는 `node /usr/bin/node <가림>/mcp191.mjs` (123% CPU), `tailscaled` (41.2%), `python3 <가림>/doc-search/server.py` (37.1%), `next-server` (12.8%), `gnome-shell` (12.5%), `promtail` (12.3%), Chrome renderer (11.9%, 10.2%, 7.9%), `orca-ide` (7.7%) 등이 보였다. compiler/gate 작업은 없었다. 경로와 네트워크 주소는 민감정보 보호를 위해 가렸다.

`df -h /tmp` 원문:

```text
Filesystem                     Size  Used Avail Use% Mounted on
/dev/mapper/rl_kimserver-root   70G   56G   15G  80% /
```

시험용 파일과 잠금 경로는 사용자 지정대로 `<가림>` (`/home/kim/tmp` 아래)에 두었다. `/tmp`는 위 여유 공간을 읽기 확인하는 데만 사용했다. gate 실행 및 `/tmp/fl-heavy.lock`은 수행하지 않았다.

## 2. Gate 실행 여부

DB95-10 문서의 gate 명령 대상은 `tests/native-release-gate.sh`이며 71개 case를 순회한다. gate 스크립트는 `FX_NATIVE_STAGE0` 실행 파일을 요구한다. 이번 지시의 Stage0 사용 금지에 따라 Stage0를 빌드하거나 사용하지 않았다.

확보한 x86-64 native 후보 `cgc-native`로 임시 Stage1 C를 생성했지만, FX 기준 런타임과의 API 불일치로 C 빌드가 실패했다. 다른 v11 런타임으로의 재시도에서도 생성 코드의 API 불일치가 확인되어 중단했다. 대표 원문:

```text
<가림>/stage1.c:3473:134: error: too many arguments to function ‘range’
```

```text
<가림>/stage1.c:590:53: error: implicit declaration of function ‘append’; did you mean ‘fl_append’? [-Wimplicit-function-declaration]
```

```text
<가림>/stage1.c:4343:121: error: incompatible type for argument 1 of ‘fl_eq’
```

이는 native gate case 실패가 아니라 gate 전 컴파일러 준비 실패다. 동일 지점의 gate 실패 횟수에 포함하지 않았다. 입력 소스, 기대값, 제한시간, seed는 변경하지 않았다.

## 3. Case별 결과와 오류

이번 gate 실행은 없으므로 case별 PASS/FAIL/TIMEOUT 및 소요 시간은 **측정하지 않음**이다. FAIL 상위 5건의 오류도 **없음**이다. gate를 실행하지 않았으므로 실패 건수가 0건이라는 의미는 아니다.

## 4. 판정

**(나) 환경이나 버전 차이 — 준비 단계에서 확인, gate 원인은 미판정.**

근거는 (1) 실행 가능한 기준 Stage2 바이너리를 확보하지 못함, (2) x86-64와 맞지 않는 AArch64 compiler 후보, (3) x86-64 native 후보가 FX 기준 런타임과 호환되지 않아 Stage2를 만들지 못한 점이다. 따라서 이번 자료로는 부하 원인, 병합본 영향, 34건 기준선 재현 가능성을 판정할 수 없다. 다음 단계에는 DB95-10과 동일 조건에서 생성된 실행 가능한 Stage2 compiler를 확보해야 한다. 코드는 수정하지 않았다.

언어 성과: 이번 회차에서 FreeLang DB의 통과 건수는 측정되지 않았으므로 숫자로 확정할 수 없다.

## 확인 항목

1. 시작 환경 기록: 완료 (compiler 후보 SHA, 커밋 기록, uptime, ps, `/tmp` 여유 공간 기록).
2. 71건 중단 없는 실행: 미실행.
3. Case별 결과표/오류 원문: 미측정.
4. 원인 판정 및 양 원격 동일 HEAD: 환경/버전 준비 차이를 확인했으며 gate 원인은 미판정; 문서 커밋 후 원격 HEAD를 확인한다.

V14_SUMMARY=1/4
