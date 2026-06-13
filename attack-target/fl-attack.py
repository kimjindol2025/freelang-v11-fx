#!/usr/bin/env python3
"""
fl-attack.py — FreeLang 인프라 공격 프레임워크
악어처럼: 조용히 접근 → 한 번에 물어뜯기 → 보고서 제출

사용법:
  python3 fl-attack.py                          # 로컬 전체
  python3 fl-attack.py --target 192.168.1.100   # 원격 단일
  python3 fl-attack.py --targets targets.txt    # 다중 타겟
  python3 fl-attack.py --scenario sqli,flood    # 시나리오 선택
"""

import sys, time, json, threading, urllib.request, urllib.error
import queue, argparse, os, socket
from collections import Counter, defaultdict
from datetime import datetime

# ──────────────────────────────────────────────────────────────
# 시나리오 정의
# ──────────────────────────────────────────────────────────────
SCENARIOS = ["health", "sqli", "recurse", "db_flood", "login_flood", "arena", "txn_acid"]

# ──────────────────────────────────────────────────────────────
# HTTP 헬퍼
# ──────────────────────────────────────────────────────────────
def _get(base, path, timeout=30):
    try:
        with urllib.request.urlopen(f"{base}{path}", timeout=timeout) as r:
            return json.loads(r.read()), r.status
    except urllib.error.HTTPError as e:
        try: b = json.loads(e.read())
        except: b = {}
        return b, e.code
    except Exception as e:
        return {"error": str(e)[:80]}, -1

def _post(base, path, data, timeout=30):
    body = json.dumps(data).encode()
    req = urllib.request.Request(f"{base}{path}", data=body,
        headers={"Content-Type": "application/json"}, method="POST")
    try:
        with urllib.request.urlopen(req, timeout=timeout) as r:
            return json.loads(r.read()), r.status
    except urllib.error.HTTPError as e:
        try: b = json.loads(e.read())
        except: b = {}
        return b, e.code
    except Exception as e:
        return {"error": str(e)[:80]}, -1

def wave(fn, clones, twait=60):
    """clones개 스레드 동시 발사 → 결과 수집"""
    q = queue.Queue()
    threads = [threading.Thread(target=fn, args=(i, q), daemon=True) for i in range(clones)]
    t0 = time.time()
    for t in threads: t.start()
    for t in threads: t.join(timeout=twait)
    elapsed = time.time() - t0
    rows = []
    while not q.empty(): rows.append(q.get())
    return rows, elapsed

# ──────────────────────────────────────────────────────────────
# 개별 시나리오
# ──────────────────────────────────────────────────────────────

def sc_health(base):
    """헬스 체크 + 기본 정보 수집"""
    r, code = _get(base, "/health")
    return {
        "pass": code == 200,
        "detail": r,
        "note": f"port={r.get('port')} reqs={r.get('reqs',0)}" if code==200 else f"OFFLINE code={code}"
    }

def sc_sqli(base):
    """SQL 인젝션 — 로그인 우회 시도"""
    payloads = [
        ("' OR '1'='1",         "classic OR bypass"),
        ("'; DROP TABLE users;--", "drop injection"),
        ("admin'--",            "comment bypass"),
        ("' UNION SELECT 1,2,3--", "union injection"),
        ("x' OR '1'='1' LIMIT 1--", "limit bypass"),
    ]
    hits = []
    for pl, label in payloads:
        r, code = _post(base, "/login", {"username": pl, "password": ""}, timeout=10)
        injected = r.get("ok") == True
        hits.append({"payload": pl, "label": label, "injected": injected, "code": code})

    vuln = any(h["injected"] for h in hits)
    return {
        "pass": not vuln,  # 통과 = 인젝션 실패 = 방어됨
        "vuln": vuln,
        "hits": [h for h in hits if h["injected"]],
        "detail": hits,
        "note": f"{'🔴 취약 ' + str(sum(h['injected'] for h in hits)) + '개 뚫림' if vuln else '🟢 방어됨'}"
    }

def sc_recurse(base):
    """재귀 폭탄 — BUG-2 내성 검증"""
    results = {}
    for n in [100, 1000, 10000, 100000]:
        t0 = time.time()
        r, code = _get(base, f"/recurse/{n}", timeout=60)
        elapsed = time.time() - t0
        expected = n * (n + 1) // 2
        ok = code == 200 and r.get("result") == expected
        results[n] = {"ok": ok, "ms": int(elapsed*1000), "code": code}

    # 동시 50클론 × n=10000
    def worker(i, q):
        t0 = time.time()
        r, code = _get(base, "/recurse/10000", timeout=30)
        q.put({"code": code, "t": time.time()-t0})
    rows, elapsed = wave(worker, 50, twait=35)
    concurrent_ok = sum(1 for r in rows if r["code"] == 200)

    cliff_gone = results.get(100000, {}).get("ok", False)
    return {
        "pass": cliff_gone,
        "single": results,
        "concurrent_50": {"ok": concurrent_ok, "total": 50, "elapsed": round(elapsed,1)},
        "note": f"{'✅ cliff 소멸' if cliff_gone else '🔴 cliff 존재'} | n=100000→{results.get(100000,{}).get('ms','-')}ms"
    }

def sc_db_flood(base):
    """DB INSERT 대량 투입 — 처리량 + 안정성"""
    _post(base, "/db-reset", {})
    time.sleep(0.3)

    summary = {}
    for clones in [100, 500, 1000, 3000]:
        def worker(i, q, _c=clones):
            t0 = time.time()
            r, code = _post(base, "/db-insert", {"val": f"flood_{clones}_{i}"}, timeout=45)
            q.put({"code": code, "t": time.time()-t0})
        rows, elapsed = wave(worker, clones, twait=50)
        codes = Counter(r["code"] for r in rows)
        ok = codes[200]
        times = [r["t"] for r in rows if r["code"]==200]
        summary[clones] = {
            "ok": ok, "total": clones,
            "rate": round(ok/clones*100, 1),
            "avg_ms": round(sum(times)/len(times)*1000) if times else 0,
            "elapsed": round(elapsed, 1)
        }

    cnt, _ = _get(base, "/db-count")
    alive, hc = _get(base, "/health")
    return {
        "pass": hc == 200,
        "summary": summary,
        "final_rows": cnt.get("rows", 0),
        "server_alive": hc == 200,
        "note": f"서버생존={'✅' if hc==200 else '💥'} | DB rows={cnt.get('rows',0):,}"
    }

def sc_login_flood(base):
    """로그인 동시 폭격 — 인증 레이어 내성"""
    results = {}
    for clones in [100, 500, 2000, 5000]:
        def worker(i, q, _c=clones):
            t0 = time.time()
            r, code = _post(base, "/login",
                {"username": f"user{i%50}", "password": "pw"}, timeout=45)
            q.put({"code": code, "t": time.time()-t0, "ok_val": r.get("ok")})
        rows, elapsed = wave(worker, clones, twait=50)
        codes = Counter(r["code"] for r in rows)
        ok = codes[200]
        results[clones] = {"ok": ok, "total": clones,
                           "rate": round(ok/clones*100,1), "elapsed": round(elapsed,1)}

    alive, hc = _get(base, "/health")
    return {
        "pass": hc == 200,
        "results": results,
        "server_alive": hc == 200,
        "note": f"서버생존={'✅' if hc==200 else '💥'} | 5000클론 성공률={results.get(5000,{}).get('rate',0)}%"
    }

def sc_arena(base):
    """글로벌 상태 안정성 — 대량 동시 접근 후 서버 생존 + 값 유효성

    Node.js 싱글스레드: 실제 race condition 없음.
    검증 목적:
      1. 2000클론 동시 write/read → connection error 0 (crash 없음)
      2. 모든 write 완료 후 read → 값이 "init" 이 아닌 유효한 값
      3. 서버 생존
    """
    time.sleep(2)  # 이전 시나리오 잔류 요청 소멸 대기

    # 2000클론 동시 발사
    def worker(i, q):
        t0 = time.time()
        if i % 2 == 0:
            r, code = _get(base, "/arena-write", timeout=20)
        else:
            r, code = _get(base, "/arena-read", timeout=20)
        q.put({"code": code, "t": time.time()-t0})

    rows, elapsed = wave(worker, 2000, twait=25)
    ok   = sum(1 for r in rows if r["code"] == 200)
    errs = sum(1 for r in rows if r["code"] == -1)  # connection error = crash 징후

    # 최종 상태: write 후 read → "init" 아닌 유효값 확인
    _get(base, "/arena-write", timeout=5)
    final, fc = _get(base, "/arena-read", timeout=5)
    val_valid = fc == 200 and final.get("global", "init") != "init"

    alive, hc = _get(base, "/health")
    passed = hc == 200 and errs == 0 and val_valid

    return {
        "pass": passed,
        "ok": ok, "errs": errs,
        "final_val": final.get("global", "?"),
        "server_alive": hc == 200,
        "note": (
            f"서버생존={'✅' if hc==200 else '💥'} | "
            f"2000클론 오류={errs} | "
            f"최종값={'✅' if val_valid else '🔴init(손상)'}"
        )
    }

def sc_txn_acid(base):
    """트랜잭션 ACID — pool-transaction ROLLBACK 검증

    /txn-broken  = 의도적 버그 재현 (참고용, pass 기준 아님)
    /txn-correct = pool-transaction 올바른 방식 → ROLLBACK 후 0행 기대
    """
    _post(base, "/db-reset", {})
    time.sleep(0.3)

    # 올바른 방식 테스트 (pass 기준)
    def worker_correct(i, q):
        r, code = _post(base, "/txn-correct", {"val": f"acid_{i}"}, timeout=30)
        q.put({"code": code})

    wave(worker_correct, 100, twait=40)
    cnt, _ = _get(base, "/db-count")
    correct_rows = cnt.get("rows", 0)

    # 버그 재현 (informational only)
    _post(base, "/db-reset", {})
    time.sleep(0.2)

    def worker_broken(i, q):
        r, code = _post(base, "/txn-broken", {"val": f"broken_{i}"}, timeout=30)
        q.put({"code": code})

    wave(worker_broken, 100, twait=40)
    cnt2, _ = _get(base, "/db-count")
    broken_rows = cnt2.get("rows", 0)

    _post(base, "/db-reset", {})
    passed = correct_rows == 0  # pool-transaction ROLLBACK이 제대로 됐으면 0

    return {
        "pass": passed,
        "correct_rows": correct_rows,   # 0이어야 정상
        "broken_rows": broken_rows,     # 참고용 (버그 재현)
        "note": (
            f"{'✅ pool-transaction ROLLBACK 정상' if passed else '🔴 ROLLBACK 실패 ' + str(correct_rows) + '행'} | "
            f"broken패턴={broken_rows}행(참고)"
        )
    }

# ──────────────────────────────────────────────────────────────
# 타겟 공격 실행
# ──────────────────────────────────────────────────────────────

SCENARIO_MAP = {
    "health":     sc_health,
    "sqli":       sc_sqli,
    "recurse":    sc_recurse,
    "db_flood":   sc_db_flood,
    "login_flood":sc_login_flood,
    "arena":      sc_arena,
    "txn_acid":   sc_txn_acid,
}

def attack_target(base, scenarios):
    """단일 타겟 전체 시나리오 실행 → 결과 dict"""
    print(f"\n  🐊 [{base}] 공격 시작")
    results = {}
    for name in scenarios:
        fn = SCENARIO_MAP.get(name)
        if not fn: continue
        print(f"    ▶ {name:<15}", end="", flush=True)
        t0 = time.time()
        try:
            res = fn(base)
        except Exception as e:
            res = {"pass": False, "note": f"exception: {e}"}
        elapsed = time.time() - t0
        status = "✅" if res.get("pass") else "❌"
        print(f" {status}  {res.get('note','')}  ({elapsed:.1f}s)")
        results[name] = {**res, "elapsed": round(elapsed, 1)}
    return results

# ──────────────────────────────────────────────────────────────
# 보고서 생성
# ──────────────────────────────────────────────────────────────

def make_report(all_results, targets, scenarios, started_at, elapsed_total):
    ts = started_at.strftime("%Y-%m-%d %H:%M:%S")
    lines = [
        f"# FreeLang 공격 보고서",
        f"",
        f"- **일시**: {ts}",
        f"- **타겟**: {', '.join(targets)}",
        f"- **시나리오**: {', '.join(scenarios)}",
        f"- **총 소요**: {elapsed_total:.1f}초",
        f"",
        f"---",
        f"",
    ]

    for base, results in all_results.items():
        total = len(results)
        passed = sum(1 for r in results.values() if r.get("pass"))
        score = round(passed / total * 100) if total else 0

        lines += [
            f"## 🎯 {base}",
            f"",
            f"**종합 점수: {passed}/{total} ({score}%)**",
            f"",
            f"| 시나리오 | 결과 | 비고 | 소요 |",
            f"|----------|------|------|------|",
        ]
        for name, r in results.items():
            icon = "✅" if r.get("pass") else "❌"
            note = r.get("note", "")
            sec  = r.get("elapsed", 0)
            lines.append(f"| {name} | {icon} | {note} | {sec}s |")

        lines.append("")

        # 취약점 상세
        vulns = []
        if results.get("sqli", {}).get("vuln"):
            hits = results["sqli"].get("hits", [])
            vulns.append(f"- **SQL 인젝션** {len(hits)}개 페이로드 통과: `{hits[0]['payload'] if hits else ''}`")
        if results.get("txn_acid", {}).get("vuln"):
            n = results["txn_acid"].get("leaked_rows", 0)
            vulns.append(f"- **ACID 붕괴** ROLLBACK 후 {n}행 유출")
        if not results.get("recurse", {}).get("pass"):
            vulns.append("- **BUG-2 미수정** 재귀 cliff 존재")

        if vulns:
            lines += ["### ⚠️ 발견된 취약점", ""] + vulns + [""]
        else:
            lines += ["### ✅ 취약점 없음", ""]

    # 전체 요약
    all_pass = sum(r.get("pass",False) for res in all_results.values() for r in res.values())
    all_total = sum(len(res) for res in all_results.values())
    lines += [
        "---",
        "",
        f"## 전체 요약",
        f"",
        f"- 타겟 수: {len(targets)}",
        f"- 전체 테스트: {all_total}",
        f"- 통과: {all_pass} / {all_total} ({round(all_pass/all_total*100) if all_total else 0}%)",
        "",
    ]
    return "\n".join(lines)

# ──────────────────────────────────────────────────────────────
# 메인
# ──────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(description="fl-attack — FreeLang 공격 프레임워크")
    parser.add_argument("--target",   default="http://localhost:40913")
    parser.add_argument("--targets",  help="타겟 목록 파일 (줄당 1개 URL)")
    parser.add_argument("--scenario", default=",".join(SCENARIOS),
                        help=f"시나리오 선택 (기본: 전체) 선택지: {','.join(SCENARIOS)}")
    parser.add_argument("--report",   default="attack-report.md")
    args = parser.parse_args()

    scenarios = [s.strip() for s in args.scenario.split(",") if s.strip()]

    # 타겟 목록
    if args.targets and os.path.exists(args.targets):
        with open(args.targets) as f:
            targets = [l.strip() for l in f if l.strip() and not l.startswith("#")]
    else:
        targets = [args.target]

    print("╔══════════════════════════════════════════════════════════╗")
    print("║   🐊 fl-attack — FreeLang 인프라 공격 프레임워크       ║")
    print(f"║   타겟: {str(targets):<48} ║")
    print(f"║   시나리오: {','.join(scenarios):<44} ║")
    print("╚══════════════════════════════════════════════════════════╝")

    started_at = datetime.now()
    t_total = time.time()
    all_results = {}

    # 타겟별 공격 (병렬 가능하지만 순차로 먼저)
    for base in targets:
        all_results[base] = attack_target(base, scenarios)

    elapsed_total = time.time() - t_total

    # 보고서 생성
    report = make_report(all_results, targets, scenarios, started_at, elapsed_total)
    report_path = os.path.join(os.path.dirname(__file__), args.report)
    with open(report_path, "w") as f:
        f.write(report)

    print(f"\n{'='*60}")
    print(f"✅ 공격 완료 — {elapsed_total:.1f}초")
    print(f"📄 보고서: {report_path}")
    print(f"{'='*60}\n")
    print(report)

if __name__ == "__main__":
    main()
