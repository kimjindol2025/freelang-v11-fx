# V-20 buffer runtime API

- RUN-ID: `RUN-20261104-26`
- 작업 가지: `fix/v20-buffer`
- 시작점: `fix/v19-top-group` at `2abfb0cef39584e711ce25fcefe2fd08af977fb7`
- AFJ source snapshot used for call-site evidence: `d02503d1c8cbeb977769a2abf1f7a77e2ddd043f`
- V-18 기준 오류 묶음: 8건 (`buf-to-binary` 1, `buf-str` 4, `file_mtime` 2, `sleep_ms` 1)

## 호출 함수와 근거

| 이름 | 호출 근거 | V-18 건 |
|---|---|---:|
| `buf-to-binary` | `tests/binary-raw.fls:2`; `src/mariadb-wire.fls:312` 및 여러 packet write 호출 | 3, 4, 5, 65, 70 |
| `buf-from-binary` | `tests/binary-raw.fls:3`; `src/mariadb-wire.fls:470` | 3, 4, 5, 65, 70 |
| `buf-str` | `src/mariadb-wire.fls:8`; case 4/5/65/70이 load하는 MariaDB wire codec | 4, 5, 65, 70 |
| `buf-u8` | `src/mariadb-wire.fls:14-16`, `21-29`, `41-42` | 4, 5, 65, 70 |
| `buf-concat` | `src/mariadb-wire.fls:14`, `26`, `41`, `59`, `63` 등 | 4, 5, 65, 70 |
| `buf-len` | `src/mariadb-wire.fls:11`, `41`, `46-47`, `57` 등 | 4, 5, 65, 70 |
| `buf-read-u8` | `src/mariadb-wire.fls:21-23`, `32-35`, `96-99` 등 | 4, 5, 65, 70 |
| `buf-read-str` | `src/mariadb-wire.fls:11` | 4, 5, 65, 70 |
| `buf-slice` | `src/mariadb-wire.fls:38`, `50`, `135`, `171`, `183`, `455-456` | 4, 5, 65, 70 |
| `file_mtime` | `src/backup.fls:18`; V-18 첫 오류 원문은 `Function not found: file_mtime` | 26, 27 |
| `sleep_ms` | `tests/auth-lockout-reset.fls:18`; V-18 첫 오류 원문은 `Function not found: sleep_ms` | 50 |

V-18 원문 오류는 `docs/V-18-fail-evidence.md:44-46,67-68,91,106,111`에 있다. `buf-*` 9개 외 이름은 buffer API에 추가하지 않았다. `file_mtime`과 `sleep_ms`는 이 8건에 직접 나온 별도 런타임 호출이며, 기존 `_fl_file_modified`와 `fl_sleep_ms`로만 연결했다.

## 구현

`runtime/v11-compat.h`에 `VBuffer { size_t len; unsigned char *data; }` 내부 형식과 9개 `buf-*` dispatch 경로를 추가했다. FreeLang 쪽 버퍼 값은 AFJ 호출 계약대로 base64 문자열이다. base64 decode 중에는 byte pointer와 별도 길이를 유지하므로 NUL은 종단자로 취급되지 않는다. `buf-to-binary`/`buf-from-binary`는 이 표현과 길이 보존 raw string 사이를 변환한다. `buf-read-str`은 요청 범위의 UTF-8 유효성을 확인하고 길이 지정 문자열을 반환한다. 범위 밖 slice/read는 오류를 던진다. 기존 문자열 변환 및 decode 경로는 수정하지 않았다.

추가한 FreeLang 단위 시험: `tests/buffer-runtime.fl`. 확인 항목은 base64 raw bytes의 NUL 포함 왕복, 빈 버퍼, u8/concat/read, 범위 밖 slice 오류다.

## 확인 결과

`runtime/core.c` 구문 검사: `gcc -fsyntax-only -Werror=implicit-function-declaration -Iruntime runtime/core.c` 종료 코드 `0`.

FreeLang 단위 시험의 C 생성은 Stage2 compiler `9e08f102a579aed6ad77edac06997637e9f7b05dbe7197de1002107a6e905864`로 완료됐다. 링크 시도 3회가 모두 환경 의존성에서 실패해 바이너리 실행 및 AFJ case 재시험은 시작하지 않았다. 세 번째 동일 원인 실패에 따라 규칙대로 시험을 중단했다.

첫 기본 링크 오류 원문(경로는 가림):

```text
<가림>:16:10: fatal error: curl/curl.h: No such file or directory
compilation terminated.
```

저장소의 HTTP stub 조합으로 바꾼 두 번째 링크 오류:

```text
undefined reference to `fl_v11_http_client'
undefined reference to `fl_v11_http'
multiple definition of `fl_http_get'
undefined reference to `ws_is_upgrade_request'
undefined reference to `ws_handle_upgrade'
```

실제 HTTP server/WebSocket과 임시 `fl_v11_http_client` stub을 사용한 세 번째 링크 오류:

```text
undefined reference to `http_get'
undefined reference to `http_post'
undefined reference to `http_put'
undefined reference to `http_del'
undefined reference to `http_patch'
undefined reference to `http_body'
undefined reference to `http_status'
undefined reference to `http_ok_p'
```

HTTP를 쓰지 않는 단위 시험용으로 시도한 stub 구성도 aliases의 HTTP alias 심볼을 제공하지 못했다. sudo나 패키지 설치는 하지 않았다.

| 확인 | 결과 |
|---|---|
| 런타임 C 구문 | PASS, 종료 코드 `0` |
| 런타임 링크 | BLOCKED, `curl/curl.h` 및 HTTP stub 심볼 문제로 3회 중단 |
| `tests/buffer-runtime.fl` | C 생성 완료, 실행 미수행 |
| V-18 대상 8건의 전후 결과 | 미수행 |
| 비대상 묶음 대표 1건씩 | 미수행 |
| Stage2/3/4 고정점 | 이번 회차 재검증 미수행; 이전 검증값만 아래 기록 |

컴파일러 소스는 수정하지 않았다. 알려진 이전 검증(`docs/V-9-slow-client.md`)의 Stage2/3/4 SHA-256은 각각 `6a0d4e384a18834d4f0cb03c10460aade5bf05f999f5db981dd5dc275d313e7d`이며, 이번 가지에서 재검증 전까지는 이전 검증값으로만 취급한다.

## 현재 판정 및 기준표

버퍼 구현과 단위 시험은 추가했지만 링크 단계에 도달하지 못해 기능 통과 여부를 확인하지 못했다. Stage2/3/4의 이전 검증 SHA-256은 동일하게 기록돼 있으나 이번에는 compiler를 재생성하지 않아 고정점 판정을 하지 않는다.

| 기준 | 결과 |
|---|---|
| 1/5 함수 목록과 원문 근거 | 완료 |
| 2/5 기존 문자열 경로를 보존한 API 전용 diff | 완료 — `runtime/v11-compat.h`의 새 dispatch/helper와 새 `.fl` 시험만 변경 |
| 3/5 새 단위 시험 통과 | 미완료 — 링크 의존성 문제 |
| 4/5 고정점 및 대상/회귀 case 확인 | 미완료 — 시험 중단 |
| 5/5 Forgejo→GitHub push와 HEAD 일치 | 보류 — 검증이 끝나지 않음 |

언어 성과(현재 검증값): buffer runtime 함수 9개 추가; 해당 8건의 통과 수는 아직 확인되지 않아 `4`에서 증가했다고 말할 수 없다.

## 보충 1: WIP 고정 및 curl 없는 단독 시험

요청된 WIP 변경 파일은 다음 3개다.

| 파일 | 변경 |
|---|---|
| `runtime/v11-compat.h` | 9개 buffer runtime API 및 `file_mtime`, `sleep_ms` dispatch 추가 |
| `tests/buffer-runtime.fl` | NUL 왕복, 범위 밖 slice 오류, 빈 버퍼 등 단위 시험 추가 |
| `docs/V-20-buffer-api.md` | V-20 조사·구현·링크 실패 기록 |

C 구문 검사 원문 명령과 결과: `gcc -fsyntax-only -Werror=implicit-function-declaration -Iruntime runtime/core.c` 종료 코드 `0`.

WIP 커밋 `385f033ad535e7fd7210f41657f201d3059241cd` (`WIP: preserve V-20 buffer runtime work pending curl link`)를 만들고 Forgejo `origin`의 `fix/v20-buffer`로 push했다. `git ls-remote` 원문 ref는 다음과 같고 로컬 HEAD와 일치한다.

```text
385f033ad535e7fd7210f41657f201d3059241cd	refs/heads/fix/v20-buffer
```

## 보충 1 단독 C 시험 결과

임시 C harness가 `runtime/core.c`를 포함해 내부 buffer dispatch를 직접 호출하도록 구성했다. curl 헤더를 포함하는 파일, HTTP stub, 가짜 라이브러리는 사용하지 않았다. NUL 왕복·slice 범위 오류·빈 버퍼 세 검사를 준비했으나 링크가 실패해 실행 출력은 없다. 보충 지시에 따라 링크를 재시도하지 않았다.

링크 오류 첫 줄 원문(경로는 가림):

```text
<가림>: in function `thread_cleanup':
undefined reference to `fl_schedule_worker_cleanup'
```

같은 링크 결과의 추가 미해결 심볼:

```text
undefined reference to `fl_schedule_promise_destroy'
undefined reference to `fl_schedule_shutdown'
```

따라서 `BUFFER_NUL_ROUNDTRIP`, `BUFFER_SLICE_RANGE_ERROR`, `BUFFER_EMPTY`의 실행 결과는 모두 미확인이다. 이번 단독 시험에서 통과한 buffer 함수 수는 `0/9`다.

| 보충 1 기준 | 결과 |
|---|---|
| 1/2 WIP 커밋, Forgejo push, 원격 HEAD 일치 | 완료 — `385f033ad535e7fd7210f41657f201d3059241cd` |
| 2/2 curl 없는 단독 C 시험 3개 통과 | 미완료 — 링크 실패, 재시도하지 않음 |

언어 성과(보충 1): 단독 시험 통과 `0/9`.

V20_SUMMARY=2/5, 보충 1: 1/2
