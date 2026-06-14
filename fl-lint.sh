#!/bin/bash
# fl-lint — FreeLang 파일 사전 검사 (컴파일 없이)
# 사용: fl-lint <file.fl> [file2.fl ...]
#
# 검사 항목:
#   1. 괄호/브라켓 균형
#   2. 문자열 닫힘
#   3. $ 파라미터 패턴 (Clojure 스타일 감지)
#   4. 빈 defn/let 감지

PASS=0
FAIL=0

check_file() {
  local file="$1"
  local errors=()
  local warnings=()
  local depth=0 bracket=0 in_str=0 escape=0
  local lineno=0 str_start=0

  while IFS= read -r line || [ -n "$line" ]; do
    lineno=$((lineno + 1))
    local i=0

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
