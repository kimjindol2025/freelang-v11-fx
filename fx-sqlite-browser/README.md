# fx-sqlite-browser

**순수 FreeLang fx(C 네이티브)로 만든 SQLite DB 브라우저.**
테이블 조회 · 스키마 · CRUD · 임의 SQL 콘솔. Node.js 0 의존, 단일 ELF 172K.

> 목적: "fx로 사내 업무도구를 만들 수 있는가"의 실증. → **E2E 검증으로 증명됨.**

---

## 실행

```bash
cd /root/kim/freelang-v11-fx/fx-sqlite-browser
bash build.sh                 # server.fl → ELF (172K)
./fx-sqlite-browser           # http://localhost:40286
```

브라우저로 `http://localhost:40286` 접속 → 좌측 테이블 클릭 → 셀 더블클릭 편집 · 행 추가/삭제 · 하단 SQL 콘솔.

---

## 기능 / API (전부 POST + JSON)

| 엔드포인트 | 동작 |
|-----------|------|
| `GET /` | 단일 페이지 UI(SPA) |
| `POST /api/tables` | 테이블 목록 (sqlite_master) |
| `POST /api/schema` `{table}` | 컬럼/타입/PK (PRAGMA table_info) |
| `POST /api/rows` `{table,limit,offset}` | 행 조회 (rowid 포함, 페이징) |
| `POST /api/query` `{sql,params,read}` | 임의 SQL. `read=true`→SELECT 결과, `false`→실행 |

**안전장치**
- 테이블/컬럼 **식별자**는 `sqlite_master` 화이트리스트로 검증 → injection 방어 (`unknown table` 거부 실측).
- **값**은 전부 `?` 바인딩(`sqlite_query_p`/`exec_p`). `limit/offset`도 JSON 정수로 바인딩.

**검증된 시나리오**(python raw 소켓 E2E): INSERT(한글+params)·UPDATE(다중컬럼)·DELETE·SELECT WHERE ?·injection 거부 모두 통과.

---

## ⚠️ 빌드 인프라: cgc-bin 딜레마와 shim 해결

이 앱을 빌드하며 fx 툴체인의 핵심 문제를 발견·해결했다.

### 문제

이 노드에서 쓸 수 있는 cgc-bin(FL→C 컴파일러)이 둘로 갈렸다:

| cgc-bin | 함수 파라미터 | 맵 리터럴 `{}` | sqlite_* 빌트인 |
|---------|:---:|:---:|:---:|
| `/root/freelang-v11/bin/cgc-bin` (설치본) | ✅ | ✅ | ❌ 모름 |
| `/tmp/*` (신버전들) | ❌ 0인자로 깨짐 | ❌ nil로 깨짐 | ✅ |

→ **둘 다 만족하는 cgc가 없다.** 신버전 소스(`cgc-main.fl`)는 설치본으로 재빌드 시 부트스트랩이 깨져(`fl_nil()` 나열) 폐기.

### 해결: runtime shim (cgc 재빌드 회피)

설치본 cgc는 모르는 함수를 `fl_fn_call(sqlite_open, …)`(간접 호출)로 생성한다. 그 첫 인자는 `FLValue`(클로저)여야 한다. 그래서:

1. 실제 C 함수를 `fxb_*`로 rename (`runtime/sqlite.c`, `http.c`) — sqlite3 C API는 보존.
2. `runtime.h`에 **동명 전역 클로저** `extern FLValue sqlite_open, …;` 선언 → cgc 생성 코드의 타입이 통과.
3. `runtime/fx-builtin-shim.c`: 그 전역들을 `fl_fn_new(wrapper)` 네이티브 클로저로 초기화(`constructor`). 호출 시 `fxb_*`로 디스패치.

대상 8개: `sqlite_open/query/exec/one/query_p/exec_p/one_p` + `server_req_body`.

→ **이제 모든 fx 앱이 설치본 cgc로 sqlite를 쓸 수 있다.** (이 노드 한정 인프라 패치)

### 해결된 이슈

- ~~`json_parse`가 `\uXXXX` 미디코드~~ → **수정됨**(`runtime/json.c`): `\uXXXX` 코드포인트를 UTF-8로 디코드(서로게이트 페어 + `\b`/`\f` 포함). 브라우저(raw UTF-8)·`\u` 이스케이프 클라이언트 양쪽 모두 한글 정상 저장/영속 검증.

### 남은 한계

- HTTPS 미지원(HTTP only). 단일 DB 파일(`data/browser.db`) 고정.

---

## 파일

- `server.fl` — 앱 전체(라우트 + API + SPA UI)
- `build.sh` — 설치본 cgc + shim 포함 빌드
- 의존 인프라: `../runtime/fx-builtin-shim.c`, `../runtime/{sqlite,http}.c`, `../runtime/runtime.h`
