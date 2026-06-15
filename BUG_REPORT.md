# FreeLang fx — 버그 보고서 & 수정 계획안

> 작성일: 2026-06-16  
> 분석 방법: 병렬 에이전트 4개 (cgc-main.fl / C 런타임 / 앱 코드 / 커버리지 갭)  
> 상태: 모든 항목 실제 코드 확인 완료 (FALSE_POSITIVE 제거됨)

---

## 1. 버그 목록 (확인 완료)

### 🔴 CRITICAL

#### BUG-01 — SQL 인젝션 (앱 전체)

**위치**: fx-notify, fx-proxy, fx-cron, fx-deploy, fx-search 전체  
**패턴**:
```lisp
;; ❌ 현재 — 사용자 입력이 SQL 문자열에 직접 삽입됨
(fxb_sqlite_exec db
  (str "INSERT INTO notifications VALUES ('" $id "','" $source "','" $event "'...)"))

(fxb_sqlite_query db
  (str "SELECT * FROM crons WHERE id='" $id "'"))
```
**영향**: 테이블 삭제, 데이터 조작, 인증 우회 가능  
**확인 위치**:
- `fx-notify/server.fl:34-55` — INSERT + WHERE 필터
- `fx-proxy/server.fl:47-54, 171-176` — pick-target + delete
- `fx-cron/server.fl:162-214` — create / get / delete / pause / resume

**수정 방법**: `fxb_sqlite_query_p` / `fxb_sqlite_exec_p` (이미 구현됨)
```lisp
;; ✅ 수정 후 — ? 플레이스홀더 바인딩
(fxb_sqlite_exec_p db
  "INSERT INTO notifications(id,source,event) VALUES (?,?,?)"
  (list $id $source $event))

(fxb_sqlite_one_p db "SELECT * FROM crons WHERE id=?" (list $id))
```

---

#### BUG-02 — try-finally 미구현 (cgc-main.fl)

**위치**: `cgc-main.fl:835-864` (cgc-try 함수)  
**원인**: 파서는 finally 절을 파싱하지만 cgc-try에서 `:finally` 필드를 읽지 않음
```lisp
;; 파서(line 485-491): finally-clause 파싱됨 → make-try 3번째 인자로 저장
;; cgc-try(line 835): (get $n :body), (get $n :catch) 만 읽음, :finally 없음
```
**재현**:
```lisp
(try
  (/ 1 0)
  (catch [$e] (println "에러"))
  (finally (println "cleanup")))  ;; "cleanup" 절대 실행 안 됨
```

---

### 🔴 HIGH

#### BUG-03 — let 섀도잉 C 중복 선언

**위치**: `cgc-main.fl:1416-1433` (cgc-let-1d / cgc-let-2d)  
**원인**: 같은 이름의 let 재바인딩 시 C 변수 중복 선언 발생
```lisp
;; ❌ 재현
(let [$x 1]
  (let [$x 2]  ;; C에서 FLValue x = 2; 중복 선언 → 컴파일 에러
    (+ $x 1)))
```
**생성 C 코드**:
```c
(__extension__ ({
    FLValue x = 1;        // 첫 번째 let
    FLValue x = 2;        // 두 번째 let — C 컴파일 에러!
    fl_add(x, fl_int(1));
}))
```

---

#### BUG-04 — 중첩 fn 클로저 캡처 오류 ✅ FIXED (2026-06-16, 커밋 6d499c4)

**위치**: `cgc-main.fl:1305-1333` (cgc-fn)  
**수정**: `_fplen = len(@outer)` → `concat(fn-params)` → body 컴파일 → `slice(0, _fplen)` 복원  
**원인**: `outer-params-atom`이 중첩 fn 컴파일 시 초기화되지 않아 내부 fn이 외부 변수를 캡처하지 못함
```lisp
;; ❌ 재현
(defn make-adder [$x]
  (fn [$y] (+ $x $y)))  ;; 내부 fn이 $x를 캡처 못 할 수 있음

;; 더 깊은 중첩
(fn [$x]
  (fn [$y]
    (fn [$z] (+ $x $y $z))))  ;; 최내부 fn: $x, $y 캡처 불확실
```

---

#### BUG-05 — fl_html_escape strcpy 버퍼 오버플로

**위치**: `runtime/core.c:401-423`  
**원인**: `strcpy(dst, "&amp;")` — 경계 체크 없이 포인터에 복사
```c
// ❌ 현재
case '&': strcpy(dst, "&amp;"); dst += 5; break;

// ✅ 수정
case '&': memcpy(dst, "&amp;", 5); dst += 5; break;
```
**note**: 버퍼 크기 `n*6+1`은 올바름. strcpy → memcpy 교체로 해결.

---

#### BUG-06 — future 메모리 누수

**위치**: `runtime/aliases.c:1043-1075` (fl_future / fl_deref)  
**원인**: `FLFuture*` 구조체가 detached thread 종료 후 `free()` 미호출
```c
// future_runner 완료 후 f (FLFuture*) 해제 코드 없음
// fl_deref에서 f->result 읽은 후 free(f) 없음
// 매 future 생성마다 ~sizeof(FLFuture) bytes 누수
```

---

### 🟡 MEDIUM

#### BUG-07 — lambda 내 recur = silent nil

**위치**: `cgc-main.fl:992`  
**원인**: fn lambda 내부에서 recur 호출 시 orphan recur 처리 → `fl_nil()` 반환
```lisp
;; ❌ 조용히 실패
(let [$f (fn [$x] (if (> $x 0) (recur (- $x 1)) 0))]
  ($f 5))
;; → fl_nil() 반환, 에러 없음 (무음 버그)

;; ✅ 올바른 방법
(loop [$x 5] (if (> $x 0) (recur (- $x 1)) 0))
```

---

#### BUG-08 — monitor-loop / cron-loop 직접 재귀 (TCO 미적용)

**위치**: `fx-monitor/server.fl:183-187`, `fx-cron/server.fl:140-144`  
**원인**: `(defn loop-fn [] ... (loop-fn))` 형태의 직접 재귀는 recur 미사용 → TCO 적용 안 됨
```lisp
;; ❌ 현재 — 스택 무한 누적
(defn monitor-loop []
  (do
    (try (run-check) (catch $e nil))
    (sleep CHECK_INTERVAL)
    (monitor-loop)))  ;; goto TCO 적용 안 됨

;; ✅ 수정 — loop/recur 패턴
(loop [$_ true]
  (do
    (try (run-check) (catch $e nil))
    (sleep CHECK_INTERVAL)
    (recur true)))
```

---

#### BUG-09 — str_replace_re strcpy

**위치**: `runtime/aliases.c:971`  
**원인**: 정규식 치환 시 `strcpy` 사용
```c
// ❌ 현재
strcpy(buf + start + rlen, s + end);

// ✅ 수정
int tail = slen - end;
memcpy(buf + start + rlen, s + end, tail);
buf[start + rlen + tail] = '\0';
```

---

#### BUG-10 — get() key.obj NULL 체크 없음

**위치**: `runtime/collection.c:333`  
**원인**: `key.tag == FL_STRING` 체크 후 `key.obj` NULL 여부 미확인
```c
// ❌ 현재
const char* ks = (key.tag == FL_STRING) ? ((FLString*)key.obj)->data : "?";
//                                                    ^^^^^^^^^ key.obj가 NULL일 수 있음

// ✅ 수정
const char* ks = (key.tag == FL_STRING && key.obj)
    ? ((FLString*)key.obj)->data : "?";
```

---

## 2. FALSE POSITIVE (버그 아님)

| 항목 | 원인 | 결론 |
|------|------|------|
| fl_vec_slice 음수 off-by-one | `len + end.i + 1` — 파이썬 slice 규칙과 동일, 올바름 | ❌ 버그 아님 |
| fl_atom_deref 미정의 | math.c:210에 정의됨, aliases.c에서 정상 호출 | ❌ 버그 아님 |
| swap!이 fl_atom_deref 사용 | atom(FL_VECTOR)에만 사용되므로 정상 동작 | ❌ 버그 아님 |

---

## 3. 수정 계획안

### Phase 1 — 즉시 (1~2일) : 보안

**목표**: SQL 인젝션 전면 패치. 준비된 함수(`_p`) 이미 있으므로 교체만 하면 됨.

| 파일 | 작업 | 함수 교체 |
|------|------|----------|
| `fx-notify/server.fl` | db-insert, db-list, db-get | `fxb_sqlite_exec` → `fxb_sqlite_exec_p` |
| `fx-proxy/server.fl` | pick-target, handle-register, handle-delete, handle-disable/enable | `fxb_sqlite_exec/query` → `_p` |
| `fx-cron/server.fl` | handle-create, handle-get, handle-delete, handle-pause, handle-resume, handle-runs | `fxb_sqlite_exec/query` → `_p` |
| `fx-deploy/server.fl` | handle-deploy, handle-get-deploy, handle-list-deploys, handle-register | `fxb_sqlite_exec/query` → `_p` |
| `fx-search/server.fl` | handle-index-doc, handle-get-doc, handle-delete-doc, handle-terms | `fxb_sqlite_exec/query` → `_p` |

```lisp
;; 패턴: 모든 fxb_sqlite_*에서
;; 문자열 연결 → ? 플레이스홀더 + (list ...)

;; Before
(fxb_sqlite_query db (str "SELECT * WHERE id='" $id "'"))

;; After
(fxb_sqlite_query_p db "SELECT * WHERE id=?" (list $id))
```

---

### Phase 2 — 단기 (3~5일) : 언어 버그

**목표**: 컴파일러 수준 버그 수정. 고정점 검증 필수.

#### 2-A. try-finally 구현 (`cgc-main.fl:cgc-try`)

```lisp
;; cgc-try에 :finally 처리 추가
(defn cgc-try [$n]
  (let [[$body-node    (get $n :body)]
        [$catch-node   (get $n :catch)]
        [$finally-node (get $n :finally)]  ;; ← 추가
        ...
        [$finally-c (if (null? $finally-node) ""
                      (str $finally-c-code " "))]  ;; ← finally C 코드 생성
        ...]
    ;; if/else 양쪽에 finally 삽입
    ...))
```

#### 2-B. let 섀도잉 — 변수명 유일화 (`cgc-let-1d`)

```lisp
;; 중복 선언 방지: _s0, _s1 suffix
(defn make-shadow-name [$n $depth]
  (if (> $depth 0) (str $n "__s" $depth) $n))
```

또는 C 블록 스코프 활용:
```lisp
;; let을 C 블록으로 감싸면 섀도잉이 자연스럽게 해결됨
;; (__extension__ ({ FLValue x = 1; { FLValue x = 2; ... } }))
```

#### 2-C. monitor-loop / cron-loop 패치 (앱 코드)

```lisp
;; fx-monitor, fx-cron에서 직접 재귀를 loop/recur로 변경
(loop [$_ true]
  (do
    (try (run-check) (catch $e nil))
    (sleep CHECK_INTERVAL)
    (recur true)))
```

---

### Phase 3 — 중기 (1~2주) : 런타임 안전성

**목표**: C 런타임 메모리 안전성 개선.

#### 3-A. strcpy → memcpy 전면 교체

```bash
# 대상 파일
grep -n "strcpy" runtime/core.c runtime/aliases.c runtime/io.c
```

각 strcpy를 `memcpy(dst, src, len)` + `dst[len] = '\0'` 패턴으로 교체.

#### 3-B. future 메모리 누수 수정

```c
// fl_deref에서 future 사용 완료 후 해제
FLValue fl_deref(FLValue handle) {
    ...
    FLFuture* f = (FLFuture*)(uintptr_t)ptr_v.i;
    pthread_mutex_lock(&f->mu);
    while (!f->done) pthread_cond_wait(&f->cv, &f->mu);
    FLValue result = fl_heap_copy(f->result);  // 결과 복사
    pthread_mutex_unlock(&f->mu);
    // 정리
    pthread_mutex_destroy(&f->mu);
    pthread_cond_destroy(&f->cv);
    fl_heap_release(f->fn);
    free(f);
    return result;
}
```

**주의**: deref가 두 번 호출되면 double-free. 해제 여부를 `f->done = 2` 등으로 마킹 필요.

#### 3-C. get() NULL 체크 추가

```c
const char* ks = (key.tag == FL_STRING && key.obj)
    ? ((FLString*)key.obj)->data : "?";
```

---

### Phase 4 — 장기 (선택적) : 언어 기능 확장

**목표**: lambda 내 recur 지원, 중첩 클로저 캡처 정확도 향상.

#### 4-A. lambda 내 recur 지원

현재 orphan recur → `fl_nil()`. 두 가지 선택:
1. **컴파일 에러**: recur가 loop/defn 외부에서 발견되면 에러 메시지 출력
2. **loop 자동 래핑**: `(fn [$x] (recur ...))` → `(fn [$x] (loop [$x $x] (recur ...)))`

→ 1번 권장 (명시적 에러가 조용한 버그보다 낫다)

#### 4-B. 중첩 fn 클로저 캡처

`outer-params-atom` 초기화 전략:
- fn 컴파일 시작 시 현재 outer-params 스냅샷 저장
- 내부 fn 컴파일 후 스냅샷 복원
- 재귀적으로 스택 관리

---

## 4. 커버리지 갭 요약 (v11 대비)

### 즉시 필요한 함수 (자주 쓰는 것 위주)

| 함수 | 상태 | 비고 |
|------|------|------|
| `inc` / `dec` | aliases.c에 있음, dispatch 없음 | 직접 호출 불가 |
| `min` / `max` | aliases.c 확인 필요 | `(< a b)` 로 대체 가능 |
| `merge` | aliases.c에 있음 | 직접 호출 불가 |
| `map-keys` / `map-vals` | aliases.c에 있음 | 직접 호출 불가 |
| `update-in` / `assoc-in` | 미구현 | 중첩 맵 수정 불가 |
| `str-split` | aliases.c: `str_split` | 이름 다름 (underscore) |
| `str-join` | aliases.c: `str_join` | 이름 다름 |
| `date-format` | 미구현 | 타임스탬프만 가능 |
| MariaDB | 완전 미지원 | SQLite만 가능 |

### dispatch 추가 대상 (aliases.c에는 있으나 직접 호출 불가)

```lisp
;; cgc-dispatch에 아래 추가하면 직접 호출 가능해짐
[(= $op "inc")       (str "fl_int(" (cgc (get $args 0)) ".i + 1)")]
[(= $op "dec")       (str "fl_int(" (cgc (get $args 0)) ".i - 1)")]
[(= $op "merge")     (str "fl_map_merge(" (cgc (get $args 0)) ", " (cgc (get $args 1)) ")")]
```

---

## 5. 우선순위 매트릭스

| 버그 | 영향도 | 수정 난이도 | 우선순위 |
|------|--------|------------|---------|
| BUG-01 SQL 인젝션 | 보안 치명적 | 쉬움 (_p 함수 이미 있음) | **P0** |
| BUG-03 let 섀도잉 | 컴파일 에러 | 중간 | **P1** |
| BUG-04 중첩 fn 캡처 | ~~런타임 오류~~ | ~~어려움~~ | ✅ **FIXED** 6d499c4 |
| BUG-02 try-finally | 기능 누락 | 중간 | **P1** |
| BUG-08 loop 재귀 | 장기 스택 누수 | 쉬움 (앱 코드만) | **P2** |
| BUG-05 html_escape | 보안/안정성 | 쉬움 (strcpy→memcpy) | **P2** |
| BUG-06 future 누수 | 장기 메모리 | 중간 | **P2** |
| BUG-07 lambda recur | 무음 버그 | 중간 (에러로 변환) | **P3** |
| BUG-09 str_replace_re | 안정성 | 쉬움 | **P3** |
| BUG-10 get() NULL | 방어적 코딩 | 쉬움 | **P3** |

---

## 6. 고정점 정책

> cgc-main.fl 변경 시 반드시:
> ```bash
> bash /home/kimjin/freelang-v11-fx/verify-fixpoint.sh
> # gen-a == gen-b == gen-c PASS 후에만 커밋
> ```
>
> 현재 고정점 SHA: `594caca1`

---

*작성: Claude Code 2026-06-16*
