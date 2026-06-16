#!/bin/bash
# fl-lint — FreeLang 파일 사전 검사 (컴파일 없이)
# 사용: fl-lint <file.fl> [file2.fl ...]
#
# 검사 항목:
#   1. 괄호/브라켓 균형
#   2. 문자열 닫힘
#   3. $ 파라미터 패턴 (Clojure 스타일 감지)
#   4. 빈 defn/let 감지
#   5. FX-TRAPS 탐지 (kebab/count/str-includes/throw/server-json/html-quote/string-1024/defn-do)

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

    # ─────────── FX-TRAPS 탐지기 (lint_detectable:true 소비) ───────────
    # 원칙: lint 는 경고만 한다. 수정하지 않는다. (정책=FX-TRAPS / 탐지=lint / 수정=runtime)
    # 각 경고는 FX-TRAPS.airc 의 trap/lock id 를 출처로 단다.

    # [trap-kebab-symbol] kebab API 호출 — fx 는 underscore
    if echo "$line" | grep -qP '\((server|json|res|http|sqlite|mariadb)-[a-z]'; then
      warnings+=("  줄 $lineno [trap-kebab-symbol] kebab API — fx 는 underscore: $(echo "$line" | grep -oP '\((server|json|res|http|sqlite|mariadb)-[a-z!_?-]+' | head -1)")
    fi

    # [trap-count-type] count 직접 사용 — 설치본 cgc 미지원(fl_fn_call 타입에러)
    if echo "$line" | grep -qP '\(count\s'; then
      warnings+=("  줄 $lineno [trap-count-type] (count ...) — SQL COUNT 또는 length 로 (설치본 cgc 미지원)")
    fi

    # [lock-str-includes / trap-str-includes-int] if 에 str_includes 직접
    if echo "$line" | grep -qP '\(if\s+\(str[_-]includes'; then
      warnings+=("  줄 $lineno [lock-str-includes] (if (str_includes ..)) 금지 — (= (str_includes x y) 1)")
    fi

    # [lock-no-throw / trap-throw-invalid-initializer]
    if echo "$line" | grep -qP '\(throw\s'; then
      warnings+=("  줄 $lineno [lock-no-throw] (throw ...) 금지 — nil/false 반환으로 (invalid C initializer 위험)")
    fi

    # [trap-server-json-string] server_json 에 맵/벡터 직접 전달
    if echo "$line" | grep -qP '\(server_json\s+[\{\[]'; then
      warnings+=("  줄 $lineno [trap-server-json-string] server_json 은 문자열만 — json_stringify 로 감싸기 (false-200)")
    fi

    # [trap-server-html-quote] server_html 줄에 이스케이프된 더블쿼트
    if echo "$line" | grep -qP 'server_html' && echo "$line" | grep -qP '\\"'; then
      warnings+=("  줄 $lineno [trap-server-html-quote] server_html 안 \\\" — HTML 속성은 single-quote 로")
    fi

    # [trap-string-literal-1024] 단일 문자열 리터럴 >1024B → tmpfile PRoot 실패
    if echo "$line" | grep -qP '"[^"]{1024,}"'; then
      warnings+=("  줄 $lineno [trap-string-literal-1024] 문자열 리터럴 >1024B — (str \"조각\" \"조각\") 로 분할")
    fi

    # [trap-defn-do] 한 줄 defn 본문 다중표현식(do/let 없이) — 보수적 휴리스틱(멀티라인은 v2)
    if echo "$line" | grep -qP '\(defn\s+\S+\s+\[[^\]]*\]\s*\(' \
       && ! echo "$line" | grep -qP '\(defn\s+\S+\s+\[[^\]]*\]\s*\((do|let|if|when|cond|case|try|->|->>)[\s)]'; then
      body="${line#*]}"
      if echo "$body" | grep -qP '\)\s*\('; then
        warnings+=("  줄 $lineno [trap-defn-do] defn 본문이 다중표현식으로 보임 — (do ...) 로 감싸세요 (첫 줄만 실행 위험)")
      fi
    fi
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
