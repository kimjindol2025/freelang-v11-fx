# FX 런타임 번들 지도

상태: `Phase 0` (문서만)  
기록일: 2026-10-12  
상위: [`decisions/SIX-ITEMS-BY-DIALECT.md`](decisions/SIX-ITEMS-BY-DIALECT.md) 6항  
근거: `runtime/*.c` 파일 헤더·export, `fl-build.sh` `RUNTIME_SRCS` (2026-10-12 실측)

## 1. 기본 정책 (고정)

**기본 빌드 = 풀셋.** 지금 `fl-build.sh`는 아래 core+net+db+mail+기타를 **한 ELF에 링크**한다.  
고정점·앱 스모크도 이 풀셋을 전제로 한다.

- `--bundle` 옵션 **구현은 Phase 1**. Phase 0은 지도만.
- 이미 있는 `--no-net`은 net 실구현을 stub으로 바꾸고 openssl/curl을 빼는 **좁은 스위치**다. 번들 체계의 완성형이 아니다.
- 기본 링크를 core-only로 줄이는 일은 결정 문서에서 **넣지 말 곳** (고정점 흔들기).

## 2. 묶음 개요

| 묶음 | 역할 | 기본 풀셋 |
|------|------|-----------|
| **core** | 값·컬렉션·산술·I/O·JSON·원자·오류·GC·alias | 항상 |
| **net** | HTTP 서버·클라이언트·WS·SSE (+ crypto 실사용 시) | 기본 on / `--no-net` 시 stub |
| **db** | SQLite · MariaDB · `(db …)` auto-attach | 항상 (sqlite3 링크, mariadb는 dlopen) |
| **mail** | SMTP(sendmail) | 항상 |
| **기타** | regex · PDF · debug · jit · shim · user-fns · cgc-bridge | 대부분 항상; 일부는 부트스트랩 전용 |

## 3. 파일 → 묶음 → 대표 빌트인

앱/코드젠이 부르는 **대표 이름** 위주. `aliases.c`의 짧은 다리 이름은 “bridge”로 표시.  
전체 심볼 목록은 파일이 export하는 `FLValue …(` 전체를 본다 — 여기는 지도용 요약.

### 3.1 core

| 파일 | 제공 (대표) | 기본 링크 |
|------|-------------|-----------|
| `core.c` | `fl_int`/`fl_str_val`/`fl_nil`, 산술·비교, `fl_println`, `fl_file_read`/`fl_file_write` | yes |
| `collection.c` | vector/map/closure: `fl_vec_*`, `fl_map_*`, `fl_fn_new`, `fl_fn_call` | yes |
| `math.c` | `fl_floor`/`fl_ceil`/…, 타입 술어, `fl_atom_new`/`fl_atom_deref`, `split`/`join`/`range` | yes |
| `io.c` | 파일·env 계열 `_fl_file_*`, `_fl_env_*`, bit ops | yes |
| `json.c` | `json_parse`, `json_stringify`, `fl_json_*` | yes |
| `process.c` | cwd/pid/run/exec/spawn (`_fl_process_*`) | yes |
| `error.c` | try 스택, `fl_throw` 경로, `fl_make_error`, uncaught 출력 | yes |
| `gc.c` | arena + RC-heap (`fl_heap_copy`, `fl_memory_stats`) | yes |
| `aliases.c` | cgc emit 이름 ↔ 구현 bridge (`env_get`, `str_*`, `count`, `future`, `fl_run_parallel`, …) | yes |

### 3.2 net

| 파일 | 제공 (대표) | 기본 링크 |
|------|-------------|-----------|
| `http.c` | `server_get`/`post`/…, `server_json`/`html`, `server_start`, `fxb_server_req_body`, `server_req_*` | yes (풀셋) |
| `http-stub.c` | 위 API nil stub | `--no-net` 때만 |
| `http_client.c` | `http_get`/`post`/`put`/`del`/`patch`/`req`, `http_body`/`status`/`ok_p` | yes |
| `http_client-stub.c` | 클라이언트 nil stub | `--no-net` |
| `websocket.c` | `server_ws`, `ws_recv`/`ws_send`/`ws_close`/`ws_conn_id` | yes |
| `websocket-stub.c` | WS stub | `--no-net` |
| `sse.c` | `server_sse`, `sse_send`/`sse_close`/`sse_alive` | yes (풀셋·`--no-net` 모두 `fl-build`에 포함) |
| `crypto-stub.c` | `sha256`/`md5` 비가용 문자열 | `--no-net` (`-DFL_NO_CRYPTO`) |

실 openssl 해시 구현 위치: **확인 안 됨을 aliases/타 파일에 둔 채** — 풀셋에서 `sha256`/`md5`가 어느 `.c`에서 오는지 Phase 0에서 단정하지 않음. stub은 `--no-net` 전용으로 확인됨.

### 3.3 db

| 파일 | 제공 (대표) | 기본 링크 |
|------|-------------|-----------|
| `sqlite.c` | `fl_rt_db`, `db_query`/`db_exec`/`db_transaction`, `fxb_sqlite_open`/`query`/`exec`/`one`/`_*_p`/`close` | yes (`-lsqlite3`) |
| `mariadb.c` | `mariadb_connect`, `mariadb_query`/`exec`/`one`/`_*_p`/`close` | yes (dlopen, 헤더 불필요) |
| `fx-builtin-shim.c` | `extern FLValue` 클로저: `sqlite_*`, `server_req_body`, `throw`, `hash_map` → `fxb_*` 등 | yes |

앱 API `(db "name")` → cgc가 `fl_rt_db(` 생성 (이름 카드·함정 archive 참고).

### 3.4 mail

| 파일 | 제공 (대표) | 기본 링크 |
|------|-------------|-----------|
| `smtp.c` | `fl_smtp_send`, `fl_smtp_send_html`, `fl_smtp_test` (sendmail popen) | yes |

FL 짧은 이름: `aliases.c`가 `smtp_send` / `smtp_send_html` → `fl_smtp_*`로 bridge (`runtime.h` 선언과 일치).

### 3.5 기타

| 파일 | 제공 (대표) | 기본 링크 |
|------|-------------|-----------|
| `regex.c` | `regex_match`/`find`/`replace`/`split` … | yes |
| `pdf_ttf.c` | `pdf_ttf_load`/`glyphs`/`embed_objs`/`size` | yes (`fl-build`) |
| `pdf_img.c` | `pdf_img_load`/`pdf_img_obj` | yes |
| `debug.c` | `FL_DEBUG` 로그, `fl_debug_dump` | yes |
| `jit.c` | JIT probe/stats/테스트 헬퍼 | yes (`fl-build`) |
| `user-fns.c` | 사용자/매니저 확장 (`ufl_*` 등) | 있으면 포함 |
| `cgc-bridge.c` | `fl_parse` (부트스트랩/네이티브 스테이지) | `verify-native-bootstrap` RUNTIME_SRCS; 일반 `fl-build` 앱 링크에는 **없음** |
| `cgc-main.c` | 생성·보조 산출물 성격 | 앱 기본 링크 아님 |
| `test_runtime.c` | 런타임 단위 테스트용 | 앱 링크 아님 |

## 4. `fl-build.sh` 풀셋 (실측 요약)

```text
core:   core collection io json math process error aliases gc debug jit fx-builtin-shim
net:    http websocket http_client sse          (+ crypto via libs; stub if --no-net)
db:     sqlite mariadb
mail:   smtp
기타:   regex pdf_ttf pdf_img [user-fns]
libs:   -lpthread -lm -ldl -lsqlite3 [-lssl -lcrypto -lcurl]
```

`scripts/verify-native-bootstrap.sh`는 `RUNTIME_SRCS`(cgc-bridge 포함)와 `APP_RUNTIME_SRCS`(앱형)를 나눈다. **둘 다 풀셋에 가깝고**, core-only가 아니다.

## 5. Phase 1에 넘기는 것

| 항목 | 내용 |
|------|------|
| `fl-build --bundle core,net,…` | opt-in 선택 링크. 기본은 풀셋 유지 |
| 번들별 스모크 | 선택 묶음만으로 hello/DB 등 |
| 문서↔심볼 자동 추출 | 이 지도를 CI로 갱신 |

## 6. 하지 않음 (Phase 0)

- `runtime/*.c` / `cgc-main.fl` 수정
- 기본 링크 축소
- AFJ·Script 번들 (AFJ 쪽은 따로)
