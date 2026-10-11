#!/bin/bash
# fl-lint — FreeLang 파일 사전 검사 (컴파일 없이)
# 사용: fl-lint <file.fl> [file2.fl ...]
#
# 검사 항목:
#   1. 괄호/브라켓 균형
#   2. 문자열 닫힘
#   3. $ 파라미터 패턴 (Clojure 스타일 감지)
#   4. 빈 defn/let 감지
#   5. FX-TRAPS / dialect 탐지 (active traps + FX naming card)
#      archived 2026-10-11: defn-do, count-type, str-includes-int, throw-invalid,
#      server-html-quote, string-literal-1024, try-map, inc-dec, cache, docstring
#      2026-10-12: [dialect-marker] 방언 표지 없음 → 경고만 (거부 금지). docs/FX-NAME-CARD.md
#      2026-10-12: [effect-anno] @in/@out/@effect 형식이 틀렸을 때만 경고 (부재는 무시).
#                  docs/FX-BUILTIN-EFFECTS.airc

PASS=0
FAIL=0

check_file() {
  local file="$1"
  local errors=()
  local warnings=()
  local depth=0 bracket=0 in_str=0 escape=0
  local lineno=0 str_start=0
  local has_dialect_marker=0

  while IFS= read -r line || [ -n "$line" ]; do
    lineno=$((lineno + 1))
    local i=0

    # 방언 표지: ;; dialect: fx  (경고만, errors에 넣지 않음 — docs/FX-NAME-CARD.md)
    if echo "$line" | grep -qiP '^\s*;;\s*dialect\s*:\s*fx\b'; then
      has_dialect_marker=1
    fi

    # 줄 단위 검사
    # ─ [W1] def 대신 define 써야 함
    if echo "$line" | grep -qP '^\s*\(def\s+[a-zA-Z]'; then
      warnings+=("  줄 $lineno [W1] (def ...) → (define ...) 를 사용하세요")
    fi

    # ─ [W2] (fn [x] ...) — $ 없는 파라미터
    if echo "$line" | grep -qP '\(fn\s+\[[^$\]]+\]'; then
      warnings+=("  줄 $lineno [W2] fn 파라미터에 \$ 없음: $(echo "$line" | grep -oP '\(fn\s+\[[^\]]+\]')")
    fi

    # ─ [W3] (defn name [x] ...) — $ 없는 파라미터
    if echo "$line" | grep -qP '\(defn\s+\S+\s+\[[^$\]]+\]'; then
      warnings+=("  줄 $lineno [W3] defn 파라미터에 \$ 없음: $(echo "$line" | grep -oP '\(defn\s+\S+\s+\[[^\]]+\]')")
    fi

    # ─────────── FX-TRAPS 탐지기 (lint_detectable:true 소비) ───────────
    # 원칙: lint 는 경고만 한다. 수정하지 않는다. (정책=FX-TRAPS / 탐지=lint / 수정=runtime)
    # 각 경고는 FX-TRAPS.airc 의 trap/lock id 를 출처로 단다.

    # [dialect-fx-api] AFJ-style kebab API — FX naming card (SIX-ITEMS-BY-DIALECT)
    if echo "$line" | grep -qP '\((server|json|res|http|sqlite|mariadb)-[a-z]'; then
      warnings+=("  줄 $lineno [dialect-fx-api] FX API는 underscore: $(echo "$line" | grep -oP '\((server|json|res|http|sqlite|mariadb)-[a-z!_?-]+' | head -1)")
    fi

    # [effect-anno] 주석을 썼는데 형식이 틀릴 때만 경고. 주석 없음 → 경고 없음.
    # 규약: ;; @in … @out … @effect pure|io|net|db|process
    if echo "$line" | grep -qP '^\s*;;.*@(in|out|effect)\b'; then
      if ! echo "$line" | grep -qP '^\s*;;\s*@in\s+\S.*\s+@out\s+\S.*\s+@effect\s+(pure|io|net|db|process)\s*$'; then
        warnings+=("  줄 $lineno [effect-anno] 형식: ;; @in … @out … @effect pure|io|net|db|process (docs/FX-BUILTIN-EFFECTS.airc)")
      fi
    fi

    # trap-server-json-string archived 2026-10-11 — runtime auto-stringifies map/vector

    # 주의: trap-runtime-header / lock-runtime-checklist 는 단일 .fl 범위 밖
    #       (runtime 함수 추가 시 5파일 동기화) → 빌드/커밋 훅 영역, lint 비대상.

    # 문자/괄호 깊이 추적
    local len=${#line}
    while [ $i -lt $len ]; do
      local c="${line:$i:1}"
      if [ $escape -eq 1 ]; then
        escape=0
      elif [ "$c" = '\\' ] && [ $in_str -eq 1 ]; then
        escape=1
      elif [ "$c" = '"' ]; then
        if [ $in_str -eq 0 ]; then
          in_str=1; str_start=$lineno
        else
          in_str=0
        fi
      elif [ $in_str -eq 0 ]; then
        if [ "$c" = ';' ]; then break; fi  # 주석
        case "$c" in
          '(') depth=$((depth+1)) ;;
          ')') depth=$((depth-1))
               [ $depth -lt 0 ] && errors+=("  줄 $lineno [E1] 닫는 괄호 초과") ;;
          '[') bracket=$((bracket+1)) ;;
          ']') bracket=$((bracket-1))
               [ $bracket -lt 0 ] && errors+=("  줄 $lineno [E2] 닫는 브라켓 초과") ;;
        esac
      fi
      i=$((i+1))
    done
  done < "$file"

  # 파일 끝 검사
  [ $depth -ne 0 ] && errors+=("  EOF [E1] 괄호 미닫힘: 깊이 $depth")
  [ $bracket -ne 0 ] && errors+=("  EOF [E2] 브라켓 미닫힘: 깊이 $bracket")
  [ $in_str -ne 0 ] && errors+=("  EOF [E3] 문자열 미닫힘 (줄 $str_start 에서 시작)")

  # [dialect-marker] 표지 없음 → 경고만 (errors 금지 — 빌드 거부 아님). docs/FX-NAME-CARD.md
  if [ "$has_dialect_marker" -eq 0 ]; then
    warnings+=("  줄 1 [dialect-marker] 방언 표지 없음 — 파일 상단에 \`;; dialect: fx\` 권장 (docs/FX-NAME-CARD.md)")
  fi

  # 결과 출력
  local fname
  fname="$(basename "$file")"
  if [ ${#errors[@]} -eq 0 ] && [ ${#warnings[@]} -eq 0 ]; then
    echo "✅ $fname ($lineno 줄)"
    PASS=$((PASS+1))
  else
    echo "📋 $fname ($lineno 줄)"
    for e in "${errors[@]}"; do echo "$e"; done
    for w in "${warnings[@]}"; do echo "$w"; done
    FAIL=$((FAIL+1))
  fi
}

if [ $# -eq 0 ]; then
  echo "사용: fl-lint <file.fl> [file2.fl ...]"
  exit 1
fi

for f in "$@"; do
  if [ -f "$f" ]; then
    check_file "$f"
  else
    echo "❌ 파일 없음: $f"
    FAIL=$((FAIL+1))
  fi
done

echo "────────────────────────"
echo "✅ $PASS 개 통과  |  ❌ $FAIL 개 실패"

[ $FAIL -eq 0 ]
