# B-9 stage1 → stage2 메모리 조사

B9=확정

조사 기준 HEAD는 `c624feb654ef7d428ceeb2af66a01778a703c10f`이며, 시작 작업 트리는 CLEAN이었다. 이 커밋에서 `investigate/b9-stage2-mem` 가지와 별도 작업 트리를 만들었다. 저장소 변경은 이 문서 하나다. 컴파일러·런타임·기존 시험·기대값·제한시간·실행기는 수정하지 않았다.

## 실행 조건과 stage1 식별

모든 빌드·측정에 `ulimit -v 2097152`와 `ulimit -c 0`을 적용했다. 전자는 프로세스 주소 공간 2147483648바이트 상한이며 RSS 상한과는 다르다. 무거운 명령 직전에 ps를 확인하고, 지정된 공용 잠금 아래 `flock -o`로 직렬 실행했다. 처음에는 다른 작업이 잠금을 보유해 차례를 기다렸다. 임시 입력·생성 C·실행 파일·로그는 작업 전용 임시 디렉터리 <가림>에만 두고 `TMPDIR=<가림>`을 전달했다. 공용 잠금 파일 외 루트 임시 경로에 작업 자료를 지정하지 않았다.

B-8에서 보존된 stage1.c를 수정 없이 현재 기준 런타임과 링크했다. `scripts/verify-e10-bootstrap.sh`의 `build_c`와 같은 `gcc -O2 -w`, 런타임 소스 목록 및 링크 옵션을 사용했다. 해당 실행기 자체는 실행하지 않았다. 기존 헤더·라이브러리만 재사용했으며 설치하지 않았다. 이번 조사에서 Stage0·seed 실행과 교체, 8GiB 실행, 전체 bootstrap 재실행은 없다.

| 식별 대상 | SHA-256 |
| --- | --- |
| 보존 stage1.c | b4cdbc346b43708d9f218301e2c84ea49934edd3d688c1b0f5814c9680314448 |
| 이번에 링크한 stage1 실행 파일 | 59cd3cde39a0a7881b2e4feb7875957c7fd5791ba93b09f745861a58f80bf65c |
| 전체 입력 self/cgc-main.fl | 7ac7ac98b777559eea7354c8de987acef7b1491afbbd53c56be15ab360694a6b |
| seed C 읽기 전용 확인 | f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c |

stage1 재빌드 stderr는 0바이트였고 실행 파일이 생성됐다. 기존 개발용 컴파일러를 stage1로 바꿔 부르지 않았다.

## 크기별 측정 3건

기존 소스나 시험 대신, 임시 입력에 동일한 `(+ 1 2)`와 LF 한 줄을 각각 256·512·1024회 넣었다. 한 줄은 8바이트이며, 내용과 토큰 밀도를 유지하면서 입력 크기를 정확히 두 배씩 늘렸다. 각 입력을 stage1으로 C까지 생성했으며, 생성 C의 빌드나 실행은 하지 않았다. GNU time의 실제 경과 시간과 최대 RSS 원문은 다음과 같다. RSS 단위 표기 KB는 도구 출력 형식이며 값은 KiB 단위다.

| 입력 크기(바이트) | 시간(초) | 최대 RSS(KB) |
| --- | --- | --- |
| 2048 | 0.07 | 56760 |
| 4096 | 0.17 | 195460 |
| 8192 | 0.57 | 731724 |

```text
FORMS=256 BYTES=2048
ELAPSED=0.07 MAX_RSS_KB=56760 TIME_EXIT=0
SHELL_EXIT=0 OUTPUT_BYTES=73600
FORMS=512 BYTES=4096
ELAPSED=0.17 MAX_RSS_KB=195460 TIME_EXIT=0
SHELL_EXIT=0 OUTPUT_BYTES=147840
FORMS=1024 BYTES=8192
ELAPSED=0.57 MAX_RSS_KB=731724 TIME_EXIT=0
SHELL_EXIT=0 OUTPUT_BYTES=296514
```

증가모양=불명. RSS 배율을 소수점 아래 4자리에서 버리면 3.4436배와 3.7435배다. 세 점만으로 실측 증가율을 정확히 비례 또는 2제곱 이상으로 확정하지 않는다. 관측값은 두 배 입력당 4배 이상이 아니다. 아래의 소스·실행 스택으로 확인한 누적 복사 비용 O(n²)와 이 실측 분류를 구분한다.

입력 SHA-256:

```text
2048 bytes: 8740a4be5da9170684e47bcf890997bebfa804f9563340e438e81d8e65385b05
4096 bytes: 50161cb139a8915b1664b12e55886d72084d8fa5267d781dde4ddb134fa5abd1
8192 bytes: 3b13bbd80fa2541f591f283ac9b38fabd5a6b34fff60c9dec4338dae2b723ac2
```

## 전체 입력: 2GiB 제한에서 1회

입력 `self/cgc-main.fl`은 129293바이트, 2816줄이다. stage1 → stage2 전체 입력 실행은 이번 조사에서 한 번뿐이다. 실행 파일·소스에 계측 코드를 넣지 않고, 단일 strace의 `-k`로 실패한 메모리 시스템 호출의 스택을 수집했다. 실패 시점까지의 전체 실행 시간에는 strace 비용이 포함된다.

```bash
ulimit -c 0
ulimit -v 2097152
export TMPDIR=<가림>
# 아래 변수의 실제 경로는 모두 <가림>.
flock -o "$LOCK" /usr/bin/time \
  -f 'ELAPSED=%e MAX_RSS_KB=%M TIME_EXIT=%x' -o "$TIME_LOG" \
  strace -k -e trace=brk,mmap,mremap,munmap -e status=failed -o "$TRACE_LOG" \
  env -i PATH="$NATIVE_PATH" TMPDIR="$TMPDIR" \
  "$STAGE1_BIN" "$COMPILER_SOURCE" "$STAGE2_C"
```

```text
STAGE1_TO_STAGE2_SHELL_EXIT=139
Command terminated by signal 11
ELAPSED=3.09 MAX_RSS_KB=1883164 TIME_EXIT=0
STAGE2_OUTPUT=ABSENT
TRACE_LINES=53
```

GNU time의 `TIME_EXIT=0`은 신호 종료에서 출력된 값이며 성공이 아니다. 별도로 수집한 실제 shell 종료 코드는 139다. stage2 C 파일은 생성되지 않았고 stdout·stderr는 각각 0바이트였다.

실패 mmap 원문과 그 아래 호출 스택에서 경로·주소·오프셋만 가린 발췌:

```text
mmap(NULL, 1048576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = -1 ENOMEM (Cannot allocate memory)
 > <가림>(__mmap+<가림>) [<가림>]
 > <가림>(sysmalloc_mmap_fallback.constprop.0+<가림>) [<가림>]
 > <가림>(sysmalloc+<가림>) [<가림>]
 > <가림>(_int_malloc+<가림>) [<가림>]
 > <가림>(__libc_malloc+<가림>) [<가림>]
 > <가림>(fl_arena_alloc+<가림>) [<가림>]
 > <가림>(fl_vec_push+<가림>) [<가림>]
 > <가림>(fl_concat+<가림>) [<가림>]
 > <가림>(token_chunk_add+<가림>) [<가림>]
 > <가림>(emit+<가림>) [<가림>]
 > <가림>(read_token+<가림>) [<가림>]
 > <가림>(lex_loop+<가림>) [<가림>]
 > <가림>(cgc_run_positional+<가림>) [<가림>]
 > <가림>(cgc_run+<가림>) [<가림>]
 > <가림>(main+<가림>) [<가림>]
```

같은 경로의 mmap ENOMEM이 2회 기록된 뒤 다음 신호 스택이 기록됐다.

```text
--- SIGSEGV {si_signo=SIGSEGV, si_code=SEGV_MAPERR, si_addr=NULL} ---
 > <가림>(__memmove_avx_unaligned_erms_rtm+<가림>) [<가림>]
 > <가림>(fl_vec_push+<가림>) [<가림>]
 > <가림>(fl_concat+<가림>) [<가림>]
 > <가림>(token_chunk_add+<가림>) [<가림>]
 > <가림>(emit+<가림>) [<가림>]
 > <가림>(read_token+<가림>) [<가림>]
 > <가림>(lex_loop+<가림>) [<가림>]
 > <가림>(cgc_run_positional+<가림>) [<가림>]
```

원본 trace SHA-256는 `ed931f5143e2a361950892567db183a3c14d125ecce97f1db631d4fc61ec9e0d`다. 원문은 접근을 제한한 작업 임시 디렉터리 <가림>에 보존한다.

폭발단계=해석(어휘 분석, lex). `self/cgc-main.fl:2743`의 `(parse (lex $src))` 중 lex 안에서 실패했다. 파일 읽기는 끝났고 AST 파서 호출·코드 생성·stage2 쓰기는 아직 도달하지 않았다. 출력 파일 부재만으로 단계를 추정한 것이 아니라 실제 실패 스택으로 판정했다. 예전 8GiB 실행의 실패 단계까지 이번 스택과 동일하다고 단정하지는 않는다.

## 확인 위치와 DB95-26 비교

| 파일:줄 | 이유 | 확실도 |
| --- | --- | --- |
| self/cgc-main.fl:65 | token-chunk-add의 같은 크기 토큰 묶음 병합이 concat을 호출한다. 보존 stage1.c:2089에도 fl_concat 호출이 있고, 전체 입력의 실패 스택에 token_chunk_add가 있다. | 확인 |
| runtime/aliases.c:413 | fl_concat의 벡터 분기는 fl_vec_new로 시작해 두 입력의 모든 원소를 fl_vec_push로 하나씩 추가한다. 이어지는 414줄도 같은 방식이며 실패 스택에서 이 함수가 확인됐다. | 확인 |
| runtime/collection.c:114 | 불변 push가 매번 길이 n+1의 배열을 할당하고 115줄에서 기존 n개를 memcpy한다. 할당 실패 검사 없이 복사하며 실제 ENOMEM 뒤 memmove의 NULL 접근으로 종료됐다. | 확인 |

fl_vec_new는 rc=1인 불변 벡터를 반환하므로 이 concat에서 builder fast-path(rc=0xFE)는 사용되지 않는다. 길이 k로 합치는 동안 원소 배열 할당량은 `sizeof(FLValue) × (1 + 2 + ... + k)`이며, 앞선 중간 배열은 arena에 남는다. 따라서 이 합치기 한 번의 누적 원소 복사·할당 비용이 O(k²)다. CLI arena는 종료 정리까지 유지되며(`runtime/gc.c:242`, `runtime/gc.c:249`), lex 반복마다 중간 배열을 회수하는 호출은 없다. 토큰 묶음 병합 구조 자체를 바꿔도 내부 concat가 이 구현이면 전체 복사 비용을 피하지 못한다.

DB95-26과_같은가=예. DB95-26 보고서 `docs/DB95-26-runtime-login.md:162`는 fl_repeat → fl_vec_push에서 매번 전체 불변 배열을 복사하고 arena에 중간 배열이 남는 문제를 기록했다. 이번에는 진입 함수가 fl_concat이고, 실제로 같은 runtime/collection.c의 fl_vec_push를 거친다는 점이 소스와 실패 스택 모두에서 확인됐다. DB95-26 보고서의 과거 실행 수치를 이번 측정값으로 재사용하지 않았다.

## 결과와 후속 제안

B9=확정. 2GiB에서 폭발한 해석 단계와 전체 복사의 실제 실행 위치를 확인했다. 소스 수정이나 수정 후 성능 개선 검증은 수행하지 않았다. 이번 증거만으로 2GiB 전체 자기 컴파일 성공이나 새 고정점 통과를 주장하지 않는다.

다음제안=별도 수정 승인 시 fl_concat 벡터 분기의 기존 builder 재사용과 할당 실패 처리를 검토하고, 같은 입력·상한으로 효과를 검증한다.

저장소에는 docs/B9-MEM.md만 추가한다. 이번 지시에 따라 GitHub의 investigate/b9-stage2-mem에만 푸시하며 Forgejo는 보류한다. 최종 문서 커밋 HEAD·GitHub 원격 HEAD 일치·CLEAN 여부는 푸시 후 최종 응답에 기록한다.
