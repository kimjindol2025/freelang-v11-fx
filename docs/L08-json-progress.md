# C12 JSON·checksum 진행 보고

## 확인한 것

v11 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4`의 JSON.parse·toSerializable/JSON.stringify·auth_sha256과 db95 WAL length/checksum 계약을 먼저 조사하고 `.fl`·정답을 커밋했다. 기존 parser가 `{`를 정상 값으로 받는 문제를 별도 최소 시험으로 확인했다. source의 JSON primitive·array·object 동작을 기준으로 native 공개 바인딩을 추가했다. seed에서 사용하는 legacy C 함수는 보존했다.

- JSON.parse 공개 API에서 잘못된 토큰·따옴표·제어 문자·escape·숫자 문법·trailing data를 거부한다. 중복 key, UTF-16 escape·서로게이트·NUL, -0·큰 정수의 Number 반올림을 검사했다.
- JSON 직렬화는 기존 shortest number/UTF-16/key 순서 helper를 사용하고 nonfinite를 null로 출력한다. v11 plain object의 toSerializable에서 __proto__가 own key로 복사되지 않는 특성을 적용했다.
- json_str/json_stringify/json_parse와 밑줄·하이픈 별칭, auth_sha256 UTF-8 연결을 추가했다. 기존 OpenSSL 기반 SHA-256을 사용하며 새 의존성은 없다.

```bash
CPATH=<가림> LIBRARY_PATH=<가림> L08_JSON_OUT=<가림> strace -f -e trace=execve -o <가림> bash scripts/verify-l08-json-self.sh
```

```text
JSON_SERIALIZE_BEFORE_BUILD_EXIT=1
JSON_INVALID_BEFORE_PROGRAM_EXIT=0
false
JSON_INVALID_BEFORE_CHECK_EXIT=1
L08_JSON_PASS=13 FAIL=0
FIXTURE_BYTES_UNCHANGED=PASS
JSON_SELF_EXIT=0
NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0
```

직렬화·parser·최소 잘못된 JSON 3개 묶음과 checksum reader 10회가 13묶음이다. 고정 body JSON은 시스템 sha256sum과 iconv UTF-16LE byte count/2로 checksum·length를 만들었으며, 언어 구현으로 정답을 생성하지 않았다. 숫자 0.1, 한글·이모지, 이미지 추가/삭제 이벤트가 포함된 합성 prepare/commit 레코드를 읽었다. checksum·length 검증이 실제 db95 재생인 것은 아니다.

[수정 전](evidence/l08-platform/json-before.txt), [수정 후](evidence/l08-platform/json-after.txt), [strace 감사](evidence/l08-platform/json-trace-audit.txt).

## 미확인 / 명시적 제약

이번 시험은 JSON 데이터형(nil·bool·number·string·array·map)과 실제 WAL checksum을 위한 언어 계약이다. 임의의 host Error·함수 객체 전체에 대한 v11 serializer 동작은 증명하지 않았으며, 함수 값은 명시적으로 거부한다. parser 입력 64 MiB·중첩 깊이 512를 native resource 한도로 두었다. 이 한도 밖의 입력까지 v11과 완전히 같다고 주장하지 않는다. 운영 복사본의 데이터가 이 한도와 데이터형 계약에 맞는지는 통합에서 확인해야 한다.

C11·C12 변경 후 최신 소스 전체 회귀·native bootstrap은 별도 드라이버에서 실행해야 한다. 이전 C18·컴파일러 보완의 전체 통과 결과를 최신 HEAD에 적용하지 않는다. 실제 db95 복사본 재생·잘린 WAL 끝·원본 데이터·URL·SHA 보존·HTTP·백업/복구는 아직 미확인이다.

전체 앱은 C 변환 후 fd·JSON·auth_sha256의 implicit declaration 오류가 없어졌다. 인증·HTTP·RSS/sitemap·타이머/TCP 연결이 남아 C 구문 검사는 여전히 종료 코드 1이다. 앱을 운영 데이터로 실행하지 않았으며 서버 23·25 접속·합치기·배포는 하지 않았다.

## fd 보조기 수정 이후 전체 검증 확정

검증 대상 커밋은 `45a31f4b6ea6071b8a5b9a7f0146191c1a1ab983`다. 새 전체 실행이 `CHECKED_PLATFORM_FULL_EXIT=0`으로 종료됐다. `L08_PLATFORM_PASS=5 FAIL=0`, fd basic `11`·extended `11`, JSON `13` 통과·0 실패다. extended는 오류 주입 8개·경계·base symlink·8-thread 800행 비교와 외부 sentinel 보존을 포함한다. 이전 L08 3차 `11`·2차 `11`·1차 `17`, HTTP `7`, L06 HTTP/TLS 각각 `110`, L07 자체 `221` 통과·0 실패 및 기존 회귀가 통과했다.

`BOOTSTRAP_EXIT=0`, `STAGE2_EQ_STAGE3=PASS`, STAGE2/STAGE3 SHA `66d1ff3f63de632194c7a858cdf8150ecff0528d711d5eeb39adb2da09ecc3b7`다. 전체 감사 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`. [전체 원문](evidence/l08-platform/fd-json-full.txt).

이 결과는 이후 추가하는 auth·HTTP 코드의 전체 통과나 실제 db95 재생·운영 데이터 호환의 완료 증거가 아니다.
