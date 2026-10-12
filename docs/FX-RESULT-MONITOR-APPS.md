# http_get 실패를 nil로 보는 감시·대시보드 앱 목록

상태: Phase 1 ② 조사만 (수정 안 함)  
기록일: 2026-10-12  
관련: [`FX-RESULT-CONVENTION.md`](FX-RESULT-CONVENTION.md) · lint `[result-nil-http]`

`http_get` 실패는 **맵 `status=0`** 이다. `(nil? $resp)` / `(null? $resp)`는 연결 실패를 잡지 못한다.

| 앱 | 파일 | 패턴 | 비고 |
|----|------|------|------|
| fx-server-monitor | `fx-server-monitor/server.fl:37` | `(if (nil? $resp) 0 1)` | UP/DOWN 오인 가능 |
| fx-dashboard | `fx-dashboard/server.fl:20,25` | `(if (nil? $r) …)` | offline/online·json 파싱 오인 가능 |
| fx-live-monitor | `fx-live-monitor/server.fl:41-43` | try/catch + `http-ok?` | status 경로 사용 — **양호** |
| fx-world | `fx-world/server.fl:10-12` | `http-ok?` / `http-body` | status 경로 사용 — **양호** |

권장(수정은 별 지시): `(http_get_result $url)` 또는 `(get $r "status")` / `http-get-safe`.
