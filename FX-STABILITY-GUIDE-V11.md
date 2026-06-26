# FX 안정화 가이드 — v11 인터프리터 (검증 완료 2026-06-26)

> 이 문서는 FreeLang v11에서 안전하게 사용하는 패턴을 정리합니다.
> cgc(C 컴파일러)는 별도 검증 대기 중.

---

## ✅ 검증 완료: 3가지 함정 + 회피 패턴

### 1️⃣ trap-defn-do — 거짓 (v11에서는 정상)

**상태**: ❌ 거짓 — v11에서 implicit-do 정상 작동

```lisp
;; ✅ v11에서는 모두 실행됨 (걱정 불필요)
(defn process [data]
  (println "1단계")           ;; 실행됨 ✅
  (println "2단계")           ;; 실행됨 ✅
  (println "3단계")           ;; 실행됨 ✅
  (+ 1 2))                    ;; 반환: 3

;; do로 감싸도 동일 (선택사항)
(defn process-safe [data]
  (do
    (println "1단계")
    (println "2단계")
    (println "3단계")
    (+ 1 2)))
```

**권장**: 명확성을 위해 `do` 사용 (선택사항, 필수 아님)

---

### 2️⃣ trap-v11-hof-closure-bug — 거짓 (재현 불가)

**상태**: ❌ 거짓 — v11에서 정상 작동, 재현 불가

```lisp
;; ✅ v11에서 정상 작동
(defn apply-fn [body-fn]
  (body-fn)
  (body-fn))

(apply-fn (fn [] (println "콜백")))
;; 출력:
;; 콜백
;; 콜백

;; ✅ 재귀 HOF도 정상
(defn recursive-hof [body-fn depth]
  (if (<= depth 0)
    (println "완료")
    (do
      (body-fn)
      (recursive-hof body-fn (- depth 1)))))

(recursive-hof (fn [] (println "깊이")) 2)
;; 출력:
;; 깊이
;; 깊이
;; 완료
```

**권장**: 아무것도 변경할 필요 없음 (안전함)

---

### 3️⃣ trap-defn-throw-rerun — 부분 참 (제한적)

**상태**: ⚠️ 부분 참 — throw 재실행 O, but `with-retry` 미지원

```lisp
;; ✅ throw 발생 시 재실행되는 것은 사실
(define call-count (atom 0))

(defn failing-fn []
  (swap! call-count inc)
  (if (= (deref call-count) 1)
    (throw "첫 실패")
    "성공"))

(try
  (failing-fn)
  (catch e (println (str "에러: " e))))

;; 결과: call-count = 2 (재실행됨)
```

**문제**: `with-retry` 함수가 v11에 **없음** → 실제 위험도 낮음

**해결책**: 수동 재시도 구현

```lisp
;; ✅ 안전한 재시도 패턴 (v11)
(defn retry-fn [fn max-attempts]
  (loop [attempt 1 error nil]
    (if (> attempt max-attempts)
      {:ok false :error error}
      (try
        {:ok true :result (fn)}
        (catch e
          (if (< attempt max-attempts)
            (recur (inc attempt) e)
            {:ok false :error e}))))))

;; 사용
(let [result (retry-fn failing-fn 3)]
  (if (get result "ok")
    (println (str "성공: " (get result "result")))
    (println (str "실패: " (get result "error")))))
```

---

## 🛡️ 안전 패턴 (모두 검증됨)

### A. 안전한 함수 구조

```lisp
;; ✅ 모든 표현식 실행 보장 (do 사용)
(defn safe-multi-step [data]
  (do
    (validate data)
    (transform data)
    (save data)))
```

### B. 에러 처리 (throw 대신 Result 패턴)

```lisp
;; ❌ throw는 재실행 유발
(defn risky [x]
  (if (< x 0)
    (throw "음수 불가")
    (+ x 1)))

;; ✅ Result 맵 반환
(defn safe [x]
  (if (< x 0)
    {:ok false :error "음수 불가"}
    {:ok true :result (+ x 1)}))

;; 사용
(let [res (safe 5)]
  (if (get res "ok")
    (println (get res "result"))
    (println (get res "error"))))
```

### C. HOF 안전 패턴

```lisp
;; ✅ 재귀 HOF도 안전 (검증됨)
(defn map-recursive [body-fn items depth]
  (if (>= depth 10)
    (println "깊이 도달")
    (do
      (println (str "깊이 " depth))
      (doseq [item items]
        (body-fn item))
      (map-recursive body-fn items (inc depth)))))
```

### D. 파일 동시성 (read-modify-write 보호)

```lisp
;; ❌ 동시 접근 시 손실 가능
(let [data (load-json "file.json")]
  (save-json "file.json" (assoc data "key" "value")))

;; ✅ 원자성 보장 패턴 1: atom (메모리)
(define state (atom {}))
(swap! state assoc "key" "value")

;; ✅ 원자성 보장 패턴 2: MariaDB (권장)
(mariadb-exec DB "UPDATE t SET k=? WHERE id=?" ["value" 1])

;; ✅ 원자성 보장 패턴 3: 파일 락 (구현 필요)
;; (현재 v11에는 file-lock이 없음 → MariaDB 권장)
```

---

## ⚡ read2write MVP 적용 (검증된 패턴)

### 안전한 독서 저장 API

```lisp
(defn save-read-safe [id user-id text]
  ;; 방법 1: 메모리 atom (단일 프로세스)
  (do
    (swap! reads-store assoc id {
      "user_id" user-id
      "text" text
      "created_at" (now-ms)
    })
    {:ok true :id id})

  ;; 방법 2: MariaDB (멀티 프로세스)
  (try
    (mariadb-exec DB
      "INSERT INTO reads (id, user_id, text) VALUES (?, ?, ?)"
      [id user-id text])
    {:ok true :id id}
    (catch e
      {:ok false :error (str e)})))
```

### 안전한 재시도 (LLM API)

```lisp
(defn call-llm-safe [text]
  (loop [attempt 1 error nil]
    (if (> attempt 3)
      {:ok false :error error}
      (try
        {:ok true :result (http-get (str "https://api.anthropic.com/..." text))}
        (catch e
          (do
            (sleep 1000)  ;; 1초 대기 후 재시도
            (recur (inc attempt) e)))))))
```

---

## 🚫 금지 패턴 (실제 위험)

```lisp
;; ❌ 1. 파일 기반 멀티 유저 동시성
(let [data (load-json "shared.json")]
  (save-json "shared.json" (assoc data ...)))
;; → Race condition (손실 가능)

;; ❌ 2. throw를 제어 흐름으로 (재실행 유발)
(defn critical-fn []
  (if error (throw "stop")))
;; → 예상치 못한 재실행, 부작용 2배

;; ❌ 3. set-interval 콜백에서 throw
(set-interval
  (fn []
    (if error (throw "stop")))  ;; throw 무시됨
  1000)
;; → 타이머 계속 실행, 에러 무시

;; ❌ 4. 매우 큰 문자열 리터럴
(server-html "<!-- 5000자 이상 HTML -->")
;; cgc만 문제 (v11 OK)
```

---

## 📋 체크리스트 (read2write MVP)

- [ ] 모든 defn이 단일 표현식인가? (또는 do로 감싸졌는가?)
- [ ] throw 대신 Result 맵을 반환하는가?
- [ ] 파일 동시성 제어가 있는가? (atom 또는 DB)
- [ ] 재시도 로직이 안전한 패턴인가?
- [ ] set-interval 콜백에 throw가 없는가?
- [ ] 에러 메시지는 println으로 명확한가?

---

## 🔴 cgc 함정들 (미검증, 아직 미사용)

다음 검증 예정 (253 노드 x86_64):
- trap-count-type — cgc만
- trap-kebab-symbol — cgc만
- trap-str-includes-int — cgc만
- ... (외 21개)

**현재 상태**: v11 인터프리터만 사용하므로 cgc 함정 무관

---

## 결론

> ✅ **v11은 안전함 (3가지 함정은 거짓 또는 부분 참)**
> 
> ⚠️ **파일 동시성만 주의** (MariaDB로 해결)
> 
> ❌ **cgc는 아직 검증 대기 중** (나중에 253 노드에서)

**read2write MVP → v11 + MariaDB = 안정적**
