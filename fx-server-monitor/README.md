# fx-server-monitor

**순수 FreeLang fx HTTP 서비스 Health 대시보드.**
서비스 등록(이름·URL·health path) → 주기 `http_get` → UP/DOWN · 응답시간 · 최근 1000회 기록 · 가용성%.

> FX 플랫폼의 **두 번째 증거**. SQLite Browser가 "데이터 조회/편집"이었다면, 이건 **`http_get` + `sqlite` 결합 = 운영 모니터링**. 검증된 두 빌트인을 묶은 첫 사례.

---

## 실행

```bash
cd /root/kim/freelang-v11-fx/fx-server-monitor
bash ../.fl-build-root.sh server.fl fx-server-monitor   # 표준 빌드(shim 포함)
FL_HTTP_QUIET=1 ./fx-server-monitor                      # http://localhost:40288
```

대시보드를 열면 10초마다 자동 폴링(`setInterval` → `/api/poll`). "지금 체크"로 수동 폴링, 상단 폼으로 서비스 등록.

---

## 기능 / API (POST + JSON)

| 엔드포인트 | 동작 |
|---|---|
| `GET /` | 대시보드 UI |
| `GET /health` | `{"status":"ok"}` (자기 모니터링용) |
| `POST /api/services/list` | 서비스별 최신 UP/DOWN·응답시간·가용성%·샘플수 |
| `POST /api/services/add` `{name,url,health_path}` | 등록 |
| `POST /api/services/del` `{id}` | 삭제(+체크 기록) |
| `POST /api/poll` | **모든 서비스 `http_get` → 결과 기록** (1000개 유지) |

### 데이터 모델
- `services(name, url, health_path)`
- `checks(service_id, ts, up, latency_ms)` — 서비스별 최근 **1000개**만 유지(초과 시 `DELETE … NOT IN (… LIMIT 1000)`)
- 가용성% = 최근 1000회 `AVG(up)*100`

---

## 검증 (E2E, 실측)

| 항목 | 결과 |
|---|---|
| UP 감지 (40286 sqlite-browser, self 40288) | ✅ `up=1`, latency 3~4ms, 100% |
| DOWN 감지 (59999 미존재) | ✅ `up=0`, 가용성 0% |
| 응답시간 측정 (`fl_now_ms` 전후차) | ✅ ms 단위 |
| 등록/삭제 | ✅ 즉시 반영 |
| **1000회 cap** | ✅ 1104행 → 1000행 (동일 SQL 직접 검증) |

빌드는 **표준 `.fl-build-root.sh`(shim 포함)** 로 통과 — `http_get`·`fl_now_ms`는 cgc native, `sqlite_*`·`server_req_body`는 shim.

---

## 환경 제약 (정직)

이 노드(Termux PRoot)에서 확인한 한계 — 그래서 "프로세스 관리자"가 아니라 **HTTP health 모니터**다:

- fx에 `exec`/`system` 빌트인 **없음** → `ps`/`ss`를 fx에서 못 부름
- `ss`/`lsof`도 `/proc/net` 제한으로 포트 안 보임
- → **프로세스 메모리/CPU/PID·restart 제어 불가.** HTTP 응답(health)만 측정
- `http_get`은 **HTTP only**(HTTPS·73 shell 불가)
- 서버 자체 백그라운드 스레드 없음 → 주기 폴링은 **클라이언트(`setInterval`)** 또는 외부 cron이 `/api/poll` 호출

---

## 함정 노트
- `count`는 설치본 cgc가 모르는 빌트인 → `fl_fn_call(count)` 타입에러. **SQL `COUNT`로 대체.** (`map`은 `fl_map_fn`으로 정상)
- `http_get` 실패 시 nil + stderr 진단 → 폴링 노이즈는 `FL_HTTP_QUIET=1`로 억제
