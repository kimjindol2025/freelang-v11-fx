# C20 atom snapshot 수명 보완

## 확인한 것

변경하지 않은 blog1 `013a5b339068412332ae12a7ef0c412963a3a880`의 stage1 저장소 시험을 소스만 추출한 복사본에서 실행했다. 운영 데이터·설정은 읽거나 변경하지 않았다. 최초 writer는 `writer_NATIVE_EXIT=139`, `BLOG_NATIVE_STORE_EXIT=1`이었다. ASan은 reset!이 이전 atom root를 해제한 뒤 지역 변수에 남은 map key를 읽는 heap-use-after-free를 검출했다. [최초 실패](evidence/l08-platform/blog-store-before.txt), [ASan 원문](evidence/l08-platform/blog-store-sanitizer-before.txt).

v11 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4`의 atom/deref/reset! 소스 계약으로 최소 `.fl` 시험과 정답을 먼저 커밋했다. 지역 변수·함수 반환·다른 atom으로의 보관은 reset 뒤에도 유효해야 한다. reset!은 입력값을 반환한다. Node 실행 없이 소스를 조사했다. [계약](../tests/l08-platform/atom-snapshot-contract.md), [최소 실패](evidence/l08-platform/atom-snapshot-before.txt).

런타임에서 deref가 읽은 root를 thread-local 참조로 보호하고, 함수 반환값을 caller arena로 복사한 뒤 해당 함수의 보호 참조를 해제한다. 요청 arena 종료와 중첩 arena 경계에서도 정리한다. atom read/retain과 root 교체에는 짧은 분할 mutex를 사용하며, callback 실행 중 mutex를 잡지 않는다. 일반 heap root의 참조 수에 별도 high bit를 써서 참조 수 255가 영구 atom 표식과 충돌하지 않게 했다. 기존 atom·promise·함수의 공유 identity는 유지한다.

```bash
CPATH=<가림> LIBRARY_PATH=<가림> L08_SNAPSHOT_OUT=<가림> L08_SNAPSHOT_COMPILER=<가림> L08_SNAPSHOT_SANITIZER_LIBDIR=<가림> strace -f -e trace=execve -o <가림> bash scripts/verify-l08-atom-snapshot.sh
bash scripts/audit-l07-execve.sh <가림>
```

최소 `.fl` 7개 출력과 C 메모리 보조 시험이 ASan/UBSan에서 통과했다. 후자는 reader 4개×1000회가 writer의 root 교체 뒤 이전 값을 읽는 경우, 중첩 arena와 반환값을 확인한다. `atom-snapshot_EXIT=0`, `atom-snapshot-memory_EXIT=0`, `SNAPSHOT_SUITE_EXIT=0`, `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`. [원문](evidence/l08-platform/atom-snapshot-after.txt). 최초 driver의 집계 표기 `L08_ATOM_SNAPSHOT_PASS=2_FAIL=0`는 이후 `L08_ATOM_SNAPSHOT_PASS=2 FAIL=0`로 표기만 수정했으며 원문을 고치지 않았다.

## 미확인

실제 저장소 fixture는 수정 후 메모리 충돌 지점을 통과했지만 `schema constraint engine failure`로 `writer_NATIVE_EXIT=1`, `BLOG_NATIVE_STORE_FIXED_EXIT=1`이다. [후속 실패](evidence/l08-platform/blog-store-after-snapshot.txt). writer 성공·reader 10회·WAL 무변경을 아직 입증하지 못했다. vendor·앱·정답·WAL 기록을 바꿔 통과시키지 않는다. v11 연산 호출 오류 문맥과 fx의 차이를 다음 조사 대상으로 남긴다.

전체 회귀·native bootstrap과 실제 저장소 ASan 재시험은 아직 미실행이다. `detect_leaks=0`이며 누수 검사는 미실행이다. swap!의 논리적 원자성, DB 다중 atom 거래, RC cycle·영구 atom 수명, 모든 할당 실패 처리는 이 결과로 완료 판정하지 않는다. 전체 블로그 실행·로그인·댓글·다중 작성자·화면·운영 데이터 호환·복구 검증도 남아 있다. 서버 23·25 접속·합치기·배포는 하지 않았다.
