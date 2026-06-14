# fx 소개 — 처음 들어온 Claude에게

> 너는 지금 **FreeLang fx** 폴더(`/root/kim/freelang-v11-fx`)에 있다.
> 이 한 장만 읽으면 fx로 코딩을 시작할 수 있다. 5분 컷.

---

## 한 문장

**fx = FreeLang 정식 언어.** Lisp 문법의 `.fl` 파일을 **C로 컴파일해 단일 ELF 바이너리**로 만든다 (Node.js 불필요, 3-5MB, 빠름).

---

## 30초 안에 첫 빌드 (이 노드에서 실측됨 ✅)

```bash
cd /root/kim/freelang-v11-fx
printf '(print "hello fx\\n")\n(print (str "1+2=" (+ 1 2) "\\n"))\n' > /tmp/h.fl

# ⚠️ 이 노드(/root)에서는 반드시 .fl-build-root.sh 를 써라
bash .fl-build-root.sh /tmp/h.fl /tmp/h     # .fl → C → gcc → ELF
/tmp/h                                       # → hello fx / 1+2=3
```

빠른 실험만 할 땐 빌드 없이 인터프리터로:
```bash
node bootstrap.js run /tmp/h.fl     # 또는: bash fl-repl.sh (대화형)
```

---

## 두 런타임을 절대 헷갈리지 마라

| | 인터프리터 | **fx C 네이티브** (= 본체) |
|---|---|---|
| 실행 | `node bootstrap.js run x.fl` | `bash .fl-build-root.sh x.fl out` → `./out` |
| 함수명 | `server-get` (keb**-**케밥) | `server_get` (under**_**스코어) |
| 핸들러 | `(fn [$req] ...)` 인라인 | `"handle-name"` **문자열** |
| 응답 | `(server-json {...})` | `(server_json (json_stringify {...}))` |

→ **섞으면 컴파일/런타임 오류.** fx 코드를 쓸 땐 underscore + 문자열 핸들러 + json_stringify.

---

## 반드시 지킬 규칙 5개

1. **⭐ `defn` 본문은 단일 표현식.** 여러 줄 나열하면 첫 줄만 실행되고 **나머지가 조용히 사라진다**(에러도 없음). 다중 로직은 `(do ...)`로 감싸라.
   ```lisp
   (defn f [] (println "1") (println "2"))      ;; ❌ "1"만
   (defn f [] (do (println "1") (println "2"))) ;; ✅
   ```
   함수 중간 로직이 안 도는 것 같으면 **빌트인 의심 전에 `do`부터 확인.**

2. **이 노드(/root)는 `.fl-build-root.sh`.** `fl-build.sh`는 다른 노드(`/home/kimjin`)용이라 여기선 cgc-bin을 못 찾는다.

3. **빌드 PASS ≠ 동작.** 200 OK, exit 0 믿지 말고 **실제 실행 출력으로 검증**한다 (이 프로젝트의 false-200 규율).

4. **cgc-bin은 아키텍처별 바이너리** (이 노드 = aarch64). 다른 노드 산출물 복사 금지, 노드별 재빌드.

5. **HTTP 클라이언트는 HTTP만**(HTTPS 미지원). curl shim은 POST 바디가 깨지니 검증은 python raw 소켓으로.

---

## 자주 쓰는 fx 패턴

```lisp
;; HTTP 서버
(defn handle-get [$req] (server_json (json_stringify {"ok" true})))
(server_get  "/path"      "handle-get")
(server_req_param $req "id")      ; path param (/x/:id)
(server_req_query $req "name")    ; ?name=...
(server_req_body  $req)           ; POST body (→ json_parse)

;; SQLite (? 바인딩 = SQL 인젝션 방어)
(define db (sqlite_open "data/app.db"))
(sqlite_exec_p  db "INSERT INTO t VALUES (?,?)" (list $a $b))
(sqlite_query_p db "SELECT * FROM t WHERE id=?" (list $id))
(sqlite_one_p   db "SELECT * FROM t WHERE id=?" (list $id))   ; 없으면 nil

;; HTTP 클라이언트 (실패 시 nil)
(http_get "http://localhost:18080/api")

;; Threading
(-> 3 add1 (+ 10) str)                 ; "14"
(->> (list 1 2 3) (filter ...) (map ...))
```

---

## 새 앱 시작 / 도구

```bash
bash fl-new.sh    # templates(hello·api-server·kim-notes·kim-short) 기반 새 프로젝트
bash fl-repl.sh   # REPL    bash fl-test.sh   # 테스트    bash fl-watch.sh  # 자동 재빌드
```

---

## 더 읽을 것 (필요할 때만)

- **`CLAUDE.md`** — fx API 레퍼런스 전체 (HTTP/SQLite/MariaDB/JSON/함정 목록·수정 이력). 가장 권위 있음.
- **`FX-STATE-2026-06-14.md`** — 현재 상태 스냅샷(검증된 사실·미해결·빌드체계 상세).
- **`fx-files/LANGUAGE-NOTES.md`** — 실전 삽질로 발견한 언어 노트.
- ⚠️ `README.md`는 구버전 서술(인터프리터 실험 시절). **CLAUDE.md가 정식.**

---

*FreeLang fx = 정식 언어. 이 문서는 새 Claude 세션의 진입점. 30초 빌드는 2026-06-14 이 노드에서 실측됨.*
