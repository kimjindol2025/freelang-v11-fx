# L07 파일 SHA-256 streaming 확장 진행

## 확인한 것

목적은 이미지 백업 checksum을 계산할 때 파일 전체를 request arena에 남기지 않는 것이다. 새 언어 API이므로 블로그 시험과 별도로 자체 시험·정답을 먼저 커밋했다. 기존 API를 대체하지 않는다.

- 최초 계약/시험: `ee6b4e3b3e50fa3f6aed5fb978522dd61243e08a`. 정확한 인자 수·함수 값 추가: `52a530eab08637f9b9e05bf39153c447d892e07d`.
- 구현: `043151bd578774b86d8a1de7760b071137cd407b`. runtime/io.c의 안전 경로 열기와 result map을 사용하며, 기존 OpenSSL libcrypto의 EVP SHA-256을 호출한다. 새 외부 의존성을 도입하지 않았다. 읽기 버퍼는 `65536` byte, 허용 파일 상한은 `67108864` byte다.
- 전체 driver 고정: `cdad51e78deee1c3cde014fd106566db77159afc`. 새 기능의 일반·sanitizer 시험을 전체 gate에 포함했다. 기준 stage0·기존 정답·상위 verify-native-bootstrap.sh는 바꾸지 않았다.

처음에는 curl header 환경 누락으로 빌드 종료 `1`이었다. 이를 기능 실패와 구분해 보존했다. 의존 경로를 지정한 실제 수정 전 실행은 모든 경우가 Function not found로 실패했다.

```text
FILE_HASH_GROUP_PASS=0 FAIL=8
FILE_HASH_BEFORE_CONFIGURED_EXIT=1
```

새 함수 값을 모르는 옛 compiler 실행물로 한 후속 빌드도 종료 `1`이었다. 같은 cgc 소스에서 이미 검증한 생성 C의 SHA-256 `cb872631385823f7b41a8c26b09faeb155b4cab649cb42b33343d24f3a5f8696`을 확인하고 새 runtime으로 compiler를 다시 링크했다. 그 compiler 실행물 SHA-256은 `1128409362c4f0f76d813404ee8ecd02c7ffcc4acd3e3de81f33c693d4cab38a`이다. 이 재링크를 새 bootstrap 완료로 보고하지 않는다.

일반 실행과 ASan/UBSan 실행은 각각 다음 원문으로 끝났다.

```text
FILE_HASH_GROUP_PASS=8 FAIL=0
FILE_HASH_NATIVE_EXIT=0
NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0
```

```text
FILE_HASH_GROUP_PASS=8 FAIL=0
FILE_HASH_SANITIZER_EXIT=0
NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0
```

빈 파일·모든 byte 256개·한글/이모지/NUL/잘못된 UTF-8·64 MiB 파일을 외부 sha256sum과 대조했다. 일반 파일만 허용하고 NUL/잘못된 경로·경로 이탈·최종/부모 symlink·FIFO·디렉터리·상한 초과를 거부한다. 정확한 인자 수와 underscore 별칭·함수 값 사용도 검사했다. EINTR·short read를 처리하고 읽기/close 오류는 hash 없이 실패한다. 읽는 중 증가·같은 크기 수정·축소 시 FILE_CHANGED이고 모든 실행의 FD 수가 복귀한다. 정상 20개 판정과 일곱 오류/부분 읽기 경우의 각 2개 판정을 8묶음으로 집계했으며 중복 합산하지 않았다.

64 MiB hash의 ru_maxrss 증가가 `16384` KiB 미만인 고정 기준도 두 실행에서 통과했다. 전체 백업의 장시간 RSS·동시 작업 메모리·p95 목표 통과라는 뜻은 아니다.

blog `1783dbcd2a07e96f2c3a9a58c1aab502d300dfe8`은 checksum 계산을 이 API로 바꿨다. 새로 빌드한 실제 export/import ASan/UBSan 회귀는 `BACKUP_INTEGRITY_PASS=13 FAIL=0`, `STREAM_BACKUP_INTEGRITY_EXIT=0`이다. 불확실 commit의 writer·별도 reader 복원 회귀는 `BACKUP_RESTORE_FAULT_PASS=4 FAIL=0`, `STREAM_BACKUP_RESTORE_EXIT=0`이다. 정상·무해시 과거 version 2 호환·같은 크기 손상/잘못된 hash/경로/다른 blog 행/예상 밖 파일 거부, 원본 합성 archive·외부 파일·거부 때 대상 WAL 보존과 commit 불확실 때 파일 보존을 확인했다. 두 trace 모두 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다. 원문은 docs/evidence/l08-platform/stream-backup-integrity.txt 및 stream-backup-restore.txt다.

## 실행 명령과 원문

검증된 native compiler·전용 출력 폴더·격리한 sanitizer libdir·기존 curl include/library 환경을 지정한 뒤 `strace -f -e trace=execve` 아래 `bash scripts/verify-l08-file-hash.sh`를 실행했다. `bash scripts/audit-l07-execve.sh`로 compiler 재링크와 두 자체 실행의 trace를 검사했다. 실제 경로는 `<가림>`이다.

계약은 tests/l08-platform/file-hash-contract.md, .fl 시험과 고정 정답은 같은 폴더의 file-hash.fl 및 file-hash.expected다. 원문은 docs/evidence/l08-platform의 file-hash-initial-build-environment.txt, file-hash-before.txt, file-hash-old-compiler-failure.txt, file-hash-native.txt, file-hash-sanitizer.txt에 보존했다.

## 전체 회귀 결과와 미확인

고정 `cdad51e78deee1c3cde014fd106566db77159afc`의 최초 전체 실행은 mode 000 파일 읽기 거부에서 `L08_P3_PASS=10 FAIL=1`, `STREAM_HASH_PLATFORM_FULL_EXIT=1`이었다. mapped root의 DAC 우회 capability가 원인이며 같은 실행물은 일반 사용자 또는 DAC 우회 capability 제거 뒤 원래 정답으로 통과했다. runtime과 기대값을 수정하지 않았다. 실패와 동일 실행물 비교는 stream-hash-full-permission-failure.txt 및 permission-context-reproduction.txt에 보존했다.

같은 고정 소스·정답을 사용하고 namespace에서 DAC_OVERRIDE와 DAC_READ_SEARCH만 제거한 전체 실행은 `UNPRIVILEGED_STREAM_HASH_FULL_EXIT=0`, `L08_PLATFORM_CURRENT_GATES=PASS`다. L08 3차 `11 PASS / 0 FAIL`, 2차 `11 PASS / 0 FAIL`, 1차 `17 PASS / 0 FAIL`, L06 HTTP/TLS 각각 `110 PASS / 0 FAIL`, L07 `221 PASS / 0 FAIL`이며 파일 hash 일반·sanitizer 각각 `8 PASS / 0 FAIL`을 포함한다. 변경하지 않은 자기 컴파일도 `STAGE2_EQ_STAGE3=PASS`, `BOOTSTRAP_EXIT=0`이다. 원문은 [stream-hash-unprivileged-full.txt](evidence/l08-platform/stream-hash-unprivileged-full.txt), 증거 커밋은 `5d851f03200e930ee4b85b70df60ef440519ebec`다. 감사는 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다. 이후 추가한 L01 compiler의 새 전체 검증과 구분한다.

실제 비밀번호 로그인·전체 브라우저 편집기·댓글·다중 작성자·L05 자체 DB 통합·이미지 처리·예약 발행·운영 복사본 호환과 전체 출시 기준은 여전히 미완료다. 기존 JS 편집기의 사진 메뉴·삽입·미리보기 27개와 생성 JS의 언어 의미 29개는 별도 부분 검증이며 제품 전체 인수를 대신하지 않는다.

동시 수정이 전부 차단된 snapshot hash를 제공하지 않는다. 파일 크기·mtime/ctime 변화와 읽은 byte 수를 검사하지만 다중 과정의 원자적 snapshot을 보장하지 않는다. FL_FILE_BASE 설정 시 기존 안전 경로 계약을 따른다. FL_NO_CRYPTO 빌드의 HASH_UNAVAILABLE 분기는 이번 자체 실행에 포함하지 않았다.

서버 23·25 접속·운영 자료 접근/변경·백업 원본 삭제·합치기·배포·강제 푸시는 하지 않았다. 새 TS·수작업 JS·Python 파일과 fl-build.sh를 사용하지 않았다.
