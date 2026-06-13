#!/usr/bin/env bash
# fl-ops — FreeLang 네이티브 앱 운영 대시보드
#
# 사용법:
#   fl-ops                   대시보드 (전체 앱 상태)
#   fl-ops status [앱명]     특정 앱 상세
#   fl-ops logs [앱명]       로그 tail
#   fl-ops top               실시간 리소스 모니터
#   fl-ops bench <URL>       간단한 부하 테스트
#   fl-ops restart <앱명>    reload (무중단)
#   fl-ops health            모든 앱 헬스체크

set -euo pipefail

PM2="$HOME/.npm-global/bin/pm2"
[[ -x "$PM2" ]] || PM2="$(which pm2 2>/dev/null || echo pm2)"

# ANSI
RESET='\033[0m'; BOLD='\033[1m'; GRAY='\033[90m'
GREEN='\033[32m'; YELLOW='\033[33m'; RED='\033[31m'
CYAN='\033[36m';  BLUE='\033[34m';  MAGENTA='\033[35m'

CMD="${1:-dashboard}"

# ── 네이티브 앱 감지: 메모리 < 20MB이고 버전 'N/A' ────────
detect_native_apps() {
    $PM2 jlist 2>/dev/null | python3 -c "
import sys, json
apps = json.load(sys.stdin)
native = []
for a in apps:
    ver = a.get('pm2_env', {}).get('version', '')
    mem = a.get('monit', {}).get('memory', 0)
    script = a.get('pm2_env', {}).get('pm_exec_path', '')
    # 네이티브: N/A 버전이거나 메모리 20MB 미만 + .fl 아님
    if (ver == 'N/A' or mem < 20*1024*1024) and not script.endswith('.fl') and not script.endswith('.js'):
        native.append(a)
print(json.dumps(native))
" 2>/dev/null
}

# ── 대시보드 ─────────────────────────────────────────
dashboard() {
    clear
    echo -e "${BOLD}${CYAN}┌─────────────────────────────────────────────────────────┐${RESET}"
    echo -e "${BOLD}${CYAN}│  🔧  FreeLang 네이티브 앱 운영 대시보드                   │${RESET}"
    echo -e "${BOLD}${CYAN}└─────────────────────────────────────────────────────────┘${RESET}"
    echo -e "${GRAY}$(date '+%Y-%m-%d %H:%M:%S')${RESET}"
    echo ""

    NATIVE_JSON=$(detect_native_apps)
    COUNT=$(echo "$NATIVE_JSON" | python3 -c "import sys,json; print(len(json.load(sys.stdin)))")

    echo -e "${BOLD}네이티브 앱 (${COUNT}개)${RESET}"
    echo -e "${GRAY}─────────────────────────────────────────────────────────${RESET}"
    printf "%-22s %-10s %6s %7s %6s %s\n" "앱명" "상태" "PID" "메모리" "CPU" "포트"
    echo -e "${GRAY}─────────────────────────────────────────────────────────${RESET}"

    echo "$NATIVE_JSON" | python3 -c "
import sys, json, subprocess, socket
apps = json.load(sys.stdin)
for a in apps:
    name   = a['name']
    status = a.get('pm2_env', {}).get('status', '?')
    pid    = a.get('pid', '-')
    mem    = a.get('monit', {}).get('memory', 0)
    cpu    = a.get('monit', {}).get('cpu', 0)
    mem_mb = mem // 1024 // 1024

    # 색
    sc = '\033[32m' if status == 'online' else '\033[31m'
    rs = '\033[0m'

    # 포트 추측 (cwd의 .projectrc.json)
    cwd = a.get('pm2_env', {}).get('pm_cwd', '')
    port = ''
    try:
        import os
        rc_path = os.path.join(cwd, '.projectrc.json')
        if os.path.exists(rc_path):
            d = json.load(open(rc_path))
            ports = d.get('ports', [])
            if ports: port = str(ports[0])
    except: pass

    print(f'{sc}{name:<22}{rs} {sc}{status:<10}{rs} {pid:>6} {mem_mb:>5}MB {cpu:>5}% {port}')
"
    echo ""

    # 시스템 요약
    echo -e "${BOLD}시스템${RESET}"
    echo -e "${GRAY}─────────────────────────────────────────────────────────${RESET}"
    # 네이티브 총 메모리
    TOTAL_MEM=$(echo "$NATIVE_JSON" | python3 -c "
import sys, json
apps = json.load(sys.stdin)
total = sum(a.get('monit', {}).get('memory', 0) for a in apps)
print(total // 1024 // 1024)
")
    NODE_MEM=$($PM2 jlist 2>/dev/null | python3 -c "
import sys, json
apps = json.load(sys.stdin)
node_apps = [a for a in apps if a.get('pm2_env',{}).get('version','') not in ('N/A','')]
total = sum(a.get('monit', {}).get('memory', 0) for a in node_apps)
print(total // 1024 // 1024)
" 2>/dev/null || echo "0")

    echo -e "  네이티브 앱 메모리: ${CYAN}${TOTAL_MEM}MB${RESET}"
    echo -e "  Node.js 앱 메모리:  ${YELLOW}${NODE_MEM}MB${RESET}"
    echo ""
    echo -e "${GRAY}  Ctrl+C로 종료 | fl-ops logs <앱명> | fl-ops health${RESET}"
}

# ── 헬스체크 ─────────────────────────────────────────
health_check() {
    TARGET="${2:-}"
    NATIVE_JSON=$(detect_native_apps)

    echo -e "${BOLD}헬스체크${RESET}"
    echo ""

    echo "$NATIVE_JSON" | python3 -c "
import sys, json, urllib.request, urllib.error
apps = json.load(sys.stdin)
import os

ok = 0; fail = 0
for a in apps:
    name = a['name']
    if '$TARGET' and name != '$TARGET': continue

    cwd = a.get('pm2_env', {}).get('pm_cwd', '')
    port = None
    try:
        rc_path = os.path.join(cwd, '.projectrc.json')
        if os.path.exists(rc_path):
            d = json.load(open(rc_path))
            ports = d.get('ports', [])
            if ports: port = ports[0]
    except: pass

    if not port:
        print(f'  ⚪ {name:<22} 포트 정보 없음')
        continue

    url = f'http://localhost:{port}/health'
    try:
        r = urllib.request.urlopen(url, timeout=2)
        status = r.status
        body = r.read(100).decode('utf-8', errors='replace')
        print(f'  ✅ {name:<22} {port} → {status} {body[:50]}')
        ok += 1
    except urllib.error.HTTPError as e:
        print(f'  ⚠️  {name:<22} {port} → HTTP {e.code}')
        fail += 1
    except Exception as e:
        print(f'  ❌ {name:<22} {port} → {type(e).__name__}')
        fail += 1

print()
print(f'  결과: {ok}개 정상 / {fail}개 실패')
"
}

# ── 실시간 top ────────────────────────────────────────
top_monitor() {
    echo -e "${BOLD}실시간 모니터 (1초 갱신, Ctrl+C 종료)${RESET}"
    echo ""
    while true; do
        NATIVE_JSON=$(detect_native_apps)
        tput cup 2 0 2>/dev/null || echo ""
        echo -e "${GRAY}$(date '+%H:%M:%S')${RESET} — 네이티브 앱"
        echo "────────────────────────────────────"
        printf "%-22s %7s %6s  %s\n" "앱명" "메모리" "CPU" "상태"
        echo "────────────────────────────────────"
        echo "$NATIVE_JSON" | python3 -c "
import sys, json
apps = json.load(sys.stdin)
for a in sorted(apps, key=lambda x: x.get('monit',{}).get('memory',0), reverse=True):
    n = a['name']
    m = a.get('monit',{}).get('memory',0) // 1024 // 1024
    c = a.get('monit',{}).get('cpu',0)
    s = a.get('pm2_env',{}).get('status','?')
    sc = '\033[32m' if s=='online' else '\033[31m'
    print(f'{sc}{n:<22}{chr(0o33)}[0m {m:>5}MB {c:>5}%  {s}')
"
        sleep 1
    done
}

# ── 부하 테스트 ───────────────────────────────────────
bench() {
    URL="${2:-}"
    if [[ -z "$URL" ]]; then
        echo "사용법: fl-ops bench <URL>"
        echo "예시:   fl-ops bench http://localhost:40920/health"
        exit 1
    fi

    N=100
    CONC=10
    echo -e "${BOLD}벤치마크: $URL${RESET}"
    echo "  요청 수: $N, 동시: $CONC"
    echo ""

    python3 - "$URL" "$N" "$CONC" << 'PYEOF'
import sys, time, threading, urllib.request

url, n, conc = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
results = []
lock = threading.Lock()

def worker(count):
    for _ in range(count):
        t0 = time.time()
        try:
            r = urllib.request.urlopen(url, timeout=5)
            r.read()
            ms = (time.time() - t0) * 1000
            with lock:
                results.append((r.status, ms))
        except Exception as e:
            with lock:
                results.append((0, 0))

per = n // conc
threads = [threading.Thread(target=worker, args=(per,)) for _ in range(conc)]
t_start = time.time()
for t in threads: t.start()
for t in threads: t.join()
elapsed = time.time() - t_start

ok = [r for r in results if r[0] == 200]
times = [r[1] for r in ok]
times.sort()

if times:
    avg = sum(times) / len(times)
    p50 = times[len(times)//2]
    p95 = times[int(len(times)*0.95)]
    p99 = times[int(len(times)*0.99)]
    rps = len(results) / elapsed
    print(f"  총 요청:  {len(results)}")
    print(f"  성공:     {len(ok)} ({len(ok)*100//len(results)}%)")
    print(f"  RPS:      {rps:.1f}")
    print(f"  평균:     {avg:.1f}ms")
    print(f"  p50:      {p50:.1f}ms")
    print(f"  p95:      {p95:.1f}ms")
    print(f"  p99:      {p99:.1f}ms")
PYEOF
}

# ── 로그 ─────────────────────────────────────────────
show_logs() {
    APP="${2:-}"
    if [[ -z "$APP" ]]; then
        echo "사용법: fl-ops logs <앱명>"
        exit 1
    fi
    $PM2 logs "$APP" --lines 50
}

# ── 재시작 ────────────────────────────────────────────
do_restart() {
    APP="${2:-}"
    if [[ -z "$APP" ]]; then
        echo "사용법: fl-ops restart <앱명>"
        exit 1
    fi
    echo "🔄 $APP reload (무중단)..."
    $PM2 reload "$APP"
    echo "✅ 완료"
}

# ── 디스패치 ──────────────────────────────────────────
case "$CMD" in
    dashboard|dash|"") dashboard ;;
    status)   health_check "$@" ;;
    health)   health_check "$@" ;;
    top)      top_monitor ;;
    bench)    bench "$@" ;;
    logs)     show_logs "$@" ;;
    restart)  do_restart "$@" ;;
    help|--help|-h)
        echo "fl-ops <명령> [인수]"
        echo ""
        echo "  (기본)         대시보드"
        echo "  health [앱명]  헬스체크"
        echo "  top            실시간 모니터"
        echo "  logs <앱명>    로그 tail"
        echo "  bench <URL>    부하 테스트"
        echo "  restart <앱명> 무중단 재시작"
        ;;
    *) echo "알 수 없는 명령: $CMD"; exit 1 ;;
esac
