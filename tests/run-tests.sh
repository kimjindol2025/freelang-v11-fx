#!/usr/bin/env bash
# FreeLang C Native Runtime 테스트 러너
# 검증 역량: JIT Compilation + C Native Runtime
# 실행: bash tests/run-tests.sh

DIR="$(cd "$(dirname "$0")/.." && pwd)"
PASS=0
FAIL=0

run_test() {
  local name="$1"
  local bin="$2"
  local fl="$3"
  local expect="$4"

  output=$("$DIR/$bin" "$DIR/$fl" 2>&1)
  exit_code=$?

  if echo "$output" | grep -q "$expect" && [ $exit_code -eq 0 ]; then
    echo "✅ $name"
    PASS=$((PASS + 1))
  else
    echo "❌ $name (exit=$exit_code)"
    echo "$output" | tail -5 | sed 's/^/   /'
    FAIL=$((FAIL + 1))
  fi
}

echo "🔍 FreeLang C Native Runtime 테스트"
echo ""

# T1: JIT 기본 연산 (add1/mul2/addxy)
run_test "JIT add1(41)=42"   "jit-test-bin"   "jit-test.fl"       "add1(41) = 42"
run_test "JIT mul2(21)=42"   "jit-test-bin"   "jit-test.fl"       "mul2(21) = 42"
run_test "JIT addxy(10,32)"  "jit-test-bin"   "jit-test.fl"       "addxy(10,32) = 42"
run_test "JIT add1(0)=1"     "jit-test-bin"   "jit-test.fl"       "add1(0) = 1"

# T2: JIT 통계
run_test "JIT pool_cap=64KB" "jit-test-bin"   "jit-test.fl"       "풀 용량: 64KB"
run_test "JIT ready=true"    "jit-test-bin"   "jit-test.fl"       "JIT 준비: true"

# T3: GC 스트레스 (atom RC-Heap)
run_test "GC atom 초기화"    "gc-stress-bin"  "gc-stress-test.fl" "T1: atom 초기화"
run_test "GC 정수 누산 100회" "gc-stress-bin" "gc-stress-test.fl" "T2: 정수 누산 100회"
run_test "GC 벡터 atom 20회" "gc-stress-bin" "gc-stress-test.fl" "T6: 벡터 atom"
run_test "GC JIT 풀 준비"    "gc-stress-bin" "gc-stress-test.fl"  "T10: JIT 풀 준비"

echo ""
echo "────────────────────────────"
echo "결과: $PASS PASS / $FAIL FAIL / $((PASS + FAIL)) 총"

if [ $FAIL -eq 0 ]; then
  echo "🎉 모든 테스트 통과!"
  exit 0
else
  echo "❌ ${FAIL}개 실패"
  exit 1
fi
