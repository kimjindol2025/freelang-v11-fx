# L08 C20 요청 범위 atom 수명과 순환 수거

## 확인한 것

GC 본체 `8b102ad7390705df16b3471b1f38e9695973d91e`, 완료 worker의 소유권 보완 `8563226e0d3b92ea44f0d71ed5e0d03c457ace93`, HTTP 확장 시험 고정 `166ef374bb8417f7cd2227f23e3b98de47fe0f84`다. `.fl`·정답·계측 C helper를 먼저 커밋하고 실패 원문을 보존했다. 앱·vendor·운영 자료는 변경하지 않았다.

### 수정 전과 수정 후

| 항목 | 수정 전 원문 | 수정 후 원문 |
|---|---|---|
| scope의 임시 atom 1000개 | 기대 `0`, 실제 `1000`, `ATOM_LIFETIME_BEFORE_EXIT=1` | 실제 free 계측 `0`, `ATOM_LIFETIME_AFTER_EXIT=0` |
| worker가 캡처한 atom | 기대 `0`, 실제 `1`, `ATOM_WORKER_BEFORE_EXIT=1` | 캡처·반환 모두 해제, `ATOM_WORKER_SANITIZER_EXIT=0` |
| HTTP 임시 cycle 10000개 | `HTTP_ATOM_TEMP_LIVE=10000` | `HTTP_ATOM_TEMP_LIVE=0` |
| HTTP 다음 요청에 보관한 atom | 기존 누적 누수 포함 `HTTP_ATOM_SAVED_LIVE=10001` | `HTTP_ATOM_SAVED_LIVE=1`, 별도 요청에서 `한🙂` 읽기 성공 |
| HTTP keeper를 비운 뒤 | `HTTP_ATOM_CLEARED_LIVE=10001` | `HTTP_ATOM_CLEARED_LIVE=0` |

최초 시험·정답은 `c48db6f9977230075e2e86ea8f0ccf9af68fe56b`, 최초 실패 고정은 `a07a7e10e3b0e4f78fb8116bf8d7ebe9a8baff54`다. worker 운반 시험은 `084d87f9536ed8d594a55479ecb6b794079aadb0`, 해당 실패 고정은 `1c0b28fa2bb5dae3a5bc75ccb27342e0ddb49d3b`다. [원래 atom 계약](../tests/l08-platform/atom-lifetime-contract.md), [최초 실패](evidence/l08-platform/atom-lifetime-before.txt), [worker 실패](evidence/l08-platform/atom-worker-before.txt).

활성 arena에서 만든 atom은 내부 brand로 구분하며 arena가 초기 참조를 소유한다. heap copy는 같은 atom을 강하게 참조하고 arena copy는 반환받은 arena에 소유권을 등록한다. atom별 mutex와 그 값은 마지막 소유자가 사라질 때 해제한다. `0xFF`와 실제 참조 수 255를 혼동하지 않도록 managed atom은 별도 brand를 쓴다. arena가 없는 startup 생성값은 기존 process 수명을 유지한다.

arena 정리 시 후보 atom에서 도달하는 소유된 map/vector/string/closure 그래프의 내부 참조 수와 실제 RC를 비교한다. 외부 소유자가 있는 노드와 그 자손은 보존하고, 도달하지 못하는 atom·closure cycle은 간선을 먼저 분리한 뒤 수거한다. 공유 그래프 읽기·참조 갱신·atom 값 교체는 GC mutex로 보호한다. 사용자 callback은 이 GC mutex를 잡은 채 실행하지 않는다. 수거 메타데이터 할당이 실패하면 참조를 변경하지 않고 다음 경계에서 다시 시도한다. 이 OOM 경로의 별도 주입 시험은 미실행이다.

완료 worker는 private arena에서 실행하고 결과를 소유한 뒤 callback 참조와 arena를 정리한다. 일반 deref와 시간 제한 deref의 완료 결과는 호출자 arena로 전달한 뒤 worker의 결과 참조를 해제한다. 미완료/포기한 future와 반복·동시 대기의 전체 계약까지 해결했다고 주장하지 않는다.

### 자체·메모리·실제 HTTP 증거

- 일반 lifetime/cycle 시험 `L08_ATOM_LIFETIME_PASS=2 FAIL=0`, 종료 `0`. [일반 원문](evidence/l08-platform/atom-lifetime-after.txt).
- ASan/UBSan에서 반환 atom, 캡처 closure, 컨테이너 별칭, 1024개 강한 참조, 오류, 실제 8-thread×250 scope와 self/two-atom/recursive-closure cycle, 외부 keeper로 유지·해제, 오류 중 cycle을 확인했다. worker 캡처·반환을 추가한 최종 두 fixture도 `L08_ATOM_LIFETIME_PASS=2 FAIL=0`, `ATOM_WORKER_SANITIZER_EXIT=0`이다. [초기 sanitizer 원문](evidence/l08-platform/atom-lifetime-sanitized-after.txt), [최종 worker 포함 원문](evidence/l08-platform/atom-worker-after.txt). 최종 fixture의 단정은 lifetime `19`개, cycle `9`개다.
- native link wrapper는 실제 fl_atom_new 반환 pointer와 free만 계측한다. 수거를 대신하거나 기대값을 낮추지 않았다. 생성 계측이 실제 증가하는지도 단정한다. `GENERAL_LSAN=UNEXECUTED`, `detect_leaks=0`이며 모든 malloc의 누수 해결 증거로 사용하지 않는다.
- 기존 snapshot 자체·native 4-reader×1000회는 `L08_ATOM_SNAPSHOT_PASS=2 FAIL=0`, `MANAGED_SNAPSHOT_SANITIZER_EXIT=0`이다. C fixture의 shared root에만 명시적인 heap copy/release를 추가했으며 기존 루프·이전 값·반환·중첩 arena 단정과 출력 정답은 유지했다. [원문](evidence/l08-platform/managed-snapshot-after.txt). C API caller가 활성 arena 밖에 값을 저장할 때 이 소유권 관리가 필요함을 runtime.h에 명시했다. timer C fixture의 캡처는 이미 소유된 closure가 참조를 보유한다.
- 실제 HTTP worker `4`, handler peak `4`, 동시 curl `10`으로 기존 증가 요청 `1000`회의 counter/고유 값 `1000`을 유지했다. 추가 요청 `100`개×self-cycle `100`개는 arena 정리 후 live `0`; 다음 요청의 keeper 하나만 live `1`, keeper 해제 후 `0`, 실제 생성 계측 `10001`이다. 수정 전 고정 `35a6ec853aaa535fdf284207045043ef7d4ebc37`는 같은 시험에서 `HTTP_LIFETIME_BEFORE_EXIT=1`, 수정 후 `166ef374bb8417f7cd2227f23e3b98de47fe0f84`는 ASan/UBSan에서 `HTTP_LIFETIME_AFTER_EXIT=0`이다. [HTTP 수정 전](evidence/l08-platform/http-lifetime-before.txt), [HTTP 수정 후](evidence/l08-platform/http-lifetime-after.txt). 실제 운영 HTTP나 DB 부하 시험으로 확대하지 않는다.
- seed에서 native compiler를 빌드한 결과도 `MANAGED_NATIVE_COMPILER_BUILD_EXIT=0`이다. [원문](evidence/l08-platform/managed-native-compiler-after.txt). 이는 verify-native-bootstrap.sh 완료 증거가 아니다.

worker sanitizer 명령의 첫 시도는 shell 따옴표 오류로 `DRIVER_COMMAND_EXIT=2`, 프로그램 미실행이었다. 명령을 바로잡은 새 root의 결과가 위 최종 성공이다. [명령 설정 오류](evidence/l08-platform/atom-worker-driver-config-before.txt). 모든 위 실행의 execve 감사는 각각 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다.

```bash
L08_ATOM_LIFETIME_OUT=<가림> L08_ATOM_LIFETIME_COMPILER=<가림>/compiler L08_ATOM_LIFETIME_SANITIZER_LIBDIR=<가림> strace -f -e trace=execve,clone,clone3 -o <가림>/execve.log bash scripts/verify-l08-atom-lifetime.sh
L08_SNAPSHOT_OUT=<가림> L08_SNAPSHOT_COMPILER=<가림>/compiler L08_SNAPSHOT_SANITIZER_LIBDIR=<가림> strace -f -e trace=execve,clone,clone3 -o <가림>/execve.log bash scripts/verify-l08-atom-snapshot.sh
L08_HTTP_ATOM_RUNTIME_ROOT=<가림> L08_HTTP_ATOM_OUT=<가림> L08_HTTP_ATOM_COMPILER=<가림>/compiler L08_HTTP_ATOM_SANITIZER_LIBDIR=<가림> strace -f -e trace=execve,clone,clone3 -o <가림>/execve.log bash scripts/verify-l08-http-atom.sh
```

### 2차 오류 메시지 정답의 공개 정정

이전 고정 `fe6bb4f4f38775404685ccf96e69f8e147d3633c` 전체는 `EXTERN_CURRENT_PLATFORM_FULL_EXIT=1`, `35a6ec853aaa535fdf284207045043ef7d4ebc37` 전체도 `C20_PLATFORM_FULL_EXIT=1`이다. 둘 다 `L08_P2_PASS=9 FAIL=2`에서 멈췄으며 이후 L06/L07·bootstrap은 이 두 전체 실행에서 미실행이다. [첫 전체 실패](evidence/l08-platform/extern-current-full-before.txt), [C20 전체 실패](evidence/l08-platform/c20-full-oracle-before.txt).

원인은 contracts string/nil·source-edge boolean의 산술 타입 오류 정답 3줄이었다. v11 evalBuiltin은 `[E_TYPE_MISMATCH]`를 던지고 evalSExpr는 `[+]`/`[-]`를 앞에 붙인다. catch는 첫 대괄호 접두어만 제거하므로 `[E_TYPE_MISMATCH]`는 남는다. 최초 정답은 evalSExpr 포장을 빠뜨렸다. `1cbd70f8fc773ba41460b0b1c6a1493185a44d9c`에서 원문 파일·줄과 이력을 기록하고 해당 세 줄만 복원했으며, 이 정정을 위해 런타임 오류 출력을 바꾸지 않았다. [근거·이력](../tests/l08-phase2/contract.md). Node는 실행하지 않았다. 해당 고정 source의 별도 phase2 자체 재실행은 `L08_P2_PASS=11 FAIL=0`, `ORACLE_PHASE2_EXIT=0`이다. [원문](evidence/l08-platform/oracle-phase2-after.txt). 이 재실행을 이후 GC 소스의 전체 통과로 확대하지 않는다.

### 최신 실제 저장소·전체 앱 실행

runtime `166ef374bb8417f7cd2227f23e3b98de47fe0f84`와 승인된 source-only 복사본에서 원래 저장소 writer 1회·별도 reader 10회가 ASan/UBSan으로 모두 종료 `0`, `DB95_NATIVE_STORE_PASS=11 FAIL=0`, `MANAGED_BLOG_STORE_EXIT=0`이다. 합성 WAL SHA-256 `77f7bcc7c90b5951331155b33be630547bf6dd464aaa8229ce304f9019361337`, `DB95_NATIVE_WAL_READ_ONLY=PASS`다. [저장소 원문](evidence/l08-platform/managed-blog-store-after.txt).

같은 runtime으로 원래 main.fl·세션 seed·editor-http를 native 링크했고 세 빌드 종료 `0`, `MANAGED_BLOG_EDITOR_EXIT=0`, `NATIVE_BLOG_EDITOR_PASS=3 FAIL=0`, 재시작 `2`, WAL 무변경이다. 매번 홈·로그인 화면·기존 script·글쓰기 HTML HTTP `200`이다. main 실행물 SHA-256 `6a4bc9c438c591c4969b8cd8a5e9e261332286dd13db4e8480550e84a43fdc2c`다. [앱 원문](evidence/l08-platform/managed-blog-editor-after.txt). C 변환에는 GC 본체 단계에서 만든 native compiler를 사용했으며 이후 self compiler 소스는 바뀌지 않았다. 링크는 위 최신 고정 runtime이다.

`AUTHENTICATION_METHOD=SYNTHETIC_PRESEEDED_SESSION`, `PASSWORD_LOGIN=UNEXECUTED`, `EXISTING_EDITOR_JS=SERVED_NOT_EXECUTED`다. 새 JS를 작성하거나 실제 비밀번호 로그인·브라우저 J안·사진 메뉴 실제 조작을 검증한 것이 아니다. 각 execve 감사는 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다.

## 미확인

- 전체 회귀·L06/L07·verify-native-bootstrap.sh는 위 변경과 정정한 정답을 포함한 고정 `166ef374bb8417f7cd2227f23e3b98de47fe0f84`에서 새로 실행 중이다. 이전 전체 실패를 통과로 바꾸지 않는다. native compiler 빌드 `0`과 전체 bootstrap 검증을 구분한다.
- arena 밖 startup/process 전역 소유권, 일반 LSan, 모든 OOM 주입, promise 등 opaque native 객체를 통하는 cycle, 포기한 future·반복/동시 대기·future 예외 전파, 다양한 큰 업로드의 arena cache·장시간 RSS는 남아 있다. 모든 누수·C20 전체 완료로 판정하지 않는다.
- DB 동시 거래·WAL 일관성·복구·성능 목표, 실제 인증·브라우저 J안·이미지 처리·댓글·다중 작성자·나머지 PRD·운영 자료 호환·백업/복구 검증은 남아 있다. 서버 23·25 접속·운영 자료 변경·백업 삭제·합치기·배포는 하지 않았다.

이후 요청 arena cache 보존량·fallback 정리·overflow를 보완하고 실제 업로드·저장소·앱 회귀를 확인했다. 별도 DB 8-worker 시험에서는 성공 반환 8개에도 live 20건과 재생 32건의 불일치를 새로 재현했으며 아직 수정하지 않았다. [후속 cache·DB 원문과 범위](L08-arena-cache-progress.md). 개별 수명 시험 성공을 DB 동시성 완료로 확대하지 않는다.
