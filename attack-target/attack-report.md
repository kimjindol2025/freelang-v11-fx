# FreeLang 공격 보고서

- **일시**: 2026-06-13 18:47:23
- **타겟**: http://localhost:40912
- **시나리오**: health, sqli, recurse, db_flood, login_flood, arena, txn_acid
- **총 소요**: 273.7초

---

## 🎯 http://localhost:40912

**종합 점수: 7/7 (100%)**

| 시나리오 | 결과 | 비고 | 소요 |
|----------|------|------|------|
| health | ✅ | port=40912 reqs=0 | 0.0s |
| sqli | ✅ | 🟢 방어됨 | 0.1s |
| recurse | ✅ | ✅ cliff 소멸 | n=100000→3399ms | 22.0s |
| db_flood | ✅ | 서버생존=✅ | DB rows=3,241 | 119.6s |
| login_flood | ✅ | 서버생존=✅ | 5000클론 성공률=55.5% | 108.9s |
| arena | ✅ | 서버생존=✅ | 2000클론 오류=0 | 최종값=✅ | 3.8s |
| txn_acid | ✅ | ✅ pool-transaction ROLLBACK 정상 | broken패턴=100행(참고) | 19.2s |

### ✅ 취약점 없음

---

## 전체 요약

- 타겟 수: 1
- 전체 테스트: 7
- 통과: 7 / 7 (100%)
