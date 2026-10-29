# L08 잔여 호환 계약과 통합 검증

## C18: 먼저 고정한 정답

기준 v11 커밋 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4`의 `src/stdlib-process.ts`와 `bootstrap.js`의 `createErrorModule`·`uuid_short`를 Git 텍스트로 조사했다. Node를 실행하지 않았다.

- env_get: 누락·빈 값은 nil. env_or: 누락·빈 값은 주어진 기본값, 나머지는 문자열. 일반 함수 인자로 기본값 표현식도 평가한다.
- error_message: 문자열 그대로, truthy map.message는 JS String 변환, 나머지는 JS String 변환. nil은 null, 일반 map은 [object Object], 배열 message는 쉼표 연결이다. native에서 throw 값은 catch에 그대로 전달된다.
- process_exit: 생략 시 0, 명시 7이면 7. 시험 자식만 종료하며 후속 출력은 없어야 한다.
- uuid_short: 암호학적 난수 4바이트의 소문자 hex 8자리. 512개 표본 형식·중복 확인은 난수 품질의 수학적 증명이 아니다.

시험 `.fl`과 기대 출력을 구현 전에 커밋한다. 기존 native C ABI는 보존하고 public v11 별칭을 보완한다. 운영 환경 변수·데이터·키는 시험 입력으로 쓰지 않는다.

## 전체 목표의 남은 검증

C11 fd 내구성, C12 WAL JSON·해시, C13 인증, C14 HTTP, C15 폼, C16 HTTP 클라이언트, C17 feed, C18 환경, C19 타이머/TCP/TLS, C20 동시성의 언어 시험을 각각 요구한다. 이 문서와 C18 통과는 다른 항목이나 blog1 전체 통합의 완료 증거가 아니다.

## 전체 컴파일에서 추가 확인한 C01·C02 차이

C18과 별도로 전체 앱의 실제 실패에서 최소 재현을 추출했다. 기존 자체 시험만으로 전체 load 호환을 판정하지 않는다.

- db95 `engine-order-rows`의 map 리터럴 안 lambda가 외부 column·collation을 캡처하지 못한다. `cgc-collect-vars`가 Map 블록에 Array 전용 `get-block-items`를 적용하는 것이 원인이다. 언어 쪽에서 수정하며 앱 코드를 평탄화하지 않는다.
- v11 `bootstrap.js:2070~2100`은 `(catch [error] ...)`와 `(catch error ...)`를 모두 받는다. `evalTryBlock`은 catch 변수에 message·type·raw를 가진 객체를 넣는다. fx의 binding pass는 vector catch 변수를 읽지 못하며 기존 catch는 raw 값을 그대로 반환한다.
- C18의 `(error_message e)` 시험은 raw 문자열과 v11 오류 객체 모두에서 같은 결과이므로 catch 구조 호환의 증거가 아니다. catch 객체·범위·finally는 별도 시험으로 보완한다. 예전 음성 시험이 raw catch 문자열을 전제한다면 source 계약 근거와 변경 이력을 남겨 입력을 보완하며 정답을 구현에 맞춰 바꾸지 않는다.

`compiler-blockers.fl`과 기대 출력은 위 재현의 수정 전에 고정한다. 이 파일은 C18 자체 시험 집계에 넣지 않는다.

## C11 fd API의 선행 정답

v11 동일 커밋의 `src/stdlib-fd.ts`가 근거다. fd_open은 r/w/a만 받으며 1000부터 증가하는 synthetic handle을 반환한다. fd_write/fd_fsync/fd_close/fd_flush의 성공 결과는 바이트 수가 아니라 true다. fd_read는 실제 읽은 바이트의 UTF-8 텍스트이며 EOF는 빈 문자열이다. fd_seek는 이름과 달리 offset/whence를 무시하고 fstat의 size를 반환하며 커서를 이동하지 않는다. 이 특성은 호환 계약으로 유지하고 임의의 POSIX seek 의미로 바꾸지 않는다. 실제 바이너리 파일은 기존 L07 API를 사용한다.

writer/reader는 별도 native 과정에서 합성 image-add/image-delete 기록을 append·fsync·close하고 재시작 뒤 바이트에 해당하는 텍스트·EOF·closed handle·잘못된 모드를 검사한다. 이는 fd 자체 시험이며 db95 WAL 재생 성공과 같지 않다. db95의 checksum·잘린 끝·추가/삭제 반복 재생 검증은 C12·전체 통합에서 별도 증명해야 한다.

구현에서는 C10의 경로 보호, 핸들 잠금, 부분 쓰기와 EINTR 처리, fsync 실제 syscall, close 오류 시 핸들 폐기를 보완한다. 실패·부분 쓰기·EINTR·fsync/close fault injection도 `.fl` 경로를 통해 검사한다. 기본 writer/reader와 정답을 먼저 고정하며, 이 문서 작성은 아직 fd 구현 완료가 아니다.
### catch 원문 계약에 따른 시험 입력 보완

기존 callbacks/order/contracts/source-edge/dates의 `(catch e e)`는 오류 자체를 출력해 raw 문자열이라는 fx의 차이를 고정했다. 원래 확인하려던 메시지 정답을 보존하며 `(catch e (error_message e))`로 입력을 바꾼다. regex/regex-guards도 `e` 대신 `error_message e`에 오류 접두사·진단을 검사한다. 기대 출력 파일은 변경하지 않는다. strict-fields는 structured native Error의 code/type/message/hint를 그대로 검사한다. 일반 사용자 map은 message/type/stack/raw만 catch envelope에 옮긴다. 근거는 v11 `evalTryBlock`의 native Error와 일반 object 분기이며, 새 compiler-blockers/try-scope 시험은 message/type/raw·중첩 catch·finally를 직접 검사한다.

### 정답 정정 이력: evalThrow 경로 누락

동일 v11 커밋 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4`의 `bootstrap.js:38043~38055` (`evalThrow`)를 추가 조사했다. 이전 최소 시험은 catch만 읽고 그 앞의 throw 변환을 빠뜨렸다. 문자열·수·일반 map을 `.fl throw`하면 새 Error가 만들어지므로 catch.raw는 원래 문자열·map 자체가 아니라 Error이며 raw.message를 조회해야 한다. truthy map.message가 없으면 Error.message는 String(map)인 `[object Object]`다. 사용자 map.type·extra는 전달되지 않는다. compiler-blockers의 raw 확인 입력과 try-scope의 map/수 기대값을 이 근거로 정정했다. 기존 잘못된 커밋과 정답은 이력에 보존한다.

`bootstrap.js:37969~38020`의 native Error 분기는 `FreeLang line 숫자:`를 제거하고 `[코드]` 접두사·`(at line 숫자...)`를 message에서 제거하며 line을 추출한다. 따라서 phase2 contracts.expected의 2행과 source-edge.expected의 1행에서 `[E_TYPE_MISMATCH] ` 접두사를 제거했다. 현재 구현 출력에 맞춘 변경이 아니며, 정정된 정답은 수정 전 후보에서 실패함을 확인한 뒤 Error 포장·메시지 정규화를 구현한다. 새 시험에서 raw.message와 catch.message를 구분한다.

### C11 강제 오류 시험 정답 선행 고정

`fd-faults.fl`은 공개 fd API를 호출하고 C LD_PRELOAD 보조기는 시험 소유 `fault.wal`의 syscall만 바꾼다. 다른 파일·과정에는 적용하지 않는다. 첫 write EINTR+2바이트 부분 쓰기, write 0, fsync EINTR/EIO, read EINTR/짧은 read, close EINTR/EIO를 실제 `.fl` 호출에서 확인한다. close 주입은 실제 native fd를 닫고 같은 번호에 sentinel을 열어 재시도가 새 fd를 닫지 않는지 확인한다. 보조기가 실제 활성화됐다는 marker와 sentinel 검사를 요구하며, C 함수를 직접 호출한 결과로 언어 완료를 판정하지 않는다. 고정 출력은 모드별 .expected에 먼저 보존한다. JSON·db95 재생 시험과 이 결과를 혼합하지 않는다.

### C11 경계·실제 스레드 시험 선행 고정

추가 `.fl` 정답은 UTF-8 바이트 경계별 replacement·NUL 보존, 64 MiB append 상한과 무변경, 경로 이탈·leaf/parent/base symlink·directory/FIFO 차단이다. 별도 시험용 C barrier는 FreeLang run-parallel의 pthread 8개가 모두 도착할 때까지 대기시킨다. 실제 fd_write는 `.fl`에서 같은 handle에 스레드당 100개 한글 레코드를 쓴다. 언어 출력과 별개로 Bash 고정 정답 800행을 생성해 정렬한 실제 파일과 cmp한다. C barrier는 스케줄 겹침을 보장하는 보조기이며 fd API를 대신 호출하지 않는다. 일반 원자·WAL 거래 동시성 C20 완료로 확대 해석하지 않는다.

## C12 JSON·checksum 정답 선행 고정

기준 v11 동일 커밋 `bootstrap.js:26853~26905`의 JSON.parse 및 재귀 toSerializable+JSON.stringify, `32496`의 UTF-8 auth_sha256과 db95 `afj-tcp-wal-record-valid?`가 근거다. json_str/json_stringify 및 json_parse의 밑줄·하이픈 별칭을 연결한다. 숫자는 JS Number·shortest round-trip 표기이며 정수형 문자열 key는 숫자 순서, 나머지는 삽입 순서를 따른다. -0은 stringify 0, parse는 부호를 보존한다. nonfinite stringify는 null이다. NUL·제어 문자·서로게이트와 잘못된 JSON 문법을 별도로 검사한다. 공개 parser는 엄격히 검사하며 기존 seed용 legacy C ABI를 앱에 우회 연결하지 않는다.

합성 WAL body는 고정 JSON 텍스트로 보존하고 시스템 sha256sum과 UTF-16LE 길이로 prepare/commit checksum·length를 만든다. 언어의 json_str/sha 함수로 정답을 만들지 않는다. 별도 native reader 과정을 반복하며 원본 fixture의 cmp·SHA 불변을 확인한다. 실제 db95 load·재생이 아직 없으면 checksum 언어 시험 통과만 보고하며 L05/C11 전체 재생 완료로 판정하지 않는다. 사람 작성 JS·Node·Python은 사용하지 않는다.
