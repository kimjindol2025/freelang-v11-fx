#!/bin/bash
# FreeLang Semantic Correctness 전체 테스트 스위트
set -e
cd "$(dirname "$0")"

PASS=0
FAIL=0

run_test() {
    local name="$1"
    local cmd="$2"
    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "🧪 $name"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    if eval "$cmd"; then
        echo "✅ $name PASS"
        PASS=$((PASS + 1))
    else
        echo "❌ $name FAIL"
        FAIL=$((FAIL + 1))
    fi
}

COUNT=${1:-100}  # 기본 100개, 1000개 원하면 ./run-all.sh 1000

echo "╔══════════════════════════════════════════════════╗"
echo "║   FreeLang Semantic Correctness Test Suite      ║"
echo "║   Interpreter vs Native — 의미론 정합성 검증    ║"
echo "╚══════════════════════════════════════════════════╝"
echo "테스트 수: $COUNT (./run-all.sh 1000 으로 더 많이)"

# 1. Compiler Differential
run_test "Compiler Differential (Interp vs Native)" \
    "python3 differential/generate.py $COUNT"

# 2. Throw/Catch Fuzz
run_test "Throw/Catch Fuzz (100단 중첩)" \
    "python3 throw-fuzz/fuzz.py 5"

# 3. SQL Transaction Rollback
run_test "SQL Transaction Rollback" \
    "node /home/kimjin/freelang-v11/bootstrap.js run sql-transaction/txn_test.fl"

# 4. Arena Lifetime
run_test "Arena Lifetime (use-after-free)" \
    "python3 arena-lifetime/probe.py"

echo ""
echo "╔══════════════════════════════════════════════════╗"
echo "║   최종 결과                                      ║"
printf "║   ✅ PASS: %-3d  ❌ FAIL: %-3d                    ║\n" $PASS $FAIL
echo "╚══════════════════════════════════════════════════╝"

[ $FAIL -eq 0 ]
