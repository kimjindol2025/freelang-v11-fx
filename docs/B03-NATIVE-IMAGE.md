# B03 native image contract

User approval: 2026-11-17 “의존성·API·GIF 준비 변경 승인”.
Base c99b6f7833e7c8e73cfd18946867d927e4228146. Read-only seed SHA:
f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c.

The twenty contracts in tests/image-native/contracts.fl and contracts.expected
are fixed before implementation. No existing test, timeout, runner or seed changes.
Fixture images are synthetic C-generated JPEG baseline/progressive 1600x800,
RGBA PNG 1600x800, tiny PNG 20x10, and two-frame GIF; corrupt inputs are truncated
versions and a fake GIF signature. No original production data or keys.

## Interface

image-info-result(path, options), image-process-result(path, outputs, options).
Result is {ok:boolean, error:string, data:map-or-nil}. All invalid input and runtime
failures are observable; no silent exception suppression. Options are required:
max_pixels, max_bytes, memory_bytes, timeout_ms, jpeg_quality (positive integers,
quality 1..100). Optional expected_mime rejects a mismatch. Upper safety bounds:
512MiB worker address space, 5000ms wall time; file contract's existing max remains
unchanged. D05 passes 20,000,000 pixels, 5242880 bytes, 536870912 bytes, 5000ms, 85.
outputs is a nonempty vector of at most two {path,long_edge} maps. Only PNG/JPEG
derivatives, GIF first-frame PNG, aspect retained, no crop/enlarge. On success data
has width,height,mime,alpha,frames and process jpeg_quality and outputs metadata.

Root is existing FL_FILE_BASE, required for image APIs. No traversal/symlink,
nonregular input or output overwrite. Parent securely opens fd-relative paths,
worker receives only explicit descriptors and numeric options, never shell text.
Worker is configured by trusted FL_IMAGE_WORKER absolute executable environment.
A root-scoped lock serializes processes. Parent enforces deadline including wait,
kills/reaps a failed child, removes unpublished/partially published outputs, and
returns an error. Output publication uses exclusive atomic links, no overwrite.

The codec worker is separate so the compiler/runtime have no new codec link ABI.
Build scripts fetch pinned upstream archives to the caller's isolated build tree,
retain licenses, and link only the worker to static libraries. No system install.

IMG19/20 also require supplementary native supervisor evidence: deadline kill,
worker exit, output cleanup, concurrency one, fd/child stability and repeated-job
leak checks. contracts.fl alone does not establish those resource properties.

Contract commit: ef354a55aa12accea3447031821b9d178c647aec. Before implementation
the native execution exited 1: Function not found: image-info-result (line 6).
Initial integration retained two failures (GIF error precedence, output-path errno)
and corrected their causes without modifying the fixed contracts. Then 20/20 passed
(0.58 seconds, 10496KiB); resource supervisor 29/29, 200 repeat jobs, 4 concurrent
requests. This intermediate run precedes bounded input snapshot hardening; final
retest, language regressions, leak sanitizers and bootstrap are still required.

Pinned archive SHA-256 (source archives, not generated binaries):

| Library | SHA-256 |
|---|---|
| zlib 1.4 | 02dd159783677a749c04adbdd02b55b8a2bcc7e054f51b76b549199a916ad584 |
| libpng 1.6.59 | 86a3e4b501f7f50e392c4e456ea158893e9a595b1c63eb88c7bb9f6cf8772dad |
| libjpeg-turbo 3.2.1 | c27ad72a68da151177929aa969ae773a2da5ac2af9ce946079314fdac8cece7f |
| GIFLIB 6.1.3 | b65b66b99f0424b93525f987386f22fc5efb9da2bfc92ad4a532249aaffbab0e |

The isolated prefix contains license notices for all four. The worker uses explicit
static .a paths, and ldd must confirm no dynamic codec dependency. Build with
`bash scripts/build-image-deps.sh ISOLATED_BUILD_ROOT` and then
`bash scripts/build-image-worker.sh ISOLATED_BUILD_ROOT/prefix WORKER_OUTPUT`.
Caller supplies TMPDIR in the task area, shared heavy lock, ps and memory cap.
Set trusted FL_IMAGE_WORKER to the worker executable and FL_FILE_BASE to the
application sandbox. Missing worker configuration produces WORKER_UNAVAILABLE.

## 최종 실행 기록 — 제품 연결 전 보류

실행 소스는 `73885b248f472c49185255b1c7515b3a5fef4c54`이다. 고정 정답 20개는
수정하지 않았다. [감사 JSON](B03-NATIVE-IMAGE.json)에 원문·소스 SHA를 연결했다.
이 소스로 새 API·자원·추가 오류·bootstrap을 최종 확인했다. 아래 기존 회귀는
런타임이 같은 `a3de34169cc0f832daa7131af860076a2e201938`·
`d2f655a58302ea413416c5cee26413542bd95597`에서 수행했다. 이후 새 이미지 API의
정리 분기만 고쳤으며 최신 커밋의 기존 전체 회귀를 완주했다고 주장하지 않는다.

| 검사 | 실제 결과 |
|---|---|
| 현재 소스 재빌드·`.fl` 고정 정답 | 20/20, `ELAPSED=0.59 MAX_RSS_KB=10460 EXIT=0` |
| 격리·오류·픽셀·동시 요청 자원 단언 | 일반 32/32, ASan·UBSan·LSan 32/32, sanitizer stderr 0바이트 |
| 반복·동시 처리 | 200회 반복, 동시 요청 4개를 처리기 1개로 직렬화, FD·자식·임시 파일 잔류 0 |
| 원본 | 독립 fixture 생성 원본과 처리 후 JPEG·PNG·GIF byte/SHA가 모두 같음, 3/3 |
| 추가 삭제 오류 주입 | 수정 전 2개 실패 원문 보존, 수정 후 오류 표시·일시 오류 복구 2/2 |
| 기존 CLI / 저장소 언어 API / L07 | 4/4 / 실제 Chrome 20/20 / 221 통과·0 실패 |
| 기존 누수 / UTF-16 / 취소 Chrome | 12/12 / J_UTF16_LENGTH 통과 / 20/20 |
| 전체 native 회귀 | 앞선 21개 그룹 완료. TLS 키 생성 전 정책 확인 때문에 중단, 전체 통과 아님 |
| E09 Chrome C 판독기 | 현재 소스에서 2회 같은 `DOM.describeNode -32000` 오류, 세 번째 실행 안 함 |

E09는 원래 c99b6f78 컴파일러·런타임·같은 판독기로 한 번 20/20 통과했다.
원래 컴파일러와 새 컴파일러의 생성 JS는 바이트가 같다. 공통 C 판독기는
문서 갱신 후 querySelector만 재시도하고 describeNode는 즉시 실패한다.
문서 전환 경합이 유력하며, 실패 실행의 이벤트 순서까지 관찰했다고 주장하지 않는다.
실제 문서 갱신 뒤의 정확한 오래된 node 오류만 기록·재조회하는 7줄 수정안을
보존했지만 기존 시험·러너 변경 금지 예외 승인을 받기 전에는 적용하지 않았다.

TLS는 B16의 키 위치 조건과 현재 임시 작업 영역 조건이 충돌해 예외 승인 대기다.
키 생성 전에 해당 실행의 격리 namespace만 종료했다. 종료 137은 이 의도적인
중단이며 OOM이나 제품 회귀 판정이 아니다. 생성한 TLS 키 파일은 0개다.
완료한 그룹과 미실행인 후속 그룹을 합쳐 전체 회귀 통과로 세지 않는다.

삭제 syscall 실패가 성공 값으로 묻힐 수 있는 점은 별도 재현 정답을 먼저 커밋한 뒤
고쳤다. 정리 오류는 `CLEANUP_FAILED`와 원인 코드로 드러내고 일시적 삭제 실패를
한 번 재시도한다. 되돌릴 수 있는 새 출력은 제거하며 원본은 수정하지 않는다.
영구 파일시스템 오류까지 정리를 보장한다고 주장하지 않으며 오류를 반환한다.

최종 bootstrap은 strace 없이 실행했고 생성 C SHA는 세 단계 모두 다음과 같다.

```text
STAGE2_SHA=f38f373890d0898552cbe89768b992f934e9c4a567bf27cdf66248a3c1b5cb81
STAGE3_SHA=f38f373890d0898552cbe89768b992f934e9c4a567bf27cdf66248a3c1b5cb81
STAGE4_SHA=f38f373890d0898552cbe89768b992f934e9c4a567bf27cdf66248a3c1b5cb81
SEED_SHA_BEFORE=f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c
SEED_SHA_AFTER=f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c
```

출발 c99b6f78의 B18 기록과 같은 고정점이며 ELF SHA로 판정하지 않았다.
처리기 ELF SHA는 `a192120e8463071a2ddce30100119289a4b0cccababb070b448363b25dfbd4f1`이다.
완료 native 실행 trace 범위의 node/npm/npx/JS/Python은 각각 0이다.
bootstrap은 승인된 정적 감사이며 동적 실행 수는 미관찰이다. 의존성 빌드의
동적 전체 실행 수도 기록하지 않았으므로 trace 관찰 범위와 구분한다.

언어 성과: `.fl`이 실제 JPEG·PNG·GIF를 별도 C 처리기로 검증·축소하고, 자원 상한·오류·
원본 보존을 값으로 확인할 수 있다. 시스템 설치·새 JS·seed 교체는 하지 않았다.
전체 회귀와 위 두 예외 확인이 남아 블로그에 아직 연결하지 않았으며 B03 PASS는 없다.
