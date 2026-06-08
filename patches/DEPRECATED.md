# ⚠️ 711-regression-port.patch 폐기 (2026-06-08)

`711-regression-port.patch`(base `b76ddf23`)는 **폐기**됨. 보관은 역사 기록용.

**이유**: 원격 `freelang-v11` master 가 `b76ddf23` → **`a08ae05`** 로 진전하며, 이 patch 가
다루던 회귀(mod/inc/strlen/includes?/first-class 등)를 **원격이 자체·더 완전하게 수정**함:
- `KNOWN_ALIASES 40개` + evalBuiltin `case "mod"/"inc"/"strlen"` 등 추가
- `core 하위호환 복원`(mariadb/mongodb/ws/http-server), `server-start TLS`, 앱 호환성 검수 스크립트

**실측**: a08ae05 clone+build → scenario 15/15 정상, gateway demo **22/22 PASS**.

**그러므로**: 통합은 이 patch 적용이 아니라 **원격 a08ae05 기준**으로 진행한다.
patch 의 base(`b76ddf23`)도 이미 outdated. 재사용 금지.

상세: `../INTEGRATION-BASELINE.md` 0절.
