# 6항 × 언어별 배치

상태: `LOCKED`  
기록일: 2026-10-11  
적용 범위: FreeLang 계열 (v11, FX, FX2, FX3, AFJ, Script)  
상위 헌장: [`FREELANG-LANGUAGE-FAMILY-CHARTER.md`](FREELANG-LANGUAGE-FAMILY-CHARTER.md)

## 1. 결정

「AI가 원한 언어와 FX 사이의 거리」에서 나온 여섯 가지 개선을  
**한 언어에 몰아넣지 않는다.**  
각 계열이 맡은 역할에 맞춰 착지점을 나눈다.

| # | 개선 | 1순위 (본체) | 2순위 (맞춤·흡수) | 넣지 말 곳 |
|---|------|--------------|-------------------|------------|
| 1 | 함정 → 언어 규칙·컴파일 오류·목록 축소 | **FX** (`FX-TRAPS` → cgc/runtime) | AFJ는 `MISTAKES`로 **자기 함정만** | v11 소급, FX2 덤프, FX3로 함정 이관 |
| 2 | 실행·이름 규칙을 「하나로」 | **계열마다 한 축** (합치지 않음) | FX=`underscore` 문서+lint / AFJ=`kebab` 유지 / Script=AFJ 축 | 전 계열 심볼 강제 통합 |
| 3 | nil 대신 성공·실패, 미처리 시 거부 | **AFJ**에서 계약·match 실험 | FX는 `*-safe` / Result **병행 API**만. Script는 관례 따름 | FX 고정점 본선에 즉시 강제, FX2에만 먼저 |
| 4 | 함수마다 받는·돌려·바꾸는 칸 + 검사 | **AFJ** (effect 설계 이력) | FX는 빌트인 효과표 + lint. Script는 capability와 **어휘만** 맞춤 | FX cgc에 문법 필수화, FX3에 새 타입 시스템 |
| 5 | 오류를 `위치, 값, 이유, 고치는 법` 한 줄 | **공통 스키마** + Script 진단 CLI + FX `error.c` | AFJ stderr/도구도 같은 스키마 | 계열마다 다른 포맷으로 분기 |
| 6 | 핵심 묶음 + 웹·DB 선택 묶음 | **FX** (`fl-build` 번들, 기본은 풀셋 유지) | Script package/capability로 권한 묶음 | FX2에만 번들, 기본 링크 core-only로 축소해 고정점 흔들기 |

## 2. 핵심 원칙 (합의)

### 2.1 이름 규칙의 목표

목표는 **모든 언어의 이름을 하나로 맞추는 것**이 아니다.

목표는 AI가 다음을 **한 줄로** 알게 하는 것이다.

```text
이 파일의 방언은 FX다.
이 파일의 방언은 AFJ다.
```

- FX 내부: API underscore 등 **방언 축**을 문서·lint로 고정한다.
- AFJ 내부: kebab 정본을 유지한다.
- 계열 간 강제 통합은 하지 않는다. 헌장의 「같은 언어로 합치지 않는다」와 같다.
- AI 혼동의 진짜 원인은 「우주 단일 이름 부재」가 아니라 **방언 표지가 안 보이는 것**이다.

### 2.2 강제 실험은 AFJ에서 먼저

Result 미처리 거부, 효과 칸 문법화처럼 **규칙을 세게 거는 실험**은 AFJ에서 먼저 굳힌다.

- FX는 고정점(self-host 3세대 수렴)을 지키며 **옆에 붙이는 방식**만 쓴다  
  (병행 API, lint, 문서, 관례).
- AFJ에서 검증된 계약만 나중에 FX로 옮긴다.
- 고정점을 지키면서 앞으로 나아가는 기본 경로다.

### 2.3 계열 역할 (요약)

| 계열 | 역할 | 이번 6항에서 |
|------|------|----------------|
| v11 | 조상·역사 기준 | 참고만. 새 수정 본선 아님 |
| FX (v11-fx) | 네이티브·고정점·운영 ELF | 함정 본체, FX 이름 축, 오류 runtime, 번들 |
| FX2 | FX 확장 세대 | **지금 착지 금지**. 선별 흡수만 |
| FX3 | AI 표면 → `.fl` lowering | 표현만. 실행 의미는 FX/AFJ/Script |
| AFJ | 주력 서비스·언어 계약 | Result·effect 강제 실험 |
| Script | 자동화·capability·진단 | 오류 스키마·권한 묶음 |

## 3. 작업 층 (고정점 보호)

모든 착지는 A → B → C 순으로만 올린다.

| 층 | 의미 | 고정점 |
|----|------|--------|
| **A** | 문서·재분류·lint·관례·템플릿 | 영향 없음 |
| **B** | 병행 API·runtime 진단·opt-in 번들·함정 1개씩 | 게이트 필수 (`verify-native-bootstrap` / `verify-fixpoint`) |
| **C** | 컴파일러 강제 시맨틱·기본 링크 축소·전 계열 통합 | 별 로드맵. 지금 본선 아님 |

FX에서 C층(Result 강제, effect 문법 필수, 기본 core-only)은  
§2.2에 따라 **AFJ 검증 이전에는 열지 않는다.**

## 4. 다음 작업

1. **FX Phase 0** — 함정 재분류표  
   → [`FX-TRAPS-RECLASS-PHASE0.md`](FX-TRAPS-RECLASS-PHASE0.md) · 함정 축소는 별 트랙 (활성 RUNTIME_OPEN)
2. FX Phase 0 (6항 문서·lint, 2026-10-12 지시서)  
   - **5항 오류 스키마 — Phase 0 완료** → [`ERROR-SCHEMA.md`](ERROR-SCHEMA.md)  
   - **2항 이름 카드 — Phase 0 완료** → [`../FX-NAME-CARD.md`](../FX-NAME-CARD.md) · `fl-lint` `[dialect-marker]`  
   - **6항 번들 지도 — Phase 0 완료** → [`../FX-BUNDLE-MAP.md`](../FX-BUNDLE-MAP.md)  
   - **3항 Result 관례 — Phase 0 완료** → [`../FX-RESULT-CONVENTION.md`](../FX-RESULT-CONVENTION.md) (실패→nil은 실측분만)  
   - **4항 효과 주석 — Phase 0 완료** → [`../FX-BUILTIN-EFFECTS.airc`](../FX-BUILTIN-EFFECTS.airc) · lint `[effect-anno]`(형식만)
3. AFJ — Result·effect 계약 초안 (강제 실험 착수 조건 정리) · **AFJ 쪽은 따로**
4. Script — 오류 스키마·capability 묶음을 공통 스키마에 맞출지 별도 안건

## 5. 검증 상태

| 항목 | 상태 |
|------|------|
| 배치 합의 | LOCKED (본 문서) |
| FX 고정점 (기록일 전후 실측) | PASS — stage2==stage3; 2026-10-12 게이트 SHA `63411319…` |
| 함정 목록 0 | IN_PROGRESS — 활성 7 RUNTIME_OPEN (1항 트랙) |
| **5항 오류 한 줄 스키마 (FX Phase 0)** | **완료** — [`ERROR-SCHEMA.md`](ERROR-SCHEMA.md) |
| **5항 오류 한 줄 (FX Phase 1 ①)** | **완료** — `runtime/error.c`가 ERROR-SCHEMA 한 줄을 Uncaught 앞에 출력; `fl_make_error`에 value/fix; 자주 나는 5종 fix 문구. 회귀 `tests/six/run-error-oneline.sh` |
| **3항 Result 병행 (FX Phase 1 ②)** | **완료** — `http_get_result` 등 `*_result` 5개 + `fl_result_ok/fail` + lint `[result-nil-http]`. 감시앱 목록만 [`../FX-RESULT-MONITOR-APPS.md`](../FX-RESULT-MONITOR-APPS.md). 회귀 `tests/six/run-result-api.sh` |
| **2항 FX 이름 카드 (FX Phase 0)** | **완료** — [`../FX-NAME-CARD.md`](../FX-NAME-CARD.md) |
| **6항 번들 지도 (FX Phase 0)** | **완료** — [`../FX-BUNDLE-MAP.md`](../FX-BUNDLE-MAP.md) (`--bundle`=Phase 1) |
| **3항 Result 관례 (FX Phase 0)** | **완료** — [`../FX-RESULT-CONVENTION.md`](../FX-RESULT-CONVENTION.md) |
| **4항 효과 주석 (FX Phase 0)** | **완료** — [`../FX-BUILTIN-EFFECTS.airc`](../FX-BUILTIN-EFFECTS.airc) (20 builtins) |
| AFJ Result 강제 | NOT_STARTED |
| 전 계열 오류 스키마 | FX 문서 Phase 0 완료; AFJ/Script 구현은 따로 |
| **FX Phase 0 6항 문서·lint** | **5/5 완료** (2026-10-12) |

## 6. 변경 규칙

- 본 문서의 1순위/넣지 말 곳을 바꾸려면 사용자 합의 후 `LOCKED` 버전을 갱신한다.
- FX2·v11에 6항 본체를 넣는 제안은 기본 거부한다.
- 「이름 하나로 전 계열 통합」제안은 §2.1에 의해 거부한다.
