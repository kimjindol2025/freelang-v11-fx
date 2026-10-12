# FX 운영·조사 체크리스트

상태: `ACTIVE`  
기록일: 2026-10-11  
출처: Phase 0에서 함정 목록(`FX-TRAPS.airc`)의 **DOCS_PROCESS** 5항목을 여기로 이전  
상위: [`docs/decisions/FX-TRAPS-RECLASS-PHASE0.md`](decisions/FX-TRAPS-RECLASS-PHASE0.md)

이 문서는 **언어 시맨틱 함정이 아닙니다.**  
노드·조사·치환·계열 혼동 같은 운영 습관을 모읍니다.  
옛 함정 id는 그대로 두어 다음 AI가 검색·역추적으로 찾을 수 있게 합니다.

정본 함정 파일의 포인터: `FX-TRAPS.airc` → `@relocated` 절.

---

## 색인 (옛 trap id → 여기)

| 옛 trap id | 이 문서 앵커 | 한 줄 |
|------------|--------------|--------|
| `trap-free-locale` | [#ops-free-locale](#ops-free-locale) | `free` 출력 파싱 금지 → `/proc/meminfo` |
| `trap-runtime-header` | [#ops-runtime-header](#ops-runtime-header) | 빌트인 부재 단정 전 `aliases.c` 전수 |
| `trap-replace-all-rename` | [#ops-replace-all-rename](#ops-replace-all-rename) | 일괄 치환 후 함수명 오염 grep |
| `trap-cgc-stale-node` | [#ops-cgc-stale-node](#ops-cgc-stale-node) | 함정 전 cgc-bin 날짜·소스 대조 |
| `trap-v11-hof-closure-bug` | [#ops-v11-hof-scope](#ops-v11-hof-scope) | v11 인터프리터 조사 범위 — FX 함정 아님 |

---

## ops-free-locale

**옛 id:** `trap-free-locale`  
**FX-TRAPS 포인터:** `@relocated id: trap-free-locale`

### 지킬 것

- [ ] 메모리 사용률을 `free` 명령 출력 파싱으로 구하지 않는다.
- [ ] `/proc/meminfo`의 `MemTotal` / `MemAvailable`을 읽는다.

### 안전 패턴

```bash
awk '/MemTotal/{t=$2}/MemAvailable/{a=$2}END{print (1-a/t)*100}' /proc/meminfo
```

### 이유

`free` 헤더·단위는 locale에 따라 달라 노드마다 파싱이 깨진다. 언어/컴파일러 버그가 아니다.

---

## ops-runtime-header

**옛 id:** `trap-runtime-header`  
**관련 lock:** `lock-runtime-checklist` (`FX-TRAPS.airc`)  
**FX-TRAPS 포인터:** `@relocated id: trap-runtime-header`

### 지킬 것

- [ ] 「이 빌트인 없다」고 단정하기 전에 `runtime/aliases.c`와 `runtime/*.c`를 전수 검색한다.
- [ ] 새 runtime 빌트인을 추가할 때 아래 5곳을 같이 갱신한다  
      (`runtime/<name>.c`, `runtime/runtime.h`, `runtime/aliases.c`, `fl-build.sh`, `verify-fixpoint.sh`).

### 안전 패턴

```bash
grep -rn 'FLValue <name>' runtime/*.c runtime/*.h
```

### 이유

`process.c`만 보고 `shell_run`/`future`/`sleep_ms`가 없다고 결론 낸 사례가 있었다. 실제로는 `aliases.c`에 있다. 조사 범위 문제다.

---

## ops-replace-all-rename

**옛 id:** `trap-replace-all-rename`  
**FX-TRAPS 포인터:** `@relocated id: trap-replace-all-rename`

### 지킬 것

- [ ] 테이블명·문자열 `replace_all` / 일괄 치환 후, **함수명·심볼 오염**을 grep으로 확인한다.
- [ ] 치환 패턴을 테이블/SQL 문자열에만 좁힌다.

### 안전 패턴

```bash
# 예: plans → fx_plans 치환 후
grep -n "defn fx_" *.fl
grep -n "defn plans" *.fl
```

### 이유

`plans` → `fx_plans` 치환 시 `defn plans-all`까지 바뀌어 미정의 호출이 난 사례. 에디터/치환 습관이지 언어 규칙이 아니다.

---

## ops-cgc-stale-node

**옛 id:** `trap-cgc-stale-node`  
**FX-TRAPS 포인터:** `@relocated id: trap-cgc-stale-node`

### 지킬 것

- [ ] 「언어가 깨졌다」고 결론 내기 전에 **이 노드의 cgc-bin 날짜**와 `self/cgc-main.fl` 수리를 대조한다.
- [ ] runtime `*.c` 이슈는 노드에서 바로 재컴파일되므로, 컴파일러 바이너리 전파 지연과 구분한다.
- [ ] 고정점 게이트: `./verify-fixpoint.sh`, `./scripts/verify-native-bootstrap.sh`.

### 안전 패턴

```bash
ls -la "$(command -v cgc-bin 2>/dev/null)" \
  ../freelang-afj/bin/cgc-bin \
  bootstrap/stage0-bin 2>/dev/null
# 소스 수리 여부
rg -n '<증상키워드>' self/cgc-main.fl
```

### 이유

많은 「함정」이 구버전 cgc-bin 전파 지연이었다. 소스에는 고쳐져 있고 바이너리만 오래된 경우가 있다.

---

## ops-v11-hof-scope

**옛 id:** `trap-v11-hof-closure-bug`  
**FX-TRAPS 포인터:** `@relocated id: trap-v11-hof-closure-bug`

### 지킬 것

- [ ] 이 항목을 **FX native 함정으로 다시 올리지 않는다** (조사 범위 = v11 인터프리터).
- [ ] HOF/클로저 이슈가 FX에서 재현되면 **새 FX trap id**로 실측 후 등록한다. 이 id를 재사용하지 않는다.
- [ ] AFJ/v11 쪽 조사는 `freelang-afj` 문서·테스트를 본다.

### 기록

2026-06-26 v11 직접 실행에서 recursive-hof depth=2 callback이 정상으로 나온 기록이 있다 (`v11_status: UNVERIFIED`로 남아 있던 claimed). FX Phase 0에서는 언어 함정 목록에서 제외한다.

### 이유

계열 혼동. FX 함정 목록에 v11-only 조사를 두면 AI가 네이티브 수리로 오인한다.

---

## ops-set-interval-host

**옛 id:** `trap-set-interval-swallow`  
**FX-TRAPS 포인터:** `@relocated id: trap-set-interval-swallow`

### 지킬 것

- [ ] FX native에서 `(set_interval …)` / `(sleep_ms …)` 링크가 **undefined reference**로 실패하는 것을 「타이머 삼킴 버그」로 오인하지 않는다.
- [ ] 콜백 throw 후 stderr에 `set_interval callback error`가 찍히고 다음 틱이 이어지는 동작은 **Node/AFJ 호스트**(`freelang-afj` bootstrap / interpreter) 소관이다. FX ELF 수리 안건이 아니다.
- [ ] FX에 네이티브 타이머를 넣을 때는 새 runtime API + aliases + fl-build 동기화로 **새 trap id**를 실측 등록한다. 이 id를 재사용하지 않는다.
- [ ] AFJ 쪽 방어는 `fx-std/safe.fl`의 `safe-tick-run` / atom stop 플래그 패턴을 본다 (호스트가 `set_interval`을 제공할 때).

### 회귀 (FX unlinked 잠금)

```text
COMMAND=bash tests/traps/run-trap-set-interval-unlinked.sh
RESULT=TRAP_SET_INTERVAL_UNLINKED=PASS
```

### 기록

2026-10-13 Phase1 ⑥ 실측: `fl-build` → `undefined reference to set_interval` (및 `sleep_ms`).  
AFJ `bootstrap.js` `set_interval`은 catch 후 `console.error("set_interval callback error…")` 하고 인터벌을 유지한다.

### 이유

계열 혼동. FX 활성 함정에 두면 AI가 cgc/runtime에 타이머 삼킴 수리를 넣으려 한다. 실제 FX 상태는 **API 미링크**다.

---

## 변경 규칙

1. 여기 항목을 다시 `FX-TRAPS.airc` active `@trap`으로 올리려면 **FX에서 새로 실측**하고 Phase 0 재분류 합의가 필요하다.
2. 체크리스트만 고칠 때는 이 파일과 `@relocated`의 `moved_to` 앵커가 일치하는지 확인한다.
3. active 함정 개수(`trap_count`)에는 relocated 항목을 넣지 않는다.
