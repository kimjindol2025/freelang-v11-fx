# fx-dashboard v1

**읽기 전용 fx 생태계 운영 콘솔.** 기존 앱(monitor/notify/deploy/mail/auth/kv) + FX-TRAPS를 한 화면에서 관측한다. 포트 40308.

> 원칙: **관측만.** 배포/재시작/변경/삭제 없음. down 은 정직하게 down (false-200 회피 — 빈 칸을 online으로 위장하지 않는다).

---

## 실행

```bash
cd /root/kim/freelang-v11-fx/fx-dashboard
bash build.sh                       # FX-TRAPS 수치 주입 + 표준 빌드
FL_HTTP_QUIET=1 ./fx-dashboard      # http://localhost:40308
```

대시보드는 10초마다 각 앱을 `http_get`으로 폴링한다.

---

## 7 패널 / API

| 패널 | 데이터 | API |
|---|---|---|
| System Overview | fx-monitor `/metrics` | `GET /api/overview` |
| Services | 각 앱 health → online/offline | `GET /api/services` |
| Recent Alerts | fx-notify `/notifications` | `GET /api/events` |
| Recent Deploys | fx-deploy `/deploys` | `GET /api/events` |
| Mail Activity | fx-mail `/mails` | `GET /api/events` |
| FX-TRAPS | `FX-TRAPS.airc` (빌드 주입) | `GET /api/traps` |
| Activity Feed | notify/deploy/mail 통합 | `GET /api/events` |

집계 수단: `http_get`(native) + `json_parse`(native). UI는 vanilla JS, 차트 라이브러리·프레임워크 없음(정보 밀도 우선).

---

## 검증 (이 노드, 실측)

| 항목 | 결과 |
|---|---|
| `GET /` HTML | ✅ |
| `/api/traps` | ✅ **실데이터** trap 15·lock 4·fact 2 (FX-TRAPS.airc 주입) |
| `/api/services` | ✅ 6개 `offline` (대상 앱 down — 정직) |
| `/api/overview` | ✅ `null` (monitor down — 정직) |
| `/api/events` | ✅ 빈 배열 (정직) |
| 40308 응답 | ✅ |

→ **구조 PASS + FX-TRAPS 실데이터.** 대상 앱이 도는 노드에 배포하면 서비스/이벤트 패널이 즉시 채워진다(코드 변경 없이).

---

## 빌드 메모 (이 노드 = aarch64 PRoot)

다른 워커가 추가한 runtime 파일/의존성과 맞추느라 `.fl-build-root.sh`를 조정했다:

- `-D_GNU_SOURCE` 추가 — `aliases.c`의 `strptime`이 gcc15에서 implicit declaration 에러(`_GNU_SOURCE` 누락, 다른 워커 회귀). 근본 수정은 aliases/cgc 워커 영역.
- RUNTIME_SRCS에 `websocket.c`/`regex.c`/`smtp.c`/`http_client.c` 추가 — 동기화로 들어온 신규 파일. `aliases.c`가 이들 심볼(`http_get`/`regex_split` 등)을 참조.
- `-lssl -lcrypto -lcurl` 추가 + **libcurl4-openssl-dev 설치** — `http_get`이 `http_client.c`(libcurl)로 이동. 헤더는 `/usr/include/aarch64-linux-gnu/curl/`.

> FX-TRAPS 수치는 `file_read`가 설치본 cgc 미지원이라 **build.sh가 빌드 타임에 주입**(grep → sed). FX-TRAPS.airc 변경 시 재빌드하면 반영.

## v2 이후 (이번 제외)
배포 버튼 · 서비스 재시작 · WebSocket 실시간 · 로그 뷰어 · AIRC 그래프 · Reason Log.
