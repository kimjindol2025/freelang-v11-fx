# FreeLang Attack Target & Framework

FreeLang 인프라 보안/안정성 자동 검증 도구.

## 빠른 시작

```bash
# 1. 타겟 서버 시작 (rate limit 없음)
pm2 start /home/kimjin/freelang-v11/bootstrap.js \
  --name fl-stress-target --interpreter node \
  -- run /tmp/attack-target-nolimit.fl
# /tmp 없으면: cp attack-target-nolimit.fl /tmp/ 후 실행

# 2. 공격 실행
python3 fl-attack.py --target http://localhost:40913

# 3. 보고서 확인
cat attack-report.md
```

## 파일 구조

| 파일 | 역할 |
|------|------|
| `fl-attack.py` | 메인 공격 프레임워크 — 7개 시나리오 + 자동 보고서 |
| `target.fl` | 공격 타겟 서버 (포트 40912, rate limit 있음) |
| `clone-attack.py` | 구버전 공격 스크립트 (참고용) |
| `attack-report.md` | 마지막 실행 보고서 (자동 생성) |

## 시나리오 7개

```
health      — 서버 온라인 확인
sqli        — SQL 인젝션 5종 페이로드
recurse     — BUG-2 재귀 cliff 내성 (n=100~100000)
db_flood    — DB INSERT 100~3000 동시
login_flood — 로그인 100~5000 동시
arena       — 글로벌 변수 동시 경쟁
txn_acid    — ROLLBACK 후 DB 오염 검사
```

## 사용법

```bash
# 단일 타겟
python3 fl-attack.py --target http://localhost:40913

# 원격 타겟 (232 서버)
python3 fl-attack.py --target http://192.168.45.73:40913

# 다중 타겟 동시 (targets.txt: 줄당 URL 1개)
python3 fl-attack.py --targets targets.txt

# 특정 시나리오만
python3 fl-attack.py --scenario sqli,recurse,db_flood

# 보고서 파일명 지정
python3 fl-attack.py --report my-report.md
```

## 타겟 서버 2종

### target.fl (포트 40912) — rate limit 있음
```bash
pm2 start /home/kimjin/freelang-v11/bootstrap.js \
  --name fl-attack-target --interpreter node \
  -- run /home/kimjin/freelang-v11-fx/attack-target/target.fl
```

### attack-target-nolimit.fl (포트 40913) — 스트레스 전용
```bash
cat > /tmp/fl-nolimit.fl << 'FLEOF'
# target.fl 내용 + (server_rate_limit 999999 1000) 추가
FLEOF
pm2 start /home/kimjin/freelang-v11/bootstrap.js \
  --name fl-stress-target --interpreter node \
  -- run /tmp/fl-nolimit.fl
```

## PM2 현황

| PM2명 | 포트 | 비고 |
|-------|------|------|
| fl-attack-target | 40912 | rate limit 100req/60s |
| fl-stress-target | 40913 | rate limit 없음 |

## 알려진 이슈

- `arena` 시나리오: Node.js 싱글스레드라 실제 경쟁 없음 → 손상 수치 참고만
- `login_flood` 5000클론: 40% 성공 = 서버 죽음이 아닌 이벤트루프 큐잉
- `sqli` 취약 표시: target.fl `/login`이 의도적으로 str 연결 — 취약 검출 정상

## AIRC

```bash
node /home/kimjin/freelang-v11/bootstrap.js run \
  /home/kimjin/kim/Desktop/kim/02_Infrastructure/airc-spec/airc.fl status
```

hot 노드: `handoff-attack-framework-2026-06-13`
