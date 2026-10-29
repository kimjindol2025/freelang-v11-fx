# C13 인증 연결: 원문 조사와 다음 구현 범위

v11 기준은 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4`의 `src/stdlib-auth.ts`다. 이 문서는 TS 소스를 Git 텍스트로 읽은 기록이며 Node를 실행하지 않았다. C12의 auth_sha256은 이미 별도 연결됐다.

## 원문으로 확인한 계약

- auth_bearer_extract는 req.headers.authorization이 nullish이면 Authorization을 사용한다. 정확히 `Bearer ` 접두사만 허용하며 그 뒤를 그대로 돌려준다. lowercase bearer나 Basic은 nil이다. 빈 Bearer 뒤는 빈 문자열이다.
- 신규 hash는 scrypt N=16384,r=8,p=1, salt 16바이트·key 64바이트이며 `$scrypt$N=16384,r=8,p=1$salt_b64$hash_b64`다. OpenSSL 기존 native 의존성을 이용하고 새 외부 라이브러리는 추가하지 않는다.
- verify는 scrypt 표기와 기존 salt:sha256_hex 표기를 읽는다. source는 비용 파라미터를 저장된 문자열에서 읽고 scryptSync와 timingSafeEqual을 사용한다. 오답·손상·잘못된 파라미터 검사는 false다. native resource 한도와 비정상 빈 key/salt 등의 정확한 source 라이브러리 의미는 별도 조사·기록해야 하며 정상 fixture round-trip만으로 전 범위 호환을 주장하지 않는다.
- needs_rehash는 `$scrypt$` 접두사가 없을 때 true이며 나머지 포맷을 검사하는 함수가 아니다.
- random_token의 기본값은 32바이트다. CSPRNG 결과를 소문자 hex로 출력한다. 길이 오류와 실제 CSPRNG 실패는 숨기지 않는다.
- CSRF token은 epoch seconds 문자열+점+HMAC-SHA256 앞 16자리다. verify는 구분 조각 2개, parseInt(ts,10) 기준 age 0~3600초, 같은 서명을 timingSafeEqual로 확인한다.

## 다음 작업

독립된 source/RFC 또는 native 도구 기반 합성 fixture와 실패 `.fl`·기대 출력을 먼저 고정한다. 그 뒤 native 바인딩과 자체 시험을 구현한다. 새 TS/JS/Python·fl-build.sh·Node 실행, 운영 계정·비밀 값 사용은 금지한다. 이 문서는 아직 C13 구현·시험 통과 보고가 아니다. JWT·API key 등 앱에서 쓰지 않는 별도 함수로 범위를 넓혀 완료 기준을 바꾸지 않는다.
