# FreeLang 언어 계열 헌장

상태: `LOCKED`
기록일: 2026-10-04
적용 범위: FreeLang v11 계열, FX 계열, Script, AFJ 및 연결 플랫폼

## 1. 결정

FreeLang은 새 언어를 계속 생산하는 프로젝트가 아니다.

현재 존재하는 언어·세대·프로파일의 역할을 고정하고, 문법·런타임·도구·검증·호환성을 성숙시키는 것을 앞으로의 기본 정책으로 한다.

새로운 표현이 필요해도 먼저 기존 계열의 확장, 방언, 내부 DSL, lowering 표면으로 검토한다. 독립 언어로 승격하는 것은 기존 계열로는 표현할 수 없는 고유 의미와 독립 실행 계약이 실제 프로젝트에서 반복 확인된 경우에만 허용한다.

## 2. 계열의 공통 원칙

- 의미와 실행 결과를 문서보다 실제 검증으로 확정한다.
- 서로 다른 계열을 같은 언어라고 부르지 않는다.
- lowering, adapter, runtime 경계를 문서화한다.
- JavaScript/TypeScript는 도구·호환 경계로 사용하고, 핵심 구현을 임의로 대체하지 않는다.
- 기존 사용자 변경사항과 검증되지 않은 상태를 보존한다.
- 새 기능보다 계약 동결, 회귀 테스트, 실제 서비스 검증을 우선한다.

## 3. 언어 계열과 역할

### 3.1 v11

FreeLang의 역사적 조상·기반 계열이다. 언어 정의와 초기 컴파일 흐름을 제공한다.

- 역할: 조상 언어, 기본 의미와 역사적 기준
- 정본: `freelang-v11` 계열
- 상태: 새로운 제품 표면이 아니라 계보·호환성 기준
- 주의: 현재의 FX와 AFJ를 서로 같은 언어로 취급하지 않는다.

### 3.2 v11-fx

FreeLang v11 계열에서 분기한 C 네이티브 FX 실행 계열이다.

- 역할: 네이티브 실행, ELF 바이너리, 운영 앱, self-hosting
- 정본: `/home/kim/kim/platform/freelang-v11-fx`
- 실행: `.fl` → C/native build → ELF
- 강점: 낮은 실행 비용, 실제 앱과 운영 경계, 고정점 검증
- 정책: 현재 FX 의미와 실행 경로의 실전 기준선으로 보존한다.

### 3.3 v11-fx2

FX의 runtime·library·interface를 확장한 별도 세대다.

- 역할: 실행 계층과 라이브러리 확장, native 프로그램 실행
- 정본: `freelang-v11-fx2`
- 상태: 별도 세대이며 FX의 안정성 기준을 자동으로 대체하지 않는다.
- 정책: 기능을 흡수할 때는 실제 검증된 계약만 선별하고, 소스·문법을 무조건 합치지 않는다.

### 3.4 v11-fx3

AI가 안정적으로 읽고 작성할 수 있는 고밀도 표면을 연구하는 계열이다.

- 역할: 정책·판단·계약·반복 구조의 AI 친화적 표면
- 정본: `/home/kim/kim/platform/freelang-fx3`
- 표면: `.fx3`
- 기준 lowering: `.fx3` → canonical `.fl`
- runtime: 자체 runtime 없음. 기존 실행 경로를 사용한다.
- 상태: 준비·설계 단계. 구현 언어로 완료했다고 부르지 않는다.

FX3는 FX3 전용 저장소나 runtime을 먼저 만드는 프로젝트가 아니다. 현재의 우선 사용처는 `fx3-mcp-policy`와 같은 정책·위험·판단 표현이다.

### 3.5 FreeLang Script

기존 FreeLang 실행기를 실제 자동화·운영 작업에 사용하는 실행 프로파일이다.

- 역할: 짧은 자동화, 배치, 데이터 변환, 개발 도구, bounded host integration
- 표면: `.fls`
- 소유 범위: CLI, 권한·capability, 진단, package manifest, 실행 계약
- 정본: `freelang-script` / `freelang-script-v1` 계열
- 정책: 기존 언어와 호환되는 실전 프로파일로 발전시키며, 불필요한 독립 문법을 만들지 않는다.

### 3.6 AFJ

v11 조상 계열에서 분기한 독립적인 언어·컴파일러·런타임 계열이다.

- 역할: 주력 언어 계약, 컴파일러, deterministic verification, 서비스 구현
- 파일 표면: `.fl`, `.fds`, `.airc`
- 정본: `/home/kim/kim/platform/freelang-afj`
- 정책: AFJ와 v11·FX를 같은 언어로 합치지 않는다.
- 기본 구현 원칙: AFJ/FreeLang으로 구현하고 JS/TS는 도구 경계로 둔다.

### 3.7 freelang-aios-core

FreeLang Script 위에 올라가는 AI OS 계층이다. 언어가 아니라 Script를 사용하는 상위 계층이다. (2026-10-13 추가, 사장님 승인)

- 역할: 스케줄러, 목표 DAG·위임, 권한(capability), DB/process/git native effect
- 정본: Forgejo `kim/freelang-aios-core` main (2026-10-13 기준 `510321a`)
- 위치: `/home/kim/freelang-script` (역사적 폴더 이름 유지, 폴더 이름 변경은 닫기)
- 경계: 언어(문법, 실행기, 기본 라이브러리, 진단, 함정 회귀)는 FreeLang Script 소관이다. aios-core 안에서 언어를 따로 고치지 않는다.
- 의존: aios-core → Script 한 방향만. Script는 aios-core를 사용하지 않는다.
- 정책: Script와 병합하지 않는다. GitHub `kimjindol2025/freelang-script`는 정본이 아니다.

## 4. 계층 관계

```text
v11
├── v11-fx
│   └── v11-fx2
│       └── FX3 표면 설계
└── AFJ
    └── FreeLang Script 프로파일

FreeLang Script / AFJ
        ↓
      db95
        ↓
  서비스·MCP·플랫폼
```

위 그림은 단순한 버전 상승표가 아니다. 각 항목은 역할이 다르다.

```text
v11      조상·기반 계보
v11-fx   native 실행 기준선
v11-fx2  runtime 확장 세대
v11-fx3  AI 정책·계약 표면
Script   운영 자동화 프로파일
AFJ      독립 주력 언어·컴파일러 계열
```

FX3는 Script나 AFJ를 대체하지 않는다. FX3가 정책을 표현하면 Script/AFJ adapter가 실행 경계와 서비스 연결을 담당할 수 있다.

## 5. 프로젝트 선택 기준

| 목적 | 우선 계열 |
|---|---|
| native 서버·운영 앱 | v11-fx |
| FX runtime/library 확장 | v11-fx2 |
| AI용 정책·판단·계약 표면 | v11-fx3 |
| 자동화·배치·운영 스크립트 | FreeLang Script |
| AI OS·스케줄러·목표 위임 | freelang-aios-core |
| 컴파일러·언어 기능·서비스 기반 | AFJ |
| 저장·트랜잭션·인덱스·복구 | db95 |
| 역사적 호환성·조상 의미 확인 | v11 |

## 6. 성숙도 정책

앞으로의 작업은 아래 순서로 진행한다.

### 1단계 — MCP 서버

언어와 도구가 외부 프로젝트에서 호출될 수 있는 JSON-RPC/MCP 경계를 고정한다.

### 2단계 — 영속 DB/KV 서버

Script·AFJ·db95를 이용해 상태·audit·evidence를 안전하게 보관한다.

### 3단계 — 패키지 매니저·빌드 시스템

언어별 의존성, lock, build, verify, 재현 가능한 실행 계약을 고정한다.

### 4단계 — 컴파일러 self-hosting

컴파일러와 핵심 도구를 자체 언어로 재생성하고, deterministic fixed point와 clean checkout 검증을 유지한다.

### 5단계 — 분산 시스템·클러스터

단일 노드의 언어·DB·MCP 계약이 안정된 뒤에만 네트워크, 복제, 분산 상태, 클러스터 실행으로 확장한다.

각 단계는 이전 단계의 실제 검증을 통과해야 한다. 기능이 존재한다는 사실만으로 다음 단계로 승격하지 않는다.

## 7. 새 언어 생성 방지 규칙

새 표현이 생기면 먼저 다음 순서로 분류한다.

1. 기존 Script/AFJ 기능 확장인가?
2. FX3 표면 또는 lowering 규칙으로 해결 가능한가?
3. 기존 runtime adapter로 연결 가능한가?
4. 최소 2개 실제 프로젝트에서 반복되는 독립 의미인가?
5. 별도의 타입·효과·실행 계약이 필요한가?

4번과 5번을 모두 만족하지 않으면 새 언어를 만들지 않고 기존 계열에 기록한다.

## 8. 검증 상태 표기

문서와 보고서에서는 다음 상태를 구분한다.

```text
DESIGNED       설계됨
PREPARED       자료·계약 준비됨
IMPLEMENTED    코드가 있음
CHECKED        정적·문법 검사를 실행함
TESTED         관련 테스트를 실제 실행함
RUNTIME        실제 실행 경로를 확인함
SELF_HOSTED    자체 언어로 재생성·고정점 검증됨
BLOCKED        환경·의존성·실행 경계로 검증하지 못함
NOT_MEASURED   측정 자체가 아직 없음
```

`IMPLEMENTED`, `CHECKED`, `TESTED`, `RUNTIME`은 서로 다른 상태다. 테스트하지 않은 것은 PASS로 보고하지 않는다.

## 9. 현재 기준선 기록

2026-10-04 로컬 확인 기준:

- `freelang-v11-fx`: FX 테스트 10/10 PASS
- `freelang-fx3`: check PASS, 전용 테스트 없음
- `freelang-afj`: check PASS
- `freelang-script`: 실행·검증 경로는 존재하지만 이번 기준선 확인에서는 명령이 제한 시간 내 완료되지 않아 별도 재검증 필요
- `freelang-v11-fx2`: 이 작업 트리에 없으며 GitHub 정본의 runtime 확장 계열로 확인

이 기준선은 언어의 최종 완성 선언이 아니다. 이후 변경 때마다 실제 명령과 증거를 갱신한다.

## 10. 최종 선언

FreeLang의 목표는 언어의 개수를 늘리는 것이 아니다.

> **하나의 생태계 안에서 실행, 자동화, 정책, 저장, 컴파일, 분산 계층을 각자 가장 잘 표현하고, 서로 검증 가능하게 연결하는 것.**

이 문서 이후 새 언어·새 세대·새 표면은 반드시 기존 계열과의 관계, 독립성이 필요한 이유, lowering/adapter 경계, 실제 검증 계획을 함께 제시해야 한다.

## 11. 개정 기록

### 2026-10-13 — freelang-aios-core 별도 계열 분리 (사장님 승인)

기존 내용은 변경하지 않고 3.7 항목과 5장 표의 한 줄을 추가한다.

1. freelang-script = Script 본체(언어, 실행기, 기본 라이브러리, 진단, 함정 회귀). 정본 fg `kim/freelang-script`, 위치 `/home/kim/kim/platform/freelang-script`.
2. freelang-aios-core = AI OS(스케줄러, 목표 위임, 권한, DB/process/git 효과). 정본 fg `kim/freelang-aios-core` (`510321a`), 위치 `/home/kim/freelang-script` (폴더 이름 변경은 닫기).
3. 의존은 aios-core → script 한 방향만. script는 aios-core를 쓰지 않는다.
4. 언어 수정은 Script에서만 한다. aios-core 안에서 언어를 따로 고치지 않는다.
5. 중복 라이브러리 2개(인계·재시도 `37d2381`/`7bee7d5`, AI 실행 `29d30de`/`3d5e375`)는 비교 전까지 보류하며 새 기능을 금지한다.
6. 두 계열의 병합을 금지한다.
