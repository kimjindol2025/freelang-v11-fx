# P0 PATCH 구현 요청서
**작성**: claude (2026-06-26)  
**대상 노드**: 253 (x86_64, FreeLang 주력)  
**예상 작업시간**: 6-7시간  
**우선도**: 🔴 **P0 (치명적)**

---

## 📋 요청 사항

현재 노드(/root, PRoot aarch64)에서는 cgc 빌드가 불가능하므로, **253 노드(x86_64)에서 P0 패치 3개를 구현**해달라는 요청입니다.

---

## 🎯 P0 패치 요약

### P0-3: cgc-defn — auto-do wrapping (2시간)
**파일**: `/root/kim/freelang-v11-fx/self/cgc-main.fl` (line 1973)  
**문제**: defn 첫 줄이 문자열이면 그것을 반환값으로 처리 (docstring 미인식)  
**해결책**: body 표현식 다중 시 자동으로 (do ...) 감싸기

```lisp
;; 변경 전
(defn get-user [id]
  "사용자 조회"
  (http-get (str "/api/users/" id)))
;; 결과: "사용자 조회" 반환 (함수 기능 없음)

;; 변경 후 (자동 wrapping)
(defn get-user [id]
  (do
    "사용자 조회"
    (http-get (str "/api/users/" id))))
;; 결과: docstring 버려지고 http-get 반환값 반환
```

**패치 위치**: cgc-defn 함수 (line 1973-2005)에서:
1. body-exprs 추출 (args[2:])
2. 다중 표현식이면 {:kind "sexpr" :op "do" :args body-exprs} 생성
3. updated-args로 수정된 인자 전달

**상세**: `/root/kim/freelang-v11-fx/P0-patches.fl` 참고

---

### P0-2: cgc-parse-try — parse-primary (3시간)
**파일**: `/root/kim/freelang-v11-fx/self/cgc-main.fl` (line ~924)  
**문제**: try 본문에 맵 리터럴 `{...}` 직접 사용 시 파싱 오류  
**해결책**: try 파싱을 parse-primary로 변경 (모든 리터럴 허용)

```lisp
;; 변경 전 (cgc 파싱 오류)
(try
  {:ok true :value 42}
  (catch e {:ok false}))

;; 변경 후 (parse-primary 사용)
;; 위 코드가 정상 작동
```

**패치 위치**: cgc-try 또는 cgc-parse-try 함수에서:
1. try body 파싱을 parse-expr 대신 parse-primary 사용
2. 맵, 벡터, 숫자 등 모든 기본 표현식 허용

**상세**: `/root/kim/freelang-v11-fx/P0-patches.fl` 참고

---

### P0-1: cgc-defn-do — 재검증 (1시간)
**상태**: **이미 지원됨** (cgc-defn-stmts가 모든 body 처리)  
**P0-3 적용으로 자동 해결**

---

## 🔧 구현 단계

### 단계 1: 패치 적용 (2-3시간)

```bash
# 253 노드에서

cd /root/kim/freelang-v11-fx/self
cp cgc-main.fl cgc-main.fl.backup  # 백업

# cgc-defn 수정 (P0-3)
# 라인 1973-2005 사이에서:
# - body-exprs 추출
# - wrapped-body 생성 (다중이면 do로 감싸기)
# - $updated-args 사용

# cgc-parse-try 수정 (P0-2)
# 라인 924 근처에서:
# - parse-expr을 parse-primary로 변경
# - 또는 try 파싱 규칙 확장
```

### 단계 2: 컴파일 테스트 (1시간)

```bash
# cgc-main.fl이 자체 컴파일 가능한지 확인 (고정점 검증)
bash /root/kim/freelang-v11-fx/verify-fixpoint.sh
# ✅ 완전: gen-a == gen-b == gen-c
# ⚠️ 부분: gen-b == gen-c (cgc-bin 교체 후 재검증)
# ❌ 붕괴: gen-b != gen-c (디버그)

# 임시 cgc-bin 생성 및 테스트 컴파일
CGC_BIN=/tmp/cgc-bin-test bash /root/kim/freelang-v11-fx/fl-build.sh \
  /root/kim/freelang-v11-fx/P0-PATCH-TESTS.fl test-p0
```

### 단계 3: 단위 테스트 (1시간)

```bash
# P0-PATCH-TESTS.fl 실행
./test-p0

# 또는 v11에서 먼저 테스트
node /root/freelang-v11/bootstrap.js run P0-PATCH-TESTS.fl
```

### 단계 4: 회귀 테스트 (1시간)

```bash
# 기존 freelang 코드 컴파일 및 실행
# 프로덕션 서비스들 재시작

# read2write MVP 테스트 (있으면)
bash /root/kim/freelang-v11-fx/fl-build.sh \
  /root/kim/read2write-mvp/src/server.fl server-mvp
./server-mvp  # 헬스체크
```

### 단계 5: Gogs 커밋

```bash
cd /root/kim/freelang-v11-fx/self

# 변경사항 확인
git diff cgc-main.fl

# 커밋
git add cgc-main.fl
git commit -m "P0 PATCH: Fix defn docstring, try-map parsing

- P0-3: auto-do wrapping for multi-statement bodies
- P0-2: parse-primary for try body parsing (allow map/vector literals)
- P0-1: validated (already supported by cgc-defn-stmts)

Fixes:
  - trap-defn-docstring-cgc: docstring now properly handled
  - trap-try-map-literal: {:ok true} patterns now work
  - trap-defn-do: all body expressions guaranteed to execute

Verified: P0-PATCH-TESTS.fl passed
Fixpoint: verify-fixpoint.sh COMPLETE (gen-a == gen-b == gen-c)

Evidence:
- Commit SHA: $(git rev-parse HEAD)
- Test run: P0-PATCH-TESTS.fl output
"

# 푸시
git push origin main
```

---

## 📁 파일 목록

| 파일 | 용도 | 위치 |
|------|------|------|
| **P0-patches.fl** | 패치 설계 및 상세 | `/root/kim/freelang-v11-fx/P0-patches.fl` |
| **P0-PATCH-TESTS.fl** | 단위 테스트 케이스 | `/root/kim/freelang-v11-fx/P0-PATCH-TESTS.fl` |
| **cgc-main.fl** | 컴파일러 소스 (수정 대상) | `/root/kim/freelang-v11-fx/self/cgc-main.fl` |
| **CGC-IMPROVEMENT-ROADMAP.md** | 전체 로드맵 (P0-P2) | `/root/kim/freelang-v11-fx/CGC-IMPROVEMENT-ROADMAP.md` |
| **FX-TRAPS.airc v2.1** | 함정 명세 (검증 결과) | `/root/kim/freelang-v11-fx/FX-TRAPS.airc` |
| **FX-STABILITY-GUIDE-V11.md** | v11 안전 패턴 | `/root/kim/freelang-v11-fx/FX-STABILITY-GUIDE-V11.md` |

---

## ✅ 성공 기준

- [ ] cgc-main.fl 수정 완료 (P0-3, P0-2)
- [ ] verify-fixpoint.sh **COMPLETE** (gen-a == gen-b == gen-c)
- [ ] P0-PATCH-TESTS.fl 실행 ✅
- [ ] 기존 freelang 코드 회귀 테스트 ✅
- [ ] read2write MVP 컴파일 성공
- [ ] Gogs 커밋 완료

---

## 💡 기대 효과

```
현재 (P0 패치 전):
  - defn docstring 인식 불가 → 함수가 문자열만 반환
  - try-map 패턴 파싱 오류 → API 에러 응답 불가능
  - cgc 신뢰도 낮음 → v11만 사용 강제

P0 패치 후:
  - defn docstring 정상 작동 ✅
  - try-map 직접 사용 가능 ✅
  - cgc로 read2write MVP 컴파일 가능 ✅
  - 성능 330배 향상 가능 (메모리 70MB→3MB)
```

---

## 📞 연락처

- **로컬 (분석)**: claude (/root)
- **253 노드 (구현)**: kim (필요시)
- **Gogs**: gogs.dclub.kr/kim/freelang-v11-fx
- **작업 추적**: tasks.dclub.kr (태스크 ID: t_...)

---

**요청 상태**: ✅ 분석 완료, 🔄 구현 대기  
**예상 일정**: 이번 주 (253 노드 가용 시간에 따라)

---

## 부록: 빠른 참조

### cgc-main.fl 주요 함수 위치

```
line 924:  cgc-try 또는 cgc-parse-try
line 1611: cgc-defn-stmts (이미 다중 표현식 지원)
line 1973: cgc-defn (P0-3 패치 대상)
line 2001: cgc-defn 일반 경로 (updated-args 사용)
line 2005: cgc-defn 종료
```

### 테스트 방법

```bash
# v11 테스트 (로컬)
node /root/freelang-v11/bootstrap.js run P0-PATCH-TESTS.fl

# cgc 테스트 (253 노드, 패치 적용 후)
CGC_BIN=/root/freelang-v11/bin/cgc-bin bash fl-build.sh P0-PATCH-TESTS.fl test-p0
./test-p0
```

### 긴급 롤백

```bash
cd /root/kim/freelang-v11-fx/self
cp cgc-main.fl.backup cgc-main.fl
git checkout cgc-main.fl  # 또는
bash /root/kim/freelang-v11-fx/verify-fixpoint.sh  # 재검증
```

---

**작성**: claude (2026-06-26 분석 완료)  
**다음**: 253 노드 구현 시작
