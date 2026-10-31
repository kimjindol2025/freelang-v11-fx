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

## 인증 자체 시험 정답 선행 고정

정답은 v11 source와 [RFC 7914 §12](https://www.rfc-editor.org/rfc/rfc7914.html#section-12)의 공개 벡터에서 고정한다. RFC의 빈 password/salt이되 검증 key는 64바이트인 벡터, N=1024/r=8/p=16, N=16384/r=8/p=1을 사용한다. 생성 hash는 별도 C validator가 고정 파라미터로 다시 유도해 비교한다. 실제 운영 비밀번호·키·hash는 사용하지 않는다. 임시 CSRF key는 CSPRNG로 생성해 시험 소유 파일에만 보관하고 원문·strace 인자·보고에 출력하지 않는다.

참조 [Node v20 scrypt source](https://github.com/nodejs/node/blob/v20.18.0/lib/internal/crypto/scrypt.js)에서 default maxmem=32 MiB, 0인 N/r/p가 default로 치환됨을 읽었다. [random source](https://github.com/nodejs/node/blob/v20.18.0/lib/internal/crypto/random.js)는 숫자 범위 확인 후 uint32 변환하므로 양의 소수 byte 수를 버림한다. native random_token은 이를 재현하며 별도 resource 상한을 문서화한다. Node를 실행하지 않았으며 이 source 참조가 실제 과거 v11 실행기의 Node 버전 확인이라는 뜻은 아니다.

CSRF는 시험용 clock interpose로 boundary 3600초, expired 3601초, future 1초, parseInt prefix·space를 검증하고 별도로 실제 clock에서도 시험한다. CSPRNG 실패도 별도 interpose로 주입한다. Bearer nullish 우선순위·대소문자, random byte 기본값/소수/범위, rehash 접두사 정책, 생성 salt/key 크기를 `.fl`에서 검사한다.

## 사용자 결정 대기: 빈 검증 key

v11 verify는 decoded expected.length를 그대로 scryptSync keylen에 넘기며 참조 Node C++ source는 0을 허용한다. 따라서 빈 검증 key와 일반적인 손상 거부 요구가 충돌한다. 사용자에게 빈 key 거부를 호환 예외로 둘지 원문 동작을 보존할지 질문했다. 답변 전에는 이 분기를 임의로 확정하거나 auth_verify_password 전체 완료를 보고하지 않는다. 다른 인증 API와 정상 데이터 fixture는 계속 구현·검증한다. RFC의 빈 salt와 비어 있지 않은 key는 이 미결정 항목과 다르다.

## 현재 구현 경계

6개 API를 구현·부분 자체 시험했다. random_token은 source의 소수 버림을 보존하되 native byte 상한 1 MiB를 둔다. auth_verify_password 공개 연결과 password fixture는 사용자 결정 전 미구현·미실행이다. 이 항목이 남은 상태에서 C13 전체 완료나 blog1 로그인 성공을 주장하지 않는다.

## 사용자 확정: 손상 hash 거부

2026-10-31 사용자가 D01에 ‘손상 값으로 거부하고 정상 계정만 인증한다’를 선택했다. 빈 scrypt 검증 key는 false로 거부한다. 정상 scrypt·legacy 검증을 구현하고, 손상 값·잘못된 비용·자원 상한 초과는 인증 실패로 처리한다. 이 명시적 결정은 과거 빈 key 허용 동작에 대한 승인된 호환 예외다. 실제 계정·비밀번호를 시험에 사용하지 않는다.

검증 재개 정답은 기존 RFC/legacy fixture 12건을 유지하고 빈 key·형식/비용 오류·legacy 추가 조각의 14건과 비문자 저장값 5건, Unicode/NUL roundtrip·오답·잘못된 password 형식 4건을 합한 35건이다. 정답을 구현 전에 고정한다. v11 source의 legacy 첫 두 조각 사용과 Number 기반 비용·0 default를 보존한다. scrypt 검증 key가 0바이트인 경우만 사용자 결정으로 명시 거부한다. 외부 언어 실행이나 실제 비밀번호/hash는 사용하지 않는다.

Native 검증은 기존 OpenSSL과 32 MiB scrypt 메모리 예산을 사용한다. 입력 wire는 1 MiB, N×r×p 연산 비용은 16777216 이하로 제한하고 상한 초과는 false다. 정상 생성 포맷과 RFC 벡터를 지원하며 임의로 큰 v11 비용 전체를 지원한다고 주장하지 않는다. base64의 URL alphabet·생략한 padding·무시되는 문자와 legacy hex의 첫 유효 바이트 규칙은 기존 Buffer 계약대로 처리하되 decoded key 0바이트는 명시 거부한다. key 비교는 CRYPTO_memcmp이고 password/key 임시 버퍼는 정리한다.
