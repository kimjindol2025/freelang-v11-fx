#!/bin/bash
# fl-repl — FreeLang 네이티브 대화형 REPL
# 사용: fl-repl [--std]
# 옵션: --std  fx-std.fl 자동 로드
#
# 입력 방식:
#   - 한 줄 표현식: (+ 1 2)  → 즉시 실행
#   - 멀티라인: 첫 줄이 여는 괄호로 끝나면 ) 닫힐 때까지 계속 받음
#   - :q 또는 Ctrl+D → 종료
#   - :history       → 이번 세션 히스토리
#   - :reset         → 정의 초기화
#   - :load <파일>   → 파일 로드

SCRIPT_REAL="$(readlink -f "$0")"
SCRIPT_DIR="$(cd "$(dirname "$SCRIPT_REAL")" && pwd)"
FL_TEST="$SCRIPT_DIR/fl-test.sh"
FX_STD="$SCRIPT_DIR/fx-std.fl"

USE_STD=0
if [ "$1" = "--std" ]; then USE_STD=1; fi

echo "FreeLang Native REPL (fx 런타임)"
echo "  :q = 종료  |  :reset = 초기화  |  :load <파일>"
echo "────────────────────────────────────────────────"

# 정의 누적 (세션 내 define/defn 저장)
SESSION_DEFS=""
HISTORY=()

# 괄호 균형 확인 (정수 반환: 양수=미닫힌, 0=균형, 음수=초과)
paren_balance() {
  local line="$1" depth=0 in_str=0 i
  for ((i=0; i<${#line}; i++)); do
    c="${line:$i:1}"
    if [ "$in_str" = "1" ]; then
      [ "$c" = '"' ] && in_str=0
    elif [ "$c" = '"' ]; then
      in_str=1
    elif [ "$c" = '(' ] || [ "$c" = '[' ]; then
      depth=$((depth+1))
    elif [ "$c" = ')' ] || [ "$c" = ']' ]; then
      depth=$((depth-1))
    elif [ "$c" = ';' ]; then
      break  # 줄 주석
    fi
  done
  echo $depth
}

run_expr() {
  local expr="$1"
  local tmpfl
  tmpfl=$(mktemp /tmp/fl_repl_XXXXXX.fl)

  # 세션 정의 + 현재 표현식 작성
  {
    [ -n "$SESSION_DEFS" ] && echo "$SESSION_DEFS"
    # define/defn은 그대로, 표현식은 println로 래핑
    if echo "$expr" | grep -qE '^\s*\((define|defn|load)\b'; then
      echo "$expr"
    else
      echo "(println $expr)"
    fi
  } > "$tmpfl"

  local std_flag=""
  [ "$USE_STD" = "1" ] && std_flag="--std"

  bash "$FL_TEST" $std_flag "$tmpfl" 2>&1
  local ec=$?
  rm -f "$tmpfl"
  return $ec
}

# define/defn 이면 세션에 누적
accumulate() {
  local expr="$1"
  if echo "$expr" | grep -qE '^\s*\((define|defn)\b'; then
    SESSION_DEFS="$SESSION_DEFS"$'\n'"$expr"
  fi
}

while true; do
  # 프롬프트
  printf "fl> "
  if ! IFS= read -r line; then
    echo ""
    break
  fi

  # 빈 줄 스킵
  [ -z "$(echo "$line" | tr -d ' \t')" ] && continue

  # 특수 명령
  case "$line" in
    :q|:quit|quit|exit) break ;;
    :reset)
      SESSION_DEFS=""
      echo "  세션 초기화됨"
      continue
      ;;
    :history)
      if [ ${#HISTORY[@]} -eq 0 ]; then
        echo "  (이력 없음)"
      else
        for i in "${!HISTORY[@]}"; do
          printf "  %2d  %s\n" "$((i+1))" "${HISTORY[$i]}"
        done
      fi
      continue
      ;;
    :load\ *)
      FILE="${line#:load }"
      FILE="${FILE//\'/}"
      FILE="${FILE//\"/}"
      if [ -f "$FILE" ]; then
        SESSION_DEFS="$SESSION_DEFS"$'\n'"(load \"$FILE\")"
        echo "  로드됨: $FILE"
      else
        echo "  ❌ 파일 없음: $FILE"
      fi
      continue
      ;;
  esac

  # 멀티라인 수집 (괄호 균형 맞을 때까지)
  EXPR="$line"
  BALANCE=$(paren_balance "$line")
  while [ "$BALANCE" -gt 0 ]; do
    printf "...  "
    if ! IFS= read -r cont; then
      echo ""
      break
    fi
    EXPR="$EXPR"$'\n'"$cont"
    ADD=$(paren_balance "$cont")
    BALANCE=$((BALANCE + ADD))
  done

  HISTORY+=("$(echo "$EXPR" | head -1)")

  # 실행
  RESULT=$(run_expr "$EXPR" 2>&1)
  RC=$?

  if [ $RC -eq 0 ]; then
    [ -n "$RESULT" ] && echo "$RESULT"
    accumulate "$EXPR"
  else
    echo "$RESULT"
  fi
done

echo "bye."
