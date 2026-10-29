# L08 C20 요청 arena cache와 DB 동시 쓰기 재현

## 확인한 것

allocator 구현은 `3327ff1ca6f14b01d617e72d479f4deb049686c4`, 계측 범위를 바로잡은 시험 고정은 `4da45cd901085c9ac878771bf33422c5b1bf4b94`다. 원래 `.fl`·정답·C 계측을 `9b77c7cadcc22bd072414335f7f85afa4d239ceb`에 먼저 고정했고, 최초 실패를 `10c6d0e4110f27846ff430b34210ef49aa4127bb`에 보존했다. [계약](../tests/l08-platform/arena-cache-contract.md), [최초 실패](evidence/l08-platform/arena-cache-before.txt).

### 요청 경계 뒤 버퍼 보존량

기존 arena는 큰 청크를 추가할 때 작아진 이전 청크도 연결해 두고 요청 종료 뒤 모두 보존했다. `.fl` callback이 64 KiB부터 4 MiB까지 커지는 문자열을 64회 만들면 이 cache가 누적됐다. 요청 경계에서는 첫 64 KiB 청크만 재사용하고 나머지는 해제하도록 바꿨다. 현재 요청의 payload를 경계 전에 읽는 동작과 자식 arena를 정리하는 동안 부모의 큰 버퍼는 유지한다.

최초 계측 `136319000`바이트에는 요청 전에 만든 전역 argv vector `24`바이트도 포함됐다. 첫 수정 뒤의 일반·sanitizer 실행은 이 `24`바이트 때문에 실패했다. 원문을 지우지 않고 보존했다. [일반 계측 실패](evidence/l08-platform/arena-cache-meter-normal-before.txt), [sanitizer 계측 실패](evidence/l08-platform/arena-cache-meter-sanitized-before.txt). TLS 계측을 private test scope 준비 뒤 켜도록 고친 뒤 정답 `0`을 유지한 채 수정 전 고정 runtime과 수정 후 고정 runtime을 다시 비교했다. argv나 정답을 삭제해 통과시키지 않았다.

| 실제 요청 범위 계측 | 수정 전 `166ef374bb8417f7cd2227f23e3b98de47fe0f84` | 수정 후 `4da45cd901085c9ac878771bf33422c5b1bf4b94` |
|---|---|---|
| 64회 reset 뒤 추가 cache | `136318976` | `0` |
| fallback 포함 reset 뒤 보존 바이트 | `144707584` | `0` |
| payload 전체 내용 | `PASS` | `PASS` |
| 첫 작은 청크 재사용·중첩 부모 | `PASS` | `PASS` |
| cleanup 등록 실패 후 안전한 NULL·정리 | `FAIL` | `PASS` |
| SIZE_MAX 거부 | `FAIL` | `PASS` |
| 전체 프로그램 종료 | `1` | 일반 `0`, ASan/UBSan `0` |

[정정한 계측의 수정 전 원문](evidence/l08-platform/arena-cache-scoped-before.txt), [수정 후 일반 원문](evidence/l08-platform/arena-cache-scoped-normal.txt), [수정 후 sanitizer 원문](evidence/l08-platform/arena-cache-scoped-sanitized.txt). 각각 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다. 잘못 계측한 최초 값을 실제 요청 cache라고 계속 보고하지 않는다.

청크 확보가 실패한 뒤 fallback malloc이 성공하면 arena cleanup에 등록한다. cleanup 등록까지 실패하면 즉시 free하고 NULL을 반환한다. 첫 malloc 실패·두 malloc 실패·등록 malloc 실패를 실제 link wrapper로 주입했다. 정렬과 청크 크기 overflow를 거부하고 남은 공간 비교에서 덧셈 overflow를 피했다. 해당 분기와 반환 버퍼의 마지막 바이트도 확인했다. `L08_ARENA_CACHE_PASS=1 FAIL=0`, `ARENA_CACHE_SCOPED_NORMAL_EXIT=0`, `ARENA_CACHE_SCOPED_SANITIZED_EXIT=0`이다. `GENERAL_LSAN=UNEXECUTED`, `detect_leaks=0`이며 모든 malloc 누수를 해결한 것은 아니다.

```bash
L08_ARENA_CACHE_RUNTIME_ROOT=<가림> L08_ARENA_CACHE_OUT=<가림> L08_ARENA_CACHE_COMPILER=<가림>/compiler strace -f -e trace=execve -o <가림>/execve.log bash scripts/verify-l08-arena-cache.sh
L08_ARENA_CACHE_RUNTIME_ROOT=<가림> L08_ARENA_CACHE_OUT=<가림> L08_ARENA_CACHE_COMPILER=<가림>/compiler L08_ARENA_CACHE_SANITIZER_LIBDIR=<가림> strace -f -e trace=execve -o <가림>/execve.log bash scripts/verify-l08-arena-cache.sh
```

### 실제 업로드와 최신 앱 회귀

고정 `4da45cd901085c9ac878771bf33422c5b1bf4b94`에서 native compiler를 새로 만들고 기존 L06 실제 loopback HTTP/TLS를 실행했다. `L06_HTTP_PASS=110 FAIL=0`, `L06_TLS_PASS=110 FAIL=0`, `L06_HTTP_TLS=PASS`, `CACHE_L06_HTTP_TLS_EXIT=0`이다. 기존 PNG/JPEG/GIF 무손실·업로드 상한·중단·timeout·자기 시험 server 재시작과 임시 파일 정리 단정을 유지했다. [원문](evidence/l08-platform/arena-cache-l06-after.txt).

같은 고정 소스와 이 native compiler로 승인된 source-only blog 복사본의 저장소 ASan/UBSan writer 1회·별도 reader 10회가 모두 종료 `0`, `DB95_NATIVE_STORE_PASS=11 FAIL=0`, `CACHE_BLOG_STORE_EXIT=0`이다. 합성 WAL SHA-256 `54d6fcd07cdaad992093650e8474bc88010b9eb6d191c4a074e775d483c46059`, `DB95_NATIVE_WAL_READ_ONLY=PASS`다. [원문](evidence/l08-platform/arena-cache-store-after.txt).

원래 전체 main.fl·세션 seed·editor-http의 native 빌드도 각각 종료 `0`, `CACHE_BLOG_EDITOR_EXIT=0`, HTTP 확인 `3`회, 재시작 `2`회, WAL 무변경이다. main 실행물 SHA-256 `95d74e63d6c014b672d87642a766bde1fda2cce9c6565cd8e4c9211c708494ec`다. [원문](evidence/l08-platform/arena-cache-editor-after.txt). 합성 세션이며 `PASSWORD_LOGIN=UNEXECUTED`, `EXISTING_EDITOR_JS=SERVED_NOT_EXECUTED`다. 위 회귀의 execve 감사도 각각 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다.

```bash
L06_OUT=<가림> strace -f -e trace=execve -o <가림>/execve.log bash scripts/verify-l06-http.sh
L08_BLOG_SOURCE_ROOT=<가림> L08_BLOG_NATIVE_COMPILER=<가림>/compiler L08_BLOG_STORE_OUT=<가림> L08_BLOG_STORE_SANITIZER_LIBDIR=<가림> strace -f -e trace=execve -o <가림>/execve.log bash scripts/verify-l08-blog-store-native.sh
L08_BLOG_SOURCE_ROOT=<가림> L08_BLOG_NATIVE_COMPILER=<가림>/compiler L08_BLOG_EDITOR_OUT=<가림> strace -f -e trace=execve -o <가림>/execve.log bash scripts/verify-l08-blog-editor-native.sh
```

## 미확인 / 새로 재현한 DB 불일치

원래 app/store.fl·vendor를 변경하지 않고 실제 native worker `8`개가 각각 다른 글 `4`건을 commit하는 최소 시험·정답을 먼저 고정했다. fixture `c618fab67738ae4e2b43463049b107e8927a710e`, driver `2ec08a7a4bcf510a359db2394118640a3a08ecc9`다. C helper는 commit 전 barrier와 catalog deref 뒤 0.5 ms pause만 제공하며 읽기 안에서 barrier를 걸지 않는다. reader는 같은 WAL을 새 과정의 원래 복구로 읽는다. [계약](../tests/l08-platform/store-concurrent-contract.md).

실제 원문은 `DB_CONCURRENT_WORKERS=8`, `DB_CONCURRENT_OK_WORKERS=8`, `DB_CONCURRENT_ROWS=20`, `DB_CONCURRENT_UNIQUE_IDS=20`, `DB_CONCURRENT_write_EXIT=1`이다. 기대 live rows/고유 id는 각각 `32`다. 새 과정에서는 `DB_CONCURRENT_REPLAY_ROWS=32`, `DB_CONCURRENT_REPLAY_UNIQUE_IDS=32`, `DB_CONCURRENT_REPLAY=PASS`, `DB_CONCURRENT_read_EXIT=0`, `DB_CONCURRENT_WAL_READ_ONLY=PASS`다. 전체 `DB_CONCURRENT_BEFORE_EXIT=1`, `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다. [실패·재생 원문](evidence/l08-platform/store-concurrent-before.txt).

이는 개별 atom 갱신이 안전해도 DB의 table snapshot·검증·catalog/index 반영·WAL·revision 구간이 하나의 원자적 작업으로 보호되지 않음을 보여 준다. 재생하면 32건이 된다는 이유로 live 20건을 정상으로 보고하지 않는다. WAL 거래를 제거하거나 worker를 1개로 낮추지 않았다. 이 문제는 아직 고치지 않았으며 다음 동기화 구현의 실패 기준으로 유지한다.

다음 구현은 명시적인 언어 동기화 primitive의 자체 `.fl`·고정 정답부터 진행하고, callback 한 번·인자 순서·중첩 재진입·오류 시 해제·다른 lock의 독립 진행·관리되는 lock 수명을 확인해야 한다. 그 뒤 FreeLang 저장소 경계에서 commit 전체와 읽기를 보호하고 같은 8-worker live/replay 시험과 실제 HTTP 통합을 확인해야 한다. DB 함수명을 C runtime에 하드코딩하거나 atom 값을 임의로 바꾸어 mutex를 흉내 내는 우회는 하지 않는다. 기존 함수 wrapping이 오류 메시지·함수 값 의미를 바꾸지 않는지도 검증한다.

- 이전 GC 고정 `166ef374bb8417f7cd2227f23e3b98de47fe0f84` 전체 실행은 계속 관찰 중이다. cache 변경 고정 `4da45cd901085c9ac878771bf33422c5b1bf4b94`의 전체 언어 회귀·L06/L07·native bootstrap도 새로 실행 중이다. 이미 실행 중인 두 고정 driver에 이후 추가한 DB 통합 재현은 포함되지 않으며, 두 driver의 결과가 나오더라도 새 DB 실패나 C20 전체 완료를 통과로 바꾸지 않는다.
- 일반 LSan·장시간 실제 RSS·PRD p95, opaque promise cycle·startup 수명·포기한 future/반복/동시 대기·예외, DB 오류/장애 원자성·동시 읽기·WAL/복구·성능의 나머지 검증은 남아 있다.
- 실제 비밀번호 검증의 빈 scrypt 키 계약은 사용자 결정 대기다. 브라우저 J안·이미지 처리·댓글·다중 작성자와 나머지 PRD·운영 자료 호환·백업/복구도 남아 있다. 서버 23·25 접속·운영 자료 변경·합치기·배포·강제 푸시·백업 삭제는 하지 않았다.
