# CGC 컴파일러 완전 개선 로드맵 (2026-06-26)

## 📊 함정 분류 (27개)

### ✅ P0: 치명적 (3개) — **2026-06-26 검증 완료 (cgc-bin 469KB)**

| ID | 문제 | 상태 | 비고 |
|----|----|------|--------|
| **P0-3** | trap-defn-docstring-cgc | ✅ RESOLVED | 469KB cgc-bin에서 정상 동작 확인 |
| **P0-2** | trap-try-map-literal | ✅ RESOLVED | `{"key" val}` 직접 사용 가능 |
| **P0-1** | trap-defn-do | ✅ RESOLVED | cgc-defn-stmts 다중 표현식 처리 |

**증거**: test-p0.fl → `user-42 / step1 / step2 / 15 / true` 출력 확인  
**작업량**: 0시간 (이미 해결됨) | **우선도**: ~~++++++~~

---

### 🟡 P1: 심각 (11개) — 빌드 실패 또는 false-200

| ID | 문제 | 타입 | 해결책 |
|----|----|------|---------|
| **P1-1** | trap-count-type | cgc가 count를 모름 → 빌드 실패 | aliases.c에 count 추가 |
| **P1-2** | trap-kebab-symbol | kebab-case 함수명이 미정의 | 언더스코어 강제 또는 자동 변환 |
| **P1-3** | trap-str-includes-int | 0/1 반환 (진리값 아님) | 자동으로 boolean 변환 |
| **P1-4** | trap-throw-invalid-initializer | throw가 잘못된 C 초기화자 생성 | codegen 수정 |
| **P1-5** | trap-server-json-string | 맵 직접 전달 시 false-200 | 타입 체크 + 에러 메시지 |
| **P1-6** | trap-server-html-quote | HTML 안의 " 가 문자열 파서 혼동 | 문자열 이스케이프 강화 |
| **P1-7** | trap-string-literal-1024 | 1024B+ 리터럴이 PRoot에서 tmpfile 실패 | 자동 청크 분할 |
| **P1-8** | trap-nested-defn-skip | 중첩 defn이 컴파일되지 않음 | cgc 에러 또는 자동 최상위 이동 |
| **P1-9** | trap-inc-dec-missing | inc/dec 함수가 없음 | stdlib에 구현 추가 |
| **P1-10** | trap-cache-cgc-missing | cache.fl이 v11 전용 함수 사용 | cgc용 구현 제공 또는 경고 |
| **P1-11** | trap-parallel-run-missing | parallel.fl이 미구현 함수 사용 | 미구현 플래그 또는 구현 |

**작업량**: 20-25시간 | **우선도**: +++++

---

### 🟠 P2: 중요 (10개) — 회피 가능하지만 불편

| ID | 문제 | 영향 | 회피책 |
|----|----|------|--------|
| **P2-1** | trap-lambda-capture | defn 파라미터가 람다에서 못 봄 | atom 저장 후 deref |
| **P2-2** | trap-mariadb-count-float | COUNT가 float 반환 가능 | 문자열 변환 후 비교 |
| **P2-3** | trap-replace-all-rename | 함수명도 함께 치환됨 | 패턴 구체화 또는 수동 |
| **P2-4** | trap-set-interval-swallow | 타이머 콜백 throw 무시됨 | atom 플래그로 신호 전달 |
| **P2-5** | trap-nth-out-of-bounds | 범위 밖 접근이 nil 반환 | 명시적 경계 체크 |
| **P2-6** | trap-http-get-cgc-mapping | http_get이 맵 아닌 문자열만 반환 | http_client.c 직접 노출 |
| **P2-7** | trap-free-locale | free 출력이 locale 의존 | /proc/meminfo 직접 읽기 |
| **P2-8** | trap-runtime-header | 함수 부재 오판 → 불필요한 우회 | 전수 확인 체크리스트 |
| **P2-9** | trap-extern-flvalue-defn | extern FLValue를 defn 내부에서 호출 시 타입 오류 | fxb_ 패턴 사용 |
| **P2-10** | trap-atom-deref-gc | reset! 후 old 값 참조 → dangling pointer | re-deref 또는 pre-extract |

**작업량**: 15-20시간 | **우선도**: ++++

---

## 🎯 **개선 전략**

### Phase 1: P0 (즉시)
```
1. P0-3: docstring auto-do wrapping (cgc-defn)
2. P0-2: try-map parse-primary (cgc-parse-try)
3. P0-1: 재검증 (아마 이미 지원됨)

예상: 6-7시간 | 난이도: ⭐⭐-⭐⭐⭐
```

### Phase 2: P1 (다음 주)
```
1. P1-1,3,8,9,10: 빌드 에러 (aliases.c, codegen)
2. P1-2,6: 파싱/타입 (cgc-main.fl)
3. P1-4,5,7,11: codegen (cgc-emit-*)

예상: 20-25시간 | 난이도: ⭐⭐-⭐⭐⭐⭐
```

### Phase 3: P2 (월 후)
```
1. 런타임 버그 수정 (P2-1,2,4,9,10)
2. API 개선 (P2-6,7,8)
3. 문서화 + 체크리스트

예상: 15-20시간 | 난이도: ⭐-⭐⭐⭐
```

---

## 📈 **영향 분석**

### Phase 1만 완료 시 (P0)
```
✅ defn 다중 표현식 지원
✅ try-map 패턴 지원
✅ docstring 사용 가능
→ read2write MVP 기본 동작 가능
```

### Phase 1+2 완료 시 (P0+P1)
```
✅ 빌드 에러 제거 (count, kebab, throw)
✅ false-200 제거
✅ 중첩 defn 지원
✅ inc/dec 표준 지원
→ read2write MVP 완전 동작 가능
```

### Phase 1+2+3 완료 시 (모두)
```
✅ 람다 클로저 버그 수정
✅ 메모리 안정성 (gc)
✅ 타이머 안정성 (set-interval)
→ 프로덕션 준비 완료
```

---

## 🔧 **수정 우선순위 (난이도 기준)**

### 쉬운 것부터 (빠른 승리)
```
1. trap-inc-dec-missing (1시간) — stdlib에 추가
2. trap-server-json-string (2시간) — 타입 체크
3. trap-mariadb-count-float (1시간) — workaround 문서화
```

### 중간 난이도
```
4. trap-defn-docstring-cgc (2시간) — auto-do wrapping
5. trap-try-map-literal (3시간) — parse-primary
6. trap-kebab-symbol (2시간) — 자동 변환
```

### 어려운 것
```
7. trap-atom-deref-gc (4시간) — GC 로직 수정
8. trap-throw-invalid-initializer (3시간) — codegen
9. trap-cache-cgc-missing (3시간) — 새 구현
```

---

## 💾 **파일 수정 목록**

### cgc-main.fl (컴파일러 소스)
- cgc-defn (line 1973) — P0-3
- cgc-parse-try (line 924) — P0-2
- cgc-* (여러) — P1 (10개)
- cgc-* (여러) — P2 (5개)

### runtime/*.c (C 런타임)
- aliases.c — P1-1, P1-9
- math.c — P2-10 (gc)
- server.c — P1-5
- http.c — P2-6

### stdlib/*.fl (표준 라이브러리)
- cache.fl → cgc용 재구현 — P1-10
- parallel.fl → 구현 — P1-11

---

## 📋 **검증 체크리스트**

### 각 패치 후
- [ ] 컴파일 성공
- [ ] 기존 코드 회귀 테스트
- [ ] 함정별 단위 테스트
- [ ] read2write MVP 테스트

### 전체 완료 후
- [ ] 모든 27개 함정 재현 테스트 통과
- [ ] 프로덕션 코드베이스에서 회귀 없음
- [ ] 성능 벤치마크 (v11 vs cgc 비교)

---

## 🚀 **일정**

| Phase | 기간 | 작업시간 | 팀 |
|-------|------|---------|-----|
| **P0** | 1주 | 6-7h | 1명 (253 노드) |
| **P1** | 2-3주 | 20-25h | 1-2명 |
| **P2** | 월 후 | 15-20h | 1명 |
| **총합** | 1-2개월 | ~50시간 | 1-2명 |

---

## 📊 **성능 기대효과**

```
현재 (v11만):
  - 메모리: 70-90MB
  - 성능: fib30 = 2분 11초
  - 개발: 느린 해석기

P0 패치 후 (cgc 사용 가능):
  - 메모리: 3-5MB (16배 개선)
  - 성능: fib30 = 0.4초 (330배 개선)
  - 개발: C 컴파일 성능

P1 완료 후 (완전 지원):
  - 안정성: 빌드 에러 제거
  - 호환성: v11 코드 직접 이식 가능

P2 완료 후 (프로덕션):
  - 신뢰성: 메모리 버그 제거
  - 예측성: 함정 제거
```

---

## 🎯 **다음 단계**

**지금**: 분석 완료 (이 문서)
**다음**: Phase 1 구현 시작 (253 노드에서)
  - cgc-defn auto-do wrapping
  - cgc-parse-try parse-primary
  - 단위 테스트

**예상 소요**: 6-7시간 (1주 이내)
