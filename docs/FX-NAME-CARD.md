# FX 이름 카드 (한 장)

상태: `Phase 1 PARTIAL` — cgc kebab API 경고 + `--strict-names` (2026-10-12)  
기록일: 2026-10-12  
상위: [`decisions/SIX-ITEMS-BY-DIALECT.md`](decisions/SIX-ITEMS-BY-DIALECT.md) 2항  
목표: AI가 **한 줄로** 「이 파일은 FX다」를 알게 한다. AFJ kebab과 **합치지 않는다**. AFJ 쪽은 따로.

## 1. 방언 표지 (파일 첫머리)

새·고치는 `.fl` 파일 상단에 한 줄:

```text
;; dialect: fx
```

- lint(`fl-lint.sh`): 표지가 **없으면 경고만** (`[dialect-marker]`). 빌드·컴파일을 막지 않는다.
- AFJ 파일은 `;; dialect: afj` (이 저장소 FX 카드 범위 밖).
- 표지 없는 기존 앱은 Phase 0에서 일괄 수정하지 않는다. 경고로만 드러낸다.

## 2. API 이름 축 — underscore

| 구분 | FX (이 카드) | AFJ (참고·합치지 않음) |
|------|--------------|------------------------|
| HTTP/JSON 등 빌트인 | `server_get` · `server_json` · `json_stringify` | `server-get` · `json-stringify` |
| 금지에 가까운 습관 | 호출부에 kebab API | — |

lint: `[dialect-fx-api]` — `(server-…)` / `(json-…)` / `(http-…)` 등 kebab 호출 경고 (기존).

실측 예: `templates/hello/server.fl`은 `server_get` / `server_json` (underscore).  
`fx-world/server.fl`은 `(http-get` 등 kebab이 많아 `[dialect-fx-api]` 경고가 난다 (2026-10-12 lint).

## 3. 핸들러 문자열

라우트 등록은 **함수 값이 아니라 문자열 이름**:

```freelang
(server_get "/" "handle-index")
(server_get "/health" "handle-health")
```

- 핸들러 **문자열**은 관례상 kebab(`"handle-index"`)을 많이 쓴다 (hello·다수 앱 실측).
- API 호출 심볼은 underscore. **문자열 핸들러 이름과 API 심볼 축을 혼동하지 말 것.**

## 4. 접두사 규칙 (2026-10-11~12 고정점 반영)

| 접두/이름 | 역할 | 비고 |
|-----------|------|------|
| `fxb_*` | 네이티브 C 빌트인 직접 호출 | 예: `fxb_sqlite_query`, `fxb_server_req_body` |
| short `sqlite_*` / `server_req_body` | cgc가 `fxb_*`로 매핑 (설치본 SHA `63411319…`) | 앱에서 short도 빌드됨. 명시적 `fxb_*`도 유효 |
| `fl_rt_db` | `(db "name")` 코드젠 대상 | 사용자 `(define db …)`와 C 심볼 충돌 방지 |
| `(db …)` | 앱 API 이름 유지 | C 심볼은 `fl_rt_db` — 앱 rename 불필요 |
| shim `extern FLValue sqlite_*` | 구 `fl_fn_call` 경로용 | 새 코드젠은 함수 호출로 쓰지 않음 |

상세 함정 이력: `FX-TRAPS.airc` `@archive` `trap-user-define-runtime-symbol`, `trap-extern-flvalue-defn`.

## 5. 한 줄 체크

```text
;; dialect: fx
API = underscore (server_get)
핸들러 등록 = 문자열 ("handle-index")
DB open 앱 API = (db …) 또는 (fxb_sqlite_open …)
AFJ kebab 호출을 이 파일에 넣지 말 것
```

## 6. Phase 0 / Phase 1

| 층 | 내용 |
|----|------|
| Phase 0 | 이 카드 + lint 방언 표지 경고 + `[dialect-fx-api]` |
| Phase 1 ④ | **cgc** naming 경고 — **접두가 아니라 닫힌 목록** [`FX-KEBAB-API-LIST.airc`](FX-KEBAB-API-LIST.airc) 34개만. 형식: `파일:줄 \| naming \| http-get \| AFJ식 kebab API \| http_get 사용 (docs/FX-NAME-CARD.md)`. 기본 경고만. 거부는 `--strict-names`. 회귀 `tests/six/run-kebab-names.sh` |
| 하지 않음 | 앱 일괄 표지 삽입, AFJ/Script 수정, 기본 거부, 전 계열 이름 통합, 사용자 kebab(`json-ok` 등) 경고 |

## 7. Phase 1 ④ 완료 / 남은 것

- 완료: 닫힌 목록 34 · `--strict-names` · fl-build 전달 · 사용자 함수 경고 0 · fx-world `http-get`만 API로 잡힘
- 남음: 설치된 shared cgc-bin 교체(사장님 승인 후), 앱 kebab 일괄 수정(범위 밖)
