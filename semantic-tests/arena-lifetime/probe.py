#!/usr/bin/env python3
"""Arena Lifetime 버그 탐지 — use-after-free 관찰"""
import subprocess, time, urllib.request, json, sys, os, signal

BOOTSTRAP = "/home/kimjin/freelang-v11/bootstrap.js"
FL = os.path.join(os.path.dirname(__file__), "arena_test.fl")
PORT = 40999

def http_get(path):
    try:
        with urllib.request.urlopen(f"http://localhost:{PORT}{path}", timeout=3) as r:
            return json.loads(r.read())
    except Exception as e:
        return {"error": str(e)}

print("🧪 Arena Lifetime Bug 탐지")
print("=" * 50)

# 서버 시작
proc = subprocess.Popen(["node", BOOTSTRAP, "run", FL],
    stdout=subprocess.PIPE, stderr=subprocess.PIPE)
time.sleep(1.5)

if proc.poll() is not None:
    print("❌ 서버 시작 실패")
    sys.exit(1)

print("✅ 서버 시작 완료")

try:
    fail = 0

    # 패턴 1: SET → 다른 요청 여러 개 → GET (Arena reset 유도)
    print("\n[패턴 1] set → 50회 stress → get")
    http_get("/set")
    for _ in range(50):
        http_get("/stress")  # 다른 Arena 할당으로 이전 것 덮어쓰기 유도
    result = http_get("/get")
    print(f"  결과: {result}")
    if "error" in result or result.get("value") == "null" or result.get("value") is None:
        print("  💥 FAIL: Arena dangling 의심!")
        fail += 1
    else:
        print("  ✅ PASS: 값 유지됨")

    # 패턴 2: stress → check (리스트가 살아있는지)
    print("\n[패턴 2] stress → 50회 set → check")
    http_get("/stress")
    for i in range(50):
        http_get("/set")
    result = http_get("/check")
    print(f"  결과: {result}")
    if result.get("type") == "list" and result.get("len") == 10:
        print("  ✅ PASS: 리스트 무결 (len=10)")
    else:
        print(f"  💥 FAIL: 리스트 손상! {result}")
        fail += 1

    # 패턴 3: 연속 1000회 set/get 정합성
    print("\n[패턴 3] 1000회 set/get 정합성")
    mismatch = 0
    for i in range(1000):
        r_set = http_get("/set")
        r_get = http_get("/get")
        # count가 증가해야 함
        if r_get.get("count", 0) == 0 and i > 5:
            mismatch += 1
    if mismatch == 0:
        print(f"  ✅ PASS: 1000회 정합성 유지")
    else:
        print(f"  💥 FAIL: {mismatch}회 이상 이상 감지")
        fail += 1

    print()
    print("=" * 50)
    if fail == 0:
        print("🎉 Arena Lifetime: use-after-free 없음 ✅")
    else:
        print(f"💥 {fail}개 패턴에서 Arena 버그 감지!")

finally:
    proc.send_signal(signal.SIGTERM)
    proc.wait(timeout=3)

sys.exit(0 if fail == 0 else 1)
