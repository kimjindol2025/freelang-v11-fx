#!/usr/bin/env python3
"""
FreeLang Compiler Differential Test Generator
Interpreter(bootstrap.js) vs Native(cgc-bin) 결과 비교
"""
import random
import subprocess
import os
import sys
import json
import time

BOOTSTRAP = "/home/kimjin/freelang-v11/bootstrap.js"
FL_BUILD  = "/home/kimjin/freelang-v11-fx/fl-build.sh"
RUNTIME_DIR = "/home/kimjin/freelang-v11-fx/runtime"
TMP = "/tmp/fl_diff_test"

os.makedirs(TMP, exist_ok=True)

# ─── 프로그램 템플릿 ───────────────────────────────────────────────

def gen_factorial(n):
    return f"""(defn fact [$n]
  (if (<= $n 1)
    1
    (* $n (fact (- $n 1)))))
(println (fact {n}))"""

def gen_fibonacci(n):
    return f"""(defn fib [$n]
  (if (<= $n 1)
    $n
    (+ (fib (- $n 1)) (fib (- $n 2)))))
(println (fib {n}))"""

def gen_map_reduce(size):
    nums = " ".join(str(random.randint(1, 100)) for _ in range(size))
    return f"""(define $lst (list {nums}))
(define $doubled (map (fn [$x] (* $x 2)) $lst))
(define $sum (reduce (fn [$acc $x] (+ $acc $x)) 0 $doubled))
(println $sum)"""

def gen_string_ops(n):
    words = ["hello", "world", "freelang", "native", "test", "kim", "abc"]
    ops = []
    for _ in range(n):
        w = random.choice(words)
        ops.append(f'(str-length "{w}")')
    body = "\n".join(f"(println {op})" for op in ops)
    return body

def gen_let_chain(depth):
    inner = "(println $x0)"
    for i in range(depth - 1, 0, -1):
        inner = f"(let [$x{i} (+ $x{i-1} {i})]\n  {inner})"
    return f"(let [$x0 1]\n  {inner})"

def gen_closure(n):
    return f"""(defn make-adder [$x]
  (fn [$y] (+ $x $y)))
(define $add{n} (make-adder {n}))
(println ($add{n} {n}))"""

def gen_tail_recursion(n):
    return f"""(defn sum-iter [$n $acc]
  (if (<= $n 0)
    $acc
    (sum-iter (- $n 1) (+ $acc $n))))
(println (sum-iter {n} 0))"""

def gen_list_ops(size):
    nums = " ".join(str(random.randint(1, 50)) for _ in range(size))
    return f"""(define $lst (list {nums}))
(define $filtered (filter (fn [$x] (> $x 25)) $lst))
(println (length $filtered))"""

def gen_nested_if(depth, val):
    def build(d):
        if d == 0:
            return str(val)
        return f"(if (> {d} 0) {build(d-1)} -1)"
    return f"(println {build(depth)})"

def gen_higher_order():
    n = random.randint(1, 5)
    return f"""(defn compose [$f $g]
  (fn [$x] ($f ($g $x))))
(define $inc (fn [$x] (+ $x 1)))
(define $dbl (fn [$x] (* $x 2)))
(define $inc_then_dbl (compose $dbl $inc))
(println ($inc_then_dbl {n}))"""

GENERATORS = [
    lambda: gen_factorial(random.randint(1, 12)),
    lambda: gen_fibonacci(random.randint(0, 15)),
    lambda: gen_map_reduce(random.randint(3, 10)),
    lambda: gen_string_ops(random.randint(2, 8)),
    lambda: gen_let_chain(random.randint(2, 6)),
    lambda: gen_closure(random.randint(1, 20)),
    lambda: gen_tail_recursion(random.randint(10, 1000)),
    lambda: gen_list_ops(random.randint(5, 15)),
    lambda: gen_nested_if(random.randint(1, 5), random.randint(0, 10)),
    lambda: gen_higher_order(),
]

# ─── 실행 ─────────────────────────────────────────────────────────

def run_interpreter(fl_file, timeout=10):
    try:
        r = subprocess.run(
            ["node", BOOTSTRAP, "run", fl_file],
            capture_output=True, text=True, timeout=timeout
        )
        return r.stdout.strip(), r.returncode
    except subprocess.TimeoutExpired:
        return "TIMEOUT", -1
    except Exception as e:
        return f"ERROR:{e}", -1

def build_native(fl_file, out_bin, timeout=30):
    try:
        r = subprocess.run(
            ["bash", FL_BUILD, fl_file, out_bin],
            capture_output=True, text=True, timeout=timeout,
            cwd=os.path.dirname(fl_file)
        )
        return r.returncode == 0, r.stderr
    except subprocess.TimeoutExpired:
        return False, "BUILD TIMEOUT"
    except Exception as e:
        return False, str(e)

def run_native(bin_file, timeout=10):
    try:
        r = subprocess.run(
            [bin_file],
            capture_output=True, text=True, timeout=timeout
        )
        return r.stdout.strip(), r.returncode
    except subprocess.TimeoutExpired:
        return "TIMEOUT", -1
    except Exception as e:
        return f"ERROR:{e}", -1

# ─── 메인 ─────────────────────────────────────────────────────────

def run_differential(count=100, verbose=False):
    results = {"pass": 0, "fail": 0, "build_fail": 0, "errors": []}

    print(f"🔬 Differential Test 시작 ({count}개)")
    print("=" * 60)

    for i in range(count):
        gen = random.choice(GENERATORS)
        code = gen()

        fl_file = f"{TMP}/test_{i}.fl"
        bin_file = f"{TMP}/test_{i}_bin"

        with open(fl_file, "w") as f:
            f.write(code)

        # Interpreter 실행
        interp_out, interp_rc = run_interpreter(fl_file)

        # Native 빌드 + 실행
        build_ok, build_err = build_native(fl_file, bin_file)
        if not build_ok:
            results["build_fail"] += 1
            if verbose:
                print(f"  [{i:4d}] BUILD_FAIL — {build_err[:60]}")
            continue

        native_out, native_rc = run_native(bin_file)

        # 비교
        if interp_out == native_out:
            results["pass"] += 1
            if verbose:
                print(f"  [{i:4d}] ✅ PASS — {interp_out[:40]}")
        else:
            results["fail"] += 1
            entry = {
                "id": i,
                "code": code,
                "interpreter": interp_out,
                "native": native_out,
            }
            results["errors"].append(entry)
            print(f"  [{i:4d}] ❌ FAIL")
            print(f"         코드: {code[:60].replace(chr(10),' ')}")
            print(f"         인터프리터: {repr(interp_out)}")
            print(f"         네이티브:   {repr(native_out)}")

        # 진행 표시
        if (i + 1) % 50 == 0:
            print(f"  진행: {i+1}/{count} — PASS={results['pass']} FAIL={results['fail']} BUILD_FAIL={results['build_fail']}")

        # 임시 파일 정리
        try:
            os.unlink(fl_file)
            if os.path.exists(bin_file):
                os.unlink(bin_file)
        except:
            pass

    print()
    print("=" * 60)
    total = results["pass"] + results["fail"] + results["build_fail"]
    tested = results["pass"] + results["fail"]
    pass_rate = (results["pass"] / tested * 100) if tested > 0 else 0

    print(f"📊 결과 ({count}개)")
    print(f"  ✅ PASS:        {results['pass']:4d}")
    print(f"  ❌ FAIL:        {results['fail']:4d}  ← 의미론 불일치!")
    print(f"  🔨 BUILD_FAIL:  {results['build_fail']:4d}")
    print(f"  📈 정합성:      {pass_rate:.1f}%")
    print()

    if results["fail"] == 0:
        print("🎉 Interpreter == Native: 100% 정합성 확인")
    else:
        print(f"⚠️  {results['fail']}개 불일치 발견 — 아래 케이스 분석 필요:")
        for e in results["errors"][:5]:
            print(f"\n  케이스 #{e['id']}:")
            print(f"  {e['code'][:100]}")
            print(f"  인터프리터: {e['interpreter']}")
            print(f"  네이티브:   {e['native']}")

    # 결과 저장
    with open(f"{TMP}/diff_results.json", "w") as f:
        json.dump(results, f, ensure_ascii=False, indent=2)
    print(f"\n💾 상세 결과: {TMP}/diff_results.json")

    return results["fail"] == 0

if __name__ == "__main__":
    count = int(sys.argv[1]) if len(sys.argv) > 1 else 100
    verbose = "--verbose" in sys.argv or "-v" in sys.argv
    ok = run_differential(count, verbose)
    sys.exit(0 if ok else 1)
