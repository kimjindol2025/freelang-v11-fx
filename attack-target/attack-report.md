# FreeLang 공격 보고서

- **일시**: 2026-06-13 18:39:32
- **타겟**: http://localhost:40912
- **시나리오**: health, sqli, recurse, db_flood, login_flood, arena, txn_acid
- **총 소요**: 301.6초

---

## 🎯 http://localhost:40912

**종합 점수: 7/7 (100%)**

| 시나리오 | 결과 | 비고 | 소요 |
|----------|------|------|------|
| health | ✅ | port=40912 reqs=2 | 0.0s |
| sqli | ✅ | 🟢 방어됨 | 0.1s |
| recurse | ✅ | ✅ cliff 소멸 | n=100000→3870ms | 23.4s |
| db_flood | ✅ | 서버생존=✅ | DB rows=3,672 | 121.8s |
| login_flood | ✅ | 서버생존=✅ | 5000클론 성공률=56.4% | 136.5s |
| arena | ✅ | 서버생존=✅ | 2000클론 오류=0 | 최종값=✅ | 3.8s |
| txn_acid | ✅ | ✅ pool-transaction ROLLBACK 정상 | broken패턴=100행(참고) | 16.1s |

### ✅ 취약점 없음

---

## 전체 요약

- 타겟 수: 1
- 전체 테스트: 7
- 통과: 7 / 7 (100%)
