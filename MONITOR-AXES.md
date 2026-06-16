# fx 모니터 축 분류 + 통합 제안

> 상태: **셋 다 보존.** 현재 조치 = 중복 축 기록 + 통합 제안까지.
> 실제 통합 작업은 **저장소 무결성(shop-store.fl) 복구 + reason 작업 반영 이후 별도 설계 단계**에서 수행 (2026-06-16 kim 결정).

세 모니터가 공존한다. 비교 결과, **두 개의 관점 축**으로 갈린다.

## 비교표

| 축 | fx-monitor (40301) | fx-server-monitor (40288) | fx-live-monitor (40290) |
|---|---|---|---|
| **관점** | **Inside-Out** (호스트) | **Outside-In** (서비스) | **Outside-In** (서비스) |
| 수집 | shell_run (CPU/Mem/Disk/PM2/Port) | http_get | http_get + websocket |
| 응답시간 | ✅ | ✅ | ✅ (response_ms) |
| 가용성% | ❌ | ✅ | ❌ |
| 1000회 cap | ❌ | ✅ | ❌ |
| 동적 등록(CRUD) | ❌ | ✅ | ❌ (하드코딩 SERVICES) |
| websocket 실시간 | ❌ | ❌ (폴링 10s) | ✅ (ws push) |
| 알림(fx-notify 연동) | ✅ | ❌ | ❌ |
| UI | ❌ headless | ✅ 대시보드 | ✅ 대시보드 |

## 판정

- **fx-monitor = Inside-Out (호스트 자원).** 시스템 메트릭 + PM2 + 알림. **다른 축 — 중복 아님, 보존.**
- **fx-server-monitor ↔ fx-live-monitor = 같은 축(Outside-In HTTP health) 중복.** 서비스 up/down·응답시간·저장·UI가 겹친다.
  - 차이가 상보적이라 완전 폐기는 아님(재발명 처리 "결과 B"):
    - **fx-server-monitor 고유**: 가용성% · 1000회 cap · 동적 등록(CRUD)
    - **fx-live-monitor 고유**: websocket 실시간 푸시 · history

> 메모: fx-server-monitor(2026-06-16 작성)는 fx-live-monitor가 이미 있는 줄 모르고 만든 **부분 재발명**이다. fetch-first 누락 = `claimed≠observed`. 통합 시 이 사실을 출발점으로.

## 통합 제안 (별도 설계 단계에서)

두 Outside-In 모니터를 **하나의 완성형**으로 합친다:

```
Outside-In 통합 모니터 =
  동적 등록(CRUD)        ← fx-server-monitor
  + 가용성% · 1000회 cap ← fx-server-monitor
  + websocket 실시간     ← fx-live-monitor
  + history              ← fx-live-monitor
```

- fx-monitor(Inside-Out)는 그대로 두고, 통합 모니터(Outside-In)와 **양방향 관측 쌍**을 이룬다.
- fx-dashboard(40308)가 둘 다 패널로 집계한다.

## 보류 사유 (지금 통합 안 하는 이유)

1. **fx-live-monitor는 다른 워커 작** — 즉결 병합/폐기는 unrelated 보존 규율 위반. 워커 조율 필요.
2. 선행 작업: **shop-store.fl 무결성 복구 + reason 작업 반영**이 먼저.
3. 통합은 그 이후 **별도 설계 단계**에서 (이 문서가 그 출발점).
