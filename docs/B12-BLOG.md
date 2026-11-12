# B-12 정식 빌드 실행 전 기록

시작 HEAD: cccfca9a354557dae0da29bf8d2d0862d6792214. CLEAN 확인 후 feat/b12-blog-resume 가지를 분리했다.

기존 정식 실행기는 bootstrap/stage0.c를 고정 C seed로 빌드·실행한다. seed SHA는 f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c다. 사용자가 B12 보충으로 이번 정식 빌드·기존 회귀의 고정 C seed 사용을 허용했다. seed 교체·재생성·수정 및 실행기 수정은 허용하지 않았다.

B-11의 보존 stage1→stage4 생성 C 고정점은 0dd2b662803aed1bc3392ac855a73e8c1066c07ec3bb46fee81ffa13a505f1aa이며, 이를 이번 seed→stage4 결과로 재사용하지 않는다.

제품의 기존 PRD 24개 시작 원표와 인수 범위는 blog1의 docs/B12-BLOG.md에 정리했다. 아직 새 PRD 통과나 B-8 완료 수를 올리지 않았다. 제품 코드는 아직 수정하지 않았고 seed·기존 시험·실행기는 그대로 유지한다. 임시 경로·주소·소유자는 <가림>으로 표시한다.

## 첫 정식 빌드 원문

기존 실행기 scripts/verify-e10-bootstrap.sh를 수정 없이 사용했다. ps 확인과 지정 공용 flock 아래, native 주소 공간 상한 2097152KiB 및 user cgroup MemoryMax=2147483648, MemorySwapMax=0, core 0을 적용했다. TMPDIR과 모든 산출물은 작업 전용 디렉터리 <가림>으로 지정했다.

```text
SEED_SHA_BEFORE=f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c
STAGE0_C_BUILD=PASS
FORMAL_RUNNER_EXIT=1
ELAPSED=18.44 MAX_RSS_KB=1531188 TIME_EXIT=1
MEMORY_PEAK_BYTES=1570254848
FreeLang: vector allocation failed
SEED_SHA_AFTER=f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c
SEED_SHA_UNCHANGED=1
```

seed→stage1의 토큰 누적은 고정 seed 안의 불변 fl_vec_push를 호출한다. B11의 .fl 렉서 변경은 이미 빌드된 이 seed의 렉서에 소급 적용되지 않는다. stage1 생성이 끝나지 않았으므로 이 실행의 stage2/3/4 고정점은 미관측이며 과거 값을 복사하지 않는다. 실패 원문과 생성물은 임시 디렉터리 <가림>에 보존했다.

## 런타임 보완 중

seed와 실행기는 그대로 유지한다. runtime/collection.c의 불변 append는 새 벡터 헤더를 반환하고, 연속 append에서만 이전 논리 길이 뒤의 예약 공간을 공유하도록 보완했다. 이전 prefix에서 분기하거나 용량이 차면 새 버퍼에 복사한다. 기존 벡터 길이·요소는 변경하지 않는다. arena 전용 prefix 표식은 heap 소유 표식과 구분하며, heap/snapshot 복사는 기존대로 논리 요소만 복사한다. 할당 실패 메시지·종료 처리를 유지한다.

기존 seed가 출력하는 vec_builder/freeze_builder C 이름을 이미 있는 builder 함수에 연결하고, 이전 str_split의 빈 구분자는 기존 UTF-16 분리 구현에 연결한다. 새 seed나 새 해석기는 만들지 않았다. 분기·경계·성장·소유 스냅샷의 임시 진단 9건은 보완 전 출력을 고정한 뒤 비교해 일치했다. 기존 시험·정답을 수정한 것이 아니다.

기존 회귀 실행기에서 서버가 모든 인터페이스에 바인딩되는 경로를 피하기 위해 runtime/http.c에 선택적인 FL_HTTP_LOOPBACK_ONLY=1 설정을 추가했다. 이 설정은 HTTP/TLS를 loopback에만 바인딩한다. 설정하지 않은 기본 동작은 보존한다. 시험·실행기 파일을 수정하지 않고 이 환경을 전달할 예정이다. 실제 실행 결과는 재검증 후 추가한다.
