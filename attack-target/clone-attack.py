#!/usr/bin/env python3
"""
FreeLang 1억 클론 공격 — 동시성 압력 하 버그 재현
타겟: 192.168.45.73:40912

시나리오 A: BUG-2 — 동시 재귀 (O(n²) eval 폭발)
시나리오 B: BUG-3 — 동시 트랜잭션 (connection-per-query 유실 규모)
시나리오 C: Arena Lifetime — 동시 글로벌 쓰기/읽기 (crash/corruption)
"""
import sys
import time
import json
import threading
import urllib.request
import urllib.error
import queue
from collections import Counter

TARGET = "http://192.168.45.73:40912"

# ─── HTTP 헬퍼 ────────────────────────────────────────────────────

def get(path, timeout=10):
    try:
        with urllib.request.urlopen(f"{TARGET}{path}", timeout=timeout) as r:
            return json.loads(r.read()), r.status
    except urllib.error.HTTPError as e:
        return {"error": str(e)}, e.code
    except Exception as e:
        return {"error": str(e)}, -1

def post(path, data, timeout=10):
    body = json.dumps(data).encode()
    req = urllib.request.Request(
        f"{TARGET}{path}", data=body,
        headers={"Content-Type": "application/json"}, method="POST"
    )
    try:
        with urllib.request.urlopen(req, timeout=timeout) as r:
            return json.loads(r.read()), r.status
    except urllib.error.HTTPError as e:
        return {"error": str(e)}, e.code
    except Exception as e:
        return {"error": str(e)}, -1

# ─── 헬스 체크 ───────────────────────────────────────────────────

def check_target():
    r, code = get("/health")
    if code == 200:
        print(f"✅ 타겟 온라인: {TARGET}")
        return True
    print(f"❌ 타겟 오프라인: {code} {r}")
    return False

# ─── 시나리오 A: BUG-2 재귀 폭발 ────────────────────────────────

def scenario_a(clones=100, n=150, timeout=15):
    print(f"\n{'='*60}")
    print(f"🔴 시나리오 A: {clones} 클론 × 재귀 n={n} 동시 발사")
    print(f"   기대: O(n²) eval → 서버 응답 시간 폭발 or 전체 hang")
    print(f"{'='*60}")

    results = queue.Queue()

    def worker(i):
        t0 = time.time()
        r, code = get(f"/recurse/{n}", timeout=timeout)
        elapsed = time.time() - t0
        results.put({"i": i, "code": code, "elapsed": elapsed, "result": r.get("result")})

    threads = [threading.Thread(target=worker, args=(i,)) for i in range(clones)]
    t_start = time.time()
    for t in threads: t.start()
    for t in threads: t.join(timeout=timeout + 2)
    total_elapsed = time.time() - t_start

    rows = []
    while not results.empty():
        rows.append(results.get())

    success   = [r for r in rows if r["code"] == 200]
    timeout_r = [r for r in rows if r["code"] == -1 and "timed out" in str(r.get("result", ""))]
    errors    = [r for r in rows if r["code"] != 200]
    no_reply  = clones - len(rows)

    if success:
        times = [r["elapsed"] for r in success]
        print(f"  ✅ 성공:    {len(success):4d}개")
        print(f"  ⏱  최소:    {min(times):.3f}s")
        print(f"  ⏱  최대:    {max(times):.3f}s")
        print(f"  ⏱  평균:    {sum(times)/len(times):.3f}s")
    print(f"  💥 에러:    {len(errors):4d}개")
    print(f"  ⚠️  무응답:  {no_reply:4d}개")
    print(f"  🕐 총 경과: {total_elapsed:.2f}s")

    if max((r["elapsed"] for r in success), default=0) > timeout * 0.8:
        print(f"\n  🔥 BUG-2 확인: 재귀 n={n}으로 응답시간 폭발 (>{timeout*0.8:.0f}s)")
    elif len(errors) + no_reply > clones * 0.3:
        print(f"\n  🔥 BUG-2 확인: {(len(errors)+no_reply)/clones*100:.0f}% 실패 — 서버 과부하")
    else:
        print(f"\n  📊 n={n}: 버그 미폭발 — n 키워서 재시도 필요")

    return success, errors

# ─── 시나리오 B: BUG-3 트랜잭션 유실 ────────────────────────────

def scenario_b(clones=200):
    print(f"\n{'='*60}")
    print(f"🔴 시나리오 B: {clones} 클론 × /txn-broken (connection-per-query)")
    print(f"   기대: ROLLBACK noop → rows 누적 (데이터 유실/오염)")
    print(f"{'='*60}")

    # DB 초기화
    post("/db-reset", {})
    time.sleep(0.5)

    results_broken = queue.Queue()
    results_correct = queue.Queue()

    def worker_broken(i):
        r, code = post("/txn-broken", {"val": f"broken-{i}"})
        results_broken.put({"i": i, "code": code, "rows": r.get("total_rows", -1)})

    def worker_correct(i):
        r, code = post("/txn-correct", {"val": f"correct-{i}"})
        results_correct.put({"i": i, "code": code, "rows": r.get("total_rows", -1)})

    # broken 방식 발사
    threads = [threading.Thread(target=worker_broken, args=(i,)) for i in range(clones)]
    for t in threads: t.start()
    for t in threads: t.join(timeout=30)

    broken_rows = []
    while not results_broken.empty():
        broken_rows.append(results_broken.get())

    final_count_broken, _ = get("/db-count")
    rows_in_db = final_count_broken.get("rows", "?")

    print(f"\n  [broken 방식 — ROLLBACK 후 DB]")
    print(f"  요청 수:        {clones}")
    print(f"  DB 실제 rows:   {rows_in_db}  ← 0이어야 정상, > 0이면 ACID 붕괴")

    if isinstance(rows_in_db, int) and rows_in_db > 0:
        print(f"  🔥 BUG-3 확인: {rows_in_db}행 유출! ({rows_in_db/clones*100:.1f}% 유실)")
    else:
        print(f"  📊 broken 방식: rows=0 (ROLLBACK이 우연히 작동?)")

    # correct 방식 비교
    post("/db-reset", {})
    time.sleep(0.3)

    threads2 = [threading.Thread(target=worker_correct, args=(i,)) for i in range(min(clones, 50))]
    for t in threads2: t.start()
    for t in threads2: t.join(timeout=30)

    final_count_correct, _ = get("/db-count")
    rows_correct = final_count_correct.get("rows", "?")

    print(f"\n  [correct 방식 — pool-transaction]")
    print(f"  요청 수:        {min(clones, 50)}")
    print(f"  DB 실제 rows:   {rows_correct}  ← 0이어야 정상")
    if isinstance(rows_correct, int) and rows_correct == 0:
        print(f"  ✅ pool-transaction: ROLLBACK 정상")
    else:
        print(f"  ❌ pool-transaction도 유출: {rows_correct}행")

    post("/db-reset", {})

# ─── 시나리오 C: Arena Lifetime ───────────────────────────────────

def scenario_c(clones=1000):
    print(f"\n{'='*60}")
    print(f"🔴 시나리오 C: {clones} 클론 × 글로벌 write/read 경쟁")
    print(f"   기대: Arena use-after-free → crash or 값 손상")
    print(f"{'='*60}")

    errors = Counter()
    writes = 0
    reads  = 0
    corrupted = 0

    results = queue.Queue()

    def writer(i):
        r, code = get("/arena-write", timeout=5)
        results.put(("w", code, r.get("written", "")))

    def reader(i):
        r, code = get("/arena-read", timeout=5)
        results.put(("r", code, r.get("global", "")))

    # write/read 혼합 발사
    threads = []
    for i in range(clones):
        if i % 3 == 0:
            threads.append(threading.Thread(target=reader, args=(i,)))
        else:
            threads.append(threading.Thread(target=writer, args=(i,)))

    t0 = time.time()
    for t in threads: t.start()
    for t in threads: t.join(timeout=20)
    elapsed = time.time() - t0

    while not results.empty():
        kind, code, val = results.get()
        if code != 200:
            errors[code] += 1
        elif kind == "w":
            writes += 1
        elif kind == "r":
            reads += 1
            if val == "init" or val == "":
                corrupted += 1

    # 헬스 체크 — 서버 살아있나?
    health, hcode = get("/health")
    server_alive = hcode == 200

    print(f"  write 성공:    {writes}")
    print(f"  read 성공:     {reads}")
    print(f"  에러:          {dict(errors)}")
    print(f"  손상 값 수:    {corrupted}  (read했는데 'init' 반환)")
    print(f"  경과:          {elapsed:.2f}s")
    print(f"  서버 생존:     {'✅' if server_alive else '💥 CRASH!'}")

    if not server_alive:
        print(f"\n  🔥 Arena crash 확인!")
    elif corrupted > reads * 0.1:
        print(f"\n  ⚠️  Arena corruption 의심: {corrupted}/{reads} 손상")
    else:
        print(f"\n  📊 Arena: crash 없음 (단일 스레드 interpreter라 경쟁 없을 수 있음)")

# ─── 메인 ────────────────────────────────────────────────────────

def main():
    print("╔══════════════════════════════════════════════════╗")
    print("║   FreeLang 1억 클론 공격 — 동시성 버그 재현     ║")
    print(f"║   타겟: {TARGET:<38} ║")
    print("╚══════════════════════════════════════════════════╝")

    if not check_target():
        sys.exit(1)

    mode = sys.argv[1] if len(sys.argv) > 1 else "all"

    if mode in ("a", "all"):
        # n=100부터 시작해서 터지는 지점 찾기
        for n in [100, 120, 150, 180]:
            _, errs = scenario_a(clones=50, n=n, timeout=12)
            if len(errs) > 10:
                print(f"  → n={n}에서 50클론 중 {len(errs)}개 실패 — 임계점 발견")
                break
            time.sleep(1)

    if mode in ("b", "all"):
        scenario_b(clones=300)

    if mode in ("c", "all"):
        scenario_c(clones=500)

    print(f"\n{'='*60}")
    print("공격 완료")

if __name__ == "__main__":
    main()
