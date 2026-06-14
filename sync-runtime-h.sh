#!/bin/bash
# sync-runtime-h — aliases.c 함수 선언을 runtime.h에 자동 동기화
ALIASES="/home/kimjin/freelang-v11-fx/runtime/aliases.c"
RUNTIME_H="/home/kimjin/freelang-v11-fx/runtime/runtime.h"

# aliases.c에서 public 함수(FLValue로 시작, static 아님) 추출
NEW_DECLS=$(grep "^FLValue " "$ALIASES" | grep -v "^FLValue __" | \
  sed 's/{.*/;/' | sed 's/ *$//' | sort -u)

# runtime.h에서 이미 있는 선언 확인
ADDED=0
while IFS= read -r decl; do
  # 함수명 추출
  FNAME=$(echo "$decl" | sed 's/^FLValue \([a-z_A-Z0-9]*\).*/\1/')
  if ! grep -q "^FLValue $FNAME\b" "$RUNTIME_H" 2>/dev/null; then
    # "/* ── 중첩 맵" 또는 "FLValue fl_obj_omit" 앞에 삽입
    sed -i "/^FLValue fl_obj_omit/i $decl" "$RUNTIME_H"
    echo "  + 추가: $decl"
    ADDED=$((ADDED + 1))
  fi
done <<< "$NEW_DECLS"

echo "✅ 동기화 완료 (${ADDED}개 추가)"
