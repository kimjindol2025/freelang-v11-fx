#!/usr/bin/env python3
"""
FreeLang Try/Catch Fuzz Tester
stack corruption / setjmp leak / longjmp bug 탐지
"""
import subprocess
import random
import os
import sys

BOOTSTRAP = "/home/kimjin/freelang-v11/bootstrap.js"
TMP = "/tmp/fl_throw_fuzz"
os.makedirs(TMP, exist_ok=True)

def gen_deep_nested_throw(depth, throw_at=None):
    """depth단 중첩 try/catch, throw_at 레벨에서 throw"""
    if throw_at is None:
        throw_at = random.randint(0, depth - 1)

    def build(level):
        if level == depth:
            if level == throw_at:
                return '(throw "deep-error")'
            return f'(println "leaf-{level}")'
        inner = build(level + 1)
        if level == throw_at:
            return f"""(try
  (throw "error-at-{level}")
  (catch $e (println (str "caught-at-{level}:" $e))))"""
        return f"""(try
  {inner}
  (catch $e (println (str "bubble-{level}:" $e))))"""

    return build(0)

def gen_throw_in_let():
    """let 바인딩 중 throw"""
    n = random.randint(2, 8)
    throw_at = random.randint(0, n - 1)
    parts = []
    for i in range(n):
        if i == throw_at:
            parts.append(f'$v{i} (throw "let-err")')
        else:
            parts.append(f'$v{i} {i}')
    bindings = " ".join(parts)
    return f"""(try
  (let [{bindings}]
    (println "unreachable"))
  (catch $e (println (str "caught:" $e))))"""

def gen_throw_rethrow(depth):
    """catch 안에서 다시 throw"""
    code = f'(throw "original")'
    for i in range(depth):
        code = f"""(try
  {code}
  (catch $e{i}
    (if (> {depth - i} 1)
      (throw (str "rethrow-{i}:" $e{i}))
      (println (str "final-catch:" $e{i})))))"""
    return code

def gen_throw_in_map():
    """map 중 throw"""
    throw_at = random.randint(0, 4)
    items = " ".join(str(i) for i in range(5))
    return f"""(try
  (map (fn [$x]
    (if (= $x {throw_at})
      (throw (str "map-throw-at-" $x))
      (* $x 2)))
    (list {items}))
  (catch $e (println (str "map-caught:" $e))))"""

def gen_finally_like():
    """catch 후 정상 코드 실행 (자원 해제 패턴)"""
    return f"""(define $state "init")
(try
  (begin
    (define $state "in-try")
    (throw "test-error"))
  (catch $e
    (begin
      (define $state "in-catch")
      (println (str "state=" $state)))))
(println (str "after-try=" $state))"""

CASES = [
    ("deep_nested_5",   lambda: gen_deep_nested_throw(5)),
    ("deep_nested_20",  lambda: gen_deep_nested_throw(20)),
    ("deep_nested_50",  lambda: gen_deep_nested_throw(50)),
    ("deep_nested_100", lambda: gen_deep_nested_throw(100)),
    ("throw_in_let",    gen_throw_in_let),
    ("rethrow_5",       lambda: gen_throw_rethrow(5)),
    ("rethrow_20",      lambda: gen_throw_rethrow(20)),
    ("throw_in_map",    gen_throw_in_map),
    ("finally_like",    gen_finally_like),
]

def run(fl_code, label, timeout=15):
    fl_file = f"{TMP}/{label}.fl"
    with open(fl_file, "w") as f:
        f.write(fl_code)
    try:
        r = subprocess.run(
            ["node", BOOTSTRAP, "run", fl_file],
            capture_output=True, text=True, timeout=timeout
        )
        return {
            "label": label,
            "rc": r.returncode,
            "stdout": r.stdout.strip(),
            "stderr": r.stderr.strip()[:200],
            "crashed": r.returncode not in (0, 1),
        }
    except subprocess.TimeoutExpired:
        return {"label": label, "rc": -999, "stdout": "", "stderr": "TIMEOUT", "crashed": True}
    except Exception as e:
        return {"label": label, "rc": -1, "stdout": "", "stderr": str(e), "crashed": True}

def run_fuzz(repeat=10):
    print(f"💣 Throw/Catch Fuzz Test (각 케이스 {repeat}회)")
    print("=" * 60)

    total_pass = total_fail = total_crash = 0

    for name, gen_fn in CASES:
        case_pass = case_fail = case_crash = 0
        for i in range(repeat):
            code = gen_fn()
            result = run(code, f"{name}_{i}")

            if result["crashed"]:
                case_crash += 1
                print(f"  💥 CRASH  [{name}_{i}] rc={result['rc']} — {result['stderr'][:80]}")
            elif result["stderr"] and "Error" in result["stderr"]:
                case_fail += 1
                print(f"  ❌ FAIL   [{name}_{i}] — {result['stderr'][:80]}")
            else:
                case_pass += 1

        status = "✅" if case_crash == 0 and case_fail == 0 else ("💥" if case_crash > 0 else "⚠️")
        print(f"  {status} {name:25s} PASS={case_pass} FAIL={case_fail} CRASH={case_crash}")
        total_pass += case_pass
        total_fail += case_fail
        total_crash += case_crash

    print()
    print("=" * 60)
    print(f"📊 결과: PASS={total_pass} FAIL={total_fail} CRASH={total_crash}")

    if total_crash == 0 and total_fail == 0:
        print("🎉 try/catch 100단 중첩: 스택 오염 없음 ✅")
    else:
        if total_crash > 0:
            print(f"💥 CRASH {total_crash}건 — setjmp/longjmp 버그 의심!")
        if total_fail > 0:
            print(f"⚠️  FAIL {total_fail}건 — catch 동작 불일치")

    return total_crash == 0 and total_fail == 0

if __name__ == "__main__":
    repeat = int(sys.argv[1]) if len(sys.argv) > 1 else 10
    ok = run_fuzz(repeat)
    sys.exit(0 if ok else 1)
