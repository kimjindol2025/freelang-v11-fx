# L06 파일 업로드 구현·검증 보고

작성일: 2026-10-29 (시험: 2026-10-28~2026-10-29) · 가지: `feat/l06-multipart` · 보고는 언어 저장소의 네이티브 기능에 한정한다. 합치기·배포하지 않았다.

## 확인한 것

### 1. 기준·정답 선행·구현 커밋

| 순서 | 전체 SHA | 내용 |
|---|---|---|
| 기준 | `90d8d8d97e4e2a0e77ddb30cf112ead4025cd70f` | L06 초안. L07 기준은 `8379fd76adc6c2934c9d056d72b4f94b9f4226d0` |
| 첫 정답 선행 | `3b417d84a68b76bf4f829e33a76deb0d58ba649f` | 실패 재현 `.fl`·독립 정답·계약, 파일당 15 MiB·전체 64 MiB·파일 4개 반영 |
| 추가 정답 선행 | `16c07d7d18c18ec3e7b508eaa8a22fb0d6ff0c8c` | 실제 HTTP/TLS 자체 시험, 닮은 종료 경계 표본 고정 |
| 추가 정답 선행 | `f1e485cf7f36efe4b3d8bb934d4b97cf1a57fed9` | TE/중복 Content-Length가 용량 오류보다 먼저 400이 되는 정답 고정 |
| 구현 | `a0bb565eae356781448c31a066fad1ee946621d7` | multipart·URL 인코딩 폼·수신 길이·임시 저장/정리·공개 getter·로그·arena 수정 |
| 추가 정답 선행 | `db0b8bfa2b0afc689d9da4152ff1edec422c8228` | 파일 안의 WebSocket/SSE 문자열과 기존 SSE 경로 회귀 |
| 프로토콜 판정 수정 | `25af8ac3dba454930b57ed3647c9e60b616d5bce` | GET 헤더만 판정해 업로드 바이트 오인 차단 |
| 동시 시험 선행 | `ea2d9244a134f7ee166875ffae2ecd808ce86247` | 100회 반복 동시 요청과 실패 응답 진단 |
| 경쟁 정답 선행 | `6c2adec3635d5b5e8f0973b1b386455eec9cbb55` | 정리 중 소실되는 marker의 실제 HTTP 재현 |
| 최종 구현 | `f02920bd8fd1f6d7412bce76fd1e1438d8575b83` | 소유 marker의 ENOENT 정리 경쟁 처리 |

[첫 `.fl` 재현](https://github.com/kimjindol2025/freelang-v11-fx/blob/3b417d84a68b76bf4f829e33a76deb0d58ba649f/tests/l06/repro.fl), [고정 정답](https://github.com/kimjindol2025/freelang-v11-fx/blob/3b417d84a68b76bf4f829e33a76deb0d58ba649f/tests/l06/repro.expected), [승인 상한을 반영한 초안](https://github.com/kimjindol2025/freelang-v11-fx/blob/f02920bd8fd1f6d7412bce76fd1e1438d8575b83/docs/L06-multipart-instruction-draft.md).

`repro.expected`는 수정 전 커밋 이후 변경하지 않았다. SHA-256은 `bbbe53b269f6a96dfa7818d6d7acf8c2dd4984a65ac45227f915bd376be12b1b`이다. 추가 프레이밍 정답의 SHA-256은 `5ff594b4110948f9311756cd6e435ad4e4faea3b7d3733f3d7b1af3e1bd46078`이다. 기존 L07 시험·정답, 기존 JIT·GC 시험 러너, `verify-native-bootstrap.sh` 원문을 변경하지 않았다.

### 2. 수정 전 실패 원문

[첫 실패 출력](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l06-multipart/docs/evidence/l06/before-repro.txt):

```text
HTTP=422
FORM_MAP=FAIL
EXIT=1
```

없는 새 API의 컴파일 오류를 실패 재현으로 쓰지 않았다. 기존 `(get req "body")`를 사용하는 `.fl` 서버에 실제 loopback URL 인코딩 폼을 보냈고, 본문이 맵이 아니라는 실패를 먼저 확인했다.

개발 중 두 오류도 정답을 먼저 고정한 뒤 수정했다.

- [닮은 종료 경계 실패](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l06-multipart/docs/evidence/l06/before-closing-prefix.txt): `FAIL false-boundary-prefix-preserved`, `L06_HTTP_PASS=102 FAIL=1`, `EXIT=1`. 파일 안의 `--B--X`를 정상 종료 경계처럼 취급했다. [보존한 개발 바이너리의 HTTP 400](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l06-multipart/docs/evidence/l06/before-prefix.http.txt)과 [수정 후 HTTP 200](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l06-multipart/docs/evidence/l06/after-prefix.http.txt)을 함께 남겼다.
- [프레이밍 실패](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l06-multipart/docs/evidence/l06/before-framing.txt): `TE_CL_OVERSIZE_HTTP=413`, `DUPLICATE_CL_OVERSIZE_HTTP=413`, `EXIT=1`. 잘못된 프레이밍은 400이어야 하는데 용량 상한을 먼저 검사했다.

- [프로토콜 본문 오인 실패](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l06-multipart/docs/evidence/l06/before-protocol-dispatch.txt): `PROTOCOL_BODY_WS_HTTP=0`, `PROTOCOL_BODY_SSE_HTTP=200`, `L06_HTTP_PASS=106 FAIL=2`, `EXIT=1`. 파일 안의 WebSocket 문자열은 무응답, SSE 문자열은 파일 대신 다른 내용을 반환했다. HTTP 200만으로 성공 판정하지 않는 이유다. 판정을 GET 요청의 헤더에만 한정하고 원래 파일 바이트를 복원해 처리했다.
- [TLS 동시 요청 실패](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l06-multipart/docs/evidence/l06/tls-concurrency-failure.txt): 한 실행에서 `L06_TLS_PASS=107 FAIL=1`이었다. 최초 실패 당시 개별 응답 세부는 수집되지 않아 그 1건의 원인을 단정하지 않는다. 이후 marker가 readdir와 openat 사이에 정상 정리되는 경쟁을 [실제 TLS 최소 재현](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l06-multipart/docs/evidence/l06/before-recovery-race.txt)으로 고정했다. `FAULT_DETAIL RECOVERY_ENOENT HTTP=500 BODY={"error":"UPLOAD_CLEANUP_FAILED"}`, `L06_TLS_PASS=109 FAIL=1`, `EXIT=1`을 확인했다. 이미 사라진 marker의 ENOENT만 정상 정리로 취급하도록 수정했고 다른 열기 실패는 계속 오류로 남긴다. 최종 시험에서는 HTTP와 TLS 각각 100회 반복 동시 요청·소실 경쟁 주입이 모두 통과했다.

### 3. 구현한 계약

[파서·수신·임시 저장](https://github.com/kimjindol2025/freelang-v11-fx/blob/f02920bd8fd1f6d7412bce76fd1e1438d8575b83/runtime/http-form.h), [HTTP/TLS 공통 처리](https://github.com/kimjindol2025/freelang-v11-fx/blob/f02920bd8fd1f6d7412bce76fd1e1438d8575b83/runtime/http.c), [네이티브 compiler 연결](https://github.com/kimjindol2025/freelang-v11-fx/blob/f02920bd8fd1f6d7412bce76fd1e1438d8575b83/self/cgc-main.fl)을 수정했다.

- 본문 전체 **64 MiB**, 파일 하나 **15 MiB**, 파일 **4개**다. multipart 부가 바이트도 전체 본문에 센다. 기존 필드·파트·헤더 상한도 적용한다.
- `server_req_fields`는 필드 맵, `server_req_files`는 전송 순서의 파일 벡터를 돌려준다. 밑줄·하이픈 호출과 `server_req_body`를 `.fl`에서 시험했다. URL 인코딩 폼과 multipart의 `body`는 필드 맵이다. 중복 텍스트 이름·텍스트/파일 이름 충돌은 400, 동일 이름의 여러 파일은 허용한다.
- Content-Length는 헤더 안에서만 정수·중복·범위를 검사한다. 선언량 미달·중단·시간 초과를 성공으로 보내지 않는다. TE+CL은 400, 길이 없는 폼은 411, TE 단독은 미지원 501이다. 폼 연결은 닫고 뒤에 붙인 요청을 실행하지 않는다.
- 파일 데이터는 명시적 길이로 다루며 NUL·0~255·비UTF-8을 보존한다. `.fl`이 L07 `file-read-result`·`file-copy-result`를 실제 호출한다.
- 저장 경로를 다시 열 때의 경쟁을 피하려고 L07 checked writer에 **내부 reserved-FD 진입점**을 보완했다. 이 진입점은 기존 `file_write_all`·flush·close·결과 맵 처리를 재사용한다. public `file-write-result`·`file-append-result`를 경로로 다시 호출하는 방식은 쓰지 않았다. 새 바이트 저장기를 별도로 만들어 L07 검사를 우회한 것이 아니다. [io.c 근거](https://github.com/kimjindol2025/freelang-v11-fx/blob/f02920bd8fd1f6d7412bce76fd1e1438d8575b83/runtime/io.c).
- 원본 파일명을 경로에 쓰지 않는다. 경로 구분자·드라이브·NUL·제어문자를 거부하고 무작위 16자리 hex 이름을 만든다. `%HH` 파일명은 디코딩하지 않는 표시용 값이다. FL_FILE_BASE 아래의 디렉터리 FD·no-follow·독점 생성·0600 파일을 사용한다.
- `O_TMPFILE`로 만든 inode를 잠근 뒤 소유 marker와 파일명을 hard link로 공개한다. 요청 종료·파싱/쓰기 실패·403·예외·선삭제 뒤 해당 요청 파일만 정리한다. 강제 종료 뒤에는 잠기지 않은 marker와 같은 inode만 회수한다. 다른 인스턴스의 잠긴 파일과 영구 복사본은 보존한다.
- HTTP와 TLS가 같은 수신·폼 파서·정리 경로를 거친다. 단순 소켓 timeout의 지연을 관측해 nonblocking 수신·monotonic deadline·poll로 절대 시간 제한을 적용했다.
- 디버그 폼 본문을 출력하지 않는다. 폼 handler 예외 메시지도 `Internal error`로 가려 입력 값이 자동으로 노출되지 않게 했다. 요청마다 커지는 업로드에서 arena가 작은 청크를 재사용할 수 있는 문제도 수정했다.

이는 새 `.fl`·C·Bash 시험과 C 런타임 변경이다. 새 TS·JS·Python 파일, Python 빌드 도우미, Node 기반 FreeLang 실행기를 추가하거나 사용하지 않았다.

### 4. 수정 후 통과 원문·명령·종료 코드

아래는 실제 실행 명령에서 로컬 경로만 `<가림>`으로 표시한 것이다. 별도 시스템 설치나 sudo 없이, L07 때 준비한 시스템 libcurl 8.9.1과 같은 버전의 헤더·링크 디렉터리를 사용했다.

```bash
export CPATH=<가림>/curl-8.9.1/include LIBRARY_PATH=<가림>/lib
L06_OUT=<가림> bash scripts/verify-l06-all.sh
```

이 드라이버는 재현 → HTTP/TLS → 기존 회귀 → 부트스트랩 순서다. 앞의 세 단계는 각각 `strace -f -e trace=execve`로 전체 빌드·자식을 추적한다. 부트스트랩은 기존 검증기가 이미 strace를 쓰므로 기존 L07 보관 도우미로 GCC·네이티브 추적을 보관한다. [실제 드라이버](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l06-multipart/scripts/verify-l06-all.sh), [전체 최종 종료 코드](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l06-multipart/docs/evidence/l06/final-summary.txt).

| 시험 | 실제 결과 | 종료 코드·근거 |
|---|---|---|
| 동일 `.fl` 재현 | `HTTP=200`, `FORM_MAP=PASS` | `REPRO_EXIT=0` · [원문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l06-multipart/docs/evidence/l06/REPRO.txt) |
| 실제 loopback HTTP | `L06_HTTP_PASS=110 FAIL=0` | `HTTP_EXIT=0` · [원문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l06-multipart/docs/evidence/l06/HTTP.txt) |
| 실제 loopback TLS | `L06_TLS_PASS=110 FAIL=0` | 같은 HTTP/TLS 단계 `0` · 같은 원문 |
| 실제 시간 초과 | `L06_TIMEOUT_PASS=2 FAIL=0` | 같은 단계 `0` · 같은 원문 |
| L07 자체 시험 | `L07_SELF_PASS=221`, `L07_SELF_FAIL=0` | `REGRESSIONS_EXIT=0` · [원문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l06-multipart/docs/evidence/l06/l07-self.txt) |
| 기존 C 런타임 회귀 | `PASS` 53건, `FAIL` 0건 | 같은 회귀 단계 `0` · [원문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l06-multipart/docs/evidence/l06/runtime-regression.txt) |
| 기존 JIT·GC 러너 | `결과: 10 PASS / 0 FAIL / 10 총` | 같은 회귀 단계 `0` · [원문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l06-multipart/docs/evidence/l06/REGRESSIONS.txt) |
| 네이티브 부트스트랩 | `STAGE2_EQ_STAGE3=PASS`, `RESIDUAL_PROCESSES=0` | `BOOTSTRAP_EXIT=0` · [원문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l06-multipart/docs/evidence/l06/BOOTSTRAP.txt) |
| 최종 드라이버 | `L06_NATIVE_GATES=PASS` | `ALL_EXIT=0` |

JIT·GC 시험은 과거 바이너리를 실행한 것이 아니라 현재 네이티브 compiler·C 런타임·jit.c로 다시 빌드하고 기존 러너·기대값을 그대로 사용했다. C 런타임 회귀의 고정 임시 파일 이름만 기존 L07 방식으로 격리했다.

시험 본체는 [FreeLang 서버](https://github.com/kimjindol2025/freelang-v11-fx/blob/f02920bd8fd1f6d7412bce76fd1e1438d8575b83/tests/l06/server.fl)이며, [실제 소켓 클라이언트](https://github.com/kimjindol2025/freelang-v11-fx/blob/f02920bd8fd1f6d7412bce76fd1e1438d8575b83/tests/l06/client.c)는 C 파서를 직접 호출하지 않는다. L07 read 결과를 `.fl`에서 검사하고 독립 C 원본 해시·바이트 패턴과 대조했다. 파일당 상한 전후·4개 전체 크기·64 MiB 본문 전후·NUL·조각 수신·잘못된 경계·경로 이탈·여러 파일·장애·정리·기존 JSON을 실제 요청했다. TLS 의존성이 있어 **TLS 시험을 실행했다**. 자체서명 인증서를 쓴 로컬 전송 시험이며 운영 인증서 검증은 아니다.

[A NUL B의 실제 HTTP 응답](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l06-multipart/docs/evidence/l06/after-nul.http.txt)은 `HTTP/1.1 200 OK`, `size:3`, `bytes:3`, `ok:true`, `independent:true`, SHA-256 `76fe3925c7167317f2df68454339f5ec3650e4062178b4f2be219b105a507907`이다. 이 응답의 `pattern:false`는 0~255 순환 fixture 전용 비교 값이며 A NUL B의 실패를 뜻하지 않는다.

### 5. 시간 제한·중단·재시작·정리

[최종 HTTP 로그](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l06-multipart/docs/evidence/l06/HTTP.txt)의 원문이다.

```text
TE_CL_OVERSIZE_HTTP=400
DUPLICATE_CL_OVERSIZE_HTTP=400
TIMEOUT_IDLE_HTTP=408 SECONDS=60.014
TIMEOUT_TRICKLE_HTTP=408 SECONDS=60.002
L06_TIMEOUT_PASS=2 FAIL=0
DISCONNECTED_SENDER_RECOVERY=PASS
LIVE_INSTANCE_TEMP_PRESERVED=PASS
SIGKILL_RESTART_OWN_TEMP=0
RESTART_HTTP=200
PERMANENT_COPY_PRESERVED=PASS
STORAGE_missing_HTTP=503
STORAGE_symlink_HTTP=503
```

`WRITE`, `SHORT`, `FLUSH`, `CLOSE`, `ALLOC`, `UNLINK`, `RANDOM`, `WRITE_SECOND`, `RECOVERY_ENOENT`는 시험 실행물에만 링크한 장애 대역이다. 런타임에 환경변수 기반 장애 스위치를 넣지 않았다. 두 번째 파일 쓰기 실패 뒤 첫 파일도 정리됐고, unlink 실패는 HTTP 500·잔여 파일을 숨기지 않은 뒤 다음 요청에서 회수했다. 독점 생성 충돌에서 기존 sentinel도 보존했다. 강제 종료한 것은 자신이 띄운 시험 서버뿐이다.

### 6. 부트스트랩 첫 실패와 PID 격리

최초 전역 실행은 고정점까지 통과했지만 `RESIDUAL_PROCESSES=FAIL`, `BOOTSTRAP_EXIT=1`이었다. [첫 실패 원문](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l06-multipart/docs/evidence/l06/bootstrap-global-scan-failure.txt), [읽기 전용 프로세스 확인](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l06-multipart/docs/evidence/l06/global-scan-processes.txt).

전역 pgrep이 다른 작업의 명령줄에 있는 `stage2-bin` 인자를 잡았다. 확인 당시 PID는 `2321074`, `2321078`, `2321079`였다. 이 프로세스들을 종료하지 않았다. 검증기 원문·정답을 바꾸지 않고 다음과 같이 **검증 작업만** PID/user namespace에서 다시 실행했다. 최종 드라이버에도 같은 격리를 적용했다.

```bash
unshare --user --map-root-user --pid --fork --mount-proc \
  env L07_OUT=<가림> bash scripts/verify-l07-bootstrap.sh
```

최종 `BOOTSTRAP_EXIT=0`이며 Stage 1·2·3 생성 C SHA-256은 모두 다음과 같다.

```text
626cdc8b0fb6e938c759f9dc70a09562e711600903c48c416864e1f52102d9af
```

`STAGE0_EQ_CANONICAL=DIFFERENT`는 기존 검증기의 보존 seed와 과거 canonical 차이 표시다. seed를 바꾸지 않았다. `CLEAN_CHECKOUT=PASS`도 검증기의 기존 출력 이름이며 시험 시작 시 Git 작업 트리가 깨끗했다는 별도 주장으로 사용하지 않는다.

### 7. strace·화면 기록·금지 사항

[전체 추적 파일](https://github.com/kimjindol2025/freelang-v11-fx/tree/feat/l06-multipart/docs/evidence/l06/traces)과 [보관본 재집계](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l06-multipart/docs/evidence/l06/archive-trace-audit.txt):

```text
NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0
```

위 0은 **FreeLang 네이티브 빌드·시험과 그 자식**의 execve 집계다. 화면 기록·워크스페이스 조회에 사용한 기존 Orca CLI는 Node 기반 외부 UI 도구이며 이 추적 범위 밖이다. 전체 작업 세션에서 Node 프로세스가 전혀 없었다는 보고가 아니다.

[실제 터미널 화면 기록](https://github.com/kimjindol2025/freelang-v11-fx/blob/feat/l06-multipart/docs/evidence/l06/terminal-screen.json)은 Orca `terminal read --screen`의 `source:screen` 렌더링 텍스트 JSON이다. 원문 결과를 표시한 터미널을 직접 읽고 로컬 경로만 가렸다. PNG를 만들었다고 주장하지 않는다. 이 작업용으로 만든 터미널만 닫았으며 다른 터미널·프로세스는 종료하지 않았다.

코드·증거 diff의 민감 값 패턴 검사와 경로 가림 검사를 통과했다. 로컬 경로 외 상태·숫자·해시는 원문이다. 서버 23·25에 접속·작업하지 않았고 운영 WAL·이미지·원본 백업을 변경하지 않았다. 합치기·배포·강제 푸시하지 않았다. Forgejo → GitHub 순서의 같은 가지 푸시와 두 최종 HEAD는 이 보고서 커밋 후 원격 ref를 읽어 별도 보고한다.

## 미확인·지원 제한

- 블로그 실연동·실데이터 이전(L08), 운영 배포·관리자 화면 시험은 하지 않았다. 언어 기능 시험 결과를 블로그 이전 완료로 확대하지 않는다.
- 본문은 최대 64 MiB를 메모리에 수신한다. 스트리밍 업로드나 장시간·대규모 동시 부하의 메모리 사용량은 검증하지 않았다. 여러 worker가 큰 본문을 함께 받으면 메모리 사용량이 커진다.
- Linux x86_64·GCC `14.3.1`·OpenSSL `3.5.1` 환경에서 시험했다. O_TMPFILE·hard link·flock을 지원하지 않는 파일시스템은 이 저장 방식을 지원하지 않으며 다른 OS·파일시스템은 미확인이다. namespace 재검증에는 unprivileged user/PID namespace 지원도 필요하다.
- chunked·중첩 multipart·지원 외 charset/전송 인코딩·반복 텍스트 필드는 지원하지 않는다. 일반 HTTP/1.1/MIME 전체 호환을 주장하지 않는다.
- PNG/GIF/JPEG 시험 표본은 시그니처 바이트 운반 표본이다. 완전한 이미지 디코딩 유효성·리사이즈·악성 이미지 판별은 시험하지 않았고 이 항목에서 구현하지 않았다. 파일명 표시 시 HTML escaping·인증·CSRF는 호출자 정책이다.
- 원자적 영구 저장·fsync·WAL 거래는 보장하지 않는다. handler의 DB/영구 파일 부작용을 HTTP 런타임이 되돌린다고 보고하지 않는다.
- 기존 `semantic-tests/run-all.sh`는 Python·Node 실행을 직접 요구하므로 실행하지 않았다. 이 보고서는 이를 포함한 저장소 모든 시험의 통과 보고가 아니라, 위에 열거한 네이티브 자체 시험·기존 네이티브 회귀·부트스트랩의 결과다. 화면은 렌더링 텍스트 기록이며 PNG 캡처는 제출하지 않았다.
