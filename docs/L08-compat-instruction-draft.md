# L08 v11 → fx 호환성 구현 지시서 초안

최초 작성 단계는 문서 초안이었다. 이후 승인 범위는 6장에 기록한다. 이 문서의 조사 표·시험 계획 자체를 구현 완료·이전 완료·시험 통과 증거로 사용하지 않는다.

## 1. 기준과 조사 범위

| 대상 | 고정 커밋 | 조사 대상 |
|---|---|---|
| fx / 시작 가지 `feat/l06-multipart` | `7566eb1aa3c07de617e71482e0a051587d718725` | 네이티브 컴파일러, C 런타임, L07·L06 계약 |
| blog1 | `013a5b339068412332ae12a7ef0c412963a3a880` | 추적 중인 앱 `.fl` 22개와 시험 `.fl` 47개 |
| blog1의 db95 의존성 | `88b4aa2ca11e5355240a1d93ece8321315c669cc` | `tcp-server.fls`, `engine.fls`, `sql-core.fls`, `protocol.fls`, `auth.fls` |
| 비교용 v11 | `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4` | 기존 API의 소스 정의. 실행하지 않음 |

작업 가지는 `docs/l08-compat-plan`이며 fx 기준 커밋에서 직접 분기했다. blog1의 앱·시험·vendor는 `220a9ecb5220492111981973950c3c048385c8ed` 대비 차이가 없다. 실제 운영 실행기가 위 v11 커밋과 같은지는 **미확인**이다. 따라서 아래 v11 원문은 비교 기준 소스이며 운영에서 측정한 출력이 아니다.

[호출 전수 목록과 파일 목록](https://github.com/kimjindol2025/freelang-v11-fx/blob/docs/l08-compat-plan/docs/L08-call-inventory.md)에 74개 파일의 조사 결과를 남겼다. 문자열·주석을 제외한 호출 위치는 12751개, 서로 다른 호출 머리는 586개다. 그중 조사 파일 어디엔가 같은 이름의 선언이 있는 머리는 423개, 외부 호출 머리는 163개다. 고차 함수의 값 인자로만 등장하는 `update`를 더해 외부 점검 대상은 164개다. 이 수치는 문법 형식도 포함하며 “표준 함수 164개가 모두 없다”는 뜻이 아니다. 조사 전체의 선언 집합과 비교한 정적 목록이지 진입점별 이름 해석·호출 가능성 증명은 아니다.

앱의 `load`와 실제 db95 의존 파일까지 포함했다. `src`는 db95 소스로 이어지는 링크다. 시험 47개는 서로 독립된 진입점이며 함께 컴파일하는 것으로 가정하지 않는다. HTTP 경로에 문자열로 등록한 처리 함수, 사용자 함수의 재정의, 표준 함수와의 이름 충돌은 호출 목록만으로 판정할 수 없어 별도 항목 C01·C02로 다룬다. 브라우저 내장 JS 문자열은 FreeLang 호출 목록에서 제외한다. 기존 JS의 fx 브라우저 출력 이전은 L01이며 이번 L08에서 새 JS를 만들지 않는다.

## 2. 근거를 읽는 방법

아래 X·B·D·V 표기는 다음 **고정 커밋의 GitHub 소스**를 뜻한다. 절의 코드 인용은 해당 파일의 원문 일부이며, 예상 결과와 구분한다.

- X: [fx](https://github.com/kimjindol2025/freelang-v11-fx/tree/7566eb1aa3c07de617e71482e0a051587d718725)
- B: [blog1](https://github.com/kimjindol2025/blog1/tree/013a5b339068412332ae12a7ef0c412963a3a880)
- D: [db95](https://github.com/kimjindol2025/afj-db95/tree/88b4aa2ca11e5355240a1d93ece8321315c669cc)
- V: [v11](https://github.com/kimjindol2025/freelang-v11/tree/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4)

v11의 TS·bootstrap JS는 Git 객체를 텍스트로만 읽었다. 실행·수정·새 파일 작성은 하지 않았다. fx의 동일 이름 함수가 있다는 사실만으로 호환이라 판정하지 않았다. 명시적 컴파일러 분기, C 선언, C 본문, 인자 수·순서·반환·오류·문자열 단위를 함께 비교했다. 명시적 분기나 정확한 C 바인딩이 없는 경우 **공개 연결 없음**으로 표시하며, 실행으로 확인한 오류 메시지를 꾸며 쓰지 않는다.

## 3. 허용·금지와 이전 원칙

- 이후 구현 언어는 `.fl` 및 fx C 런타임·생성 C, 시험 구동은 Bash만 허용한다. 새 TS·JS·Python, `fl-build.sh` 사용은 금지한다. 이번 변경은 Markdown만 허용한다.
- 부족한 기능은 fx 컴파일러·표준 라이브러리에 추가한다. 앱에서 함수 이름·인자 순서·정규식·시간 단위를 바꾸거나 db95 함수를 삭제해 컴파일만 통과시키는 우회는 금지한다.
- L07의 바이너리 무손실·오류 계약, L06의 파일당 15 MiB·전체 본문 64 MiB·4개 상한을 약화시키지 않는다. 앱의 별도 정책 상한은 별개다.
- 운영 WAL·이미지·설정·계정에 접근하거나 변경하지 않는다. 이전 검증은 합성 fixture와 승인된 비운영 복사본만 사용한다. 기존 WAL 기록 삭제·해시 재작성·계정 초기화로 차이를 숨기지 않는다.
- 23·25 서버 접속·작업, 합치기·배포·강제 푸시 금지. loopback 시험은 구현 승인 후 빈 포트를 자동 할당하고 다른 과정을 종료하지 않는다.

## 4. 차이별 API·오류 계약과 `.fl` 재현 시험 계획

각 C 번호는 별도 언어 자체 시험 묶음이다. 아래 예제와 정답은 **시험 설계**이며 실행 로그가 아니다. 모호한 v11 평가 경로·오류 계약은 먼저 결정·고정해야 한다. 블로그 시험만 통과했다고 언어 기능 완료로 판정하지 않는다.

<a id="c01"></a>

### C01 — 이름 해석, 재정의, load

**현재 원문:** D `protocol.fls:6`은 `(defn server-start [name]`을 선언한다. B `main.fl`의 HTTP 호출은 `server_start`다. X `self/cgc-main.fl`의 `c-name`은 `-`를 `_`로 바꾸고, `server-start` 분기는 `fl_http_start(`를 생성한다. D `tcp-server.fls:615`와 B `store.fl:7`에는 같은 `afj-tcp-recover-lines` 재정의가 있다. X `cgc-defn`은 정의마다 C 함수를 출력한다. `expand-loads`는 원래 경로 문자열을 방문 집합에 넣고 파일이 없으면 `[load] 파일 없음:`을 출력한 뒤 계속한다.

**차이·기대:** db95 사용자 `server-start`와 HTTP `server_start`는 별개다. 내장 함수 분기가 사용자 선언을 가로채면 안 된다. 같은 이름의 재정의는 기존 앱이 의도한 복구 함수 교체를 보존해야 한다. 중복 C 심볼 발생은 소스상 위험이며 이번에 컴파일해 관측한 오류는 아니다. 상대 경로·링크 별칭의 중복 load, 누락 파일도 명시적 규칙이 필요하다.

**시험 계획 / 수정 방향:** `.fl`에서 하이픈·밑줄 두 함수를 정의해 서로 다른 반환값을 확인한다. 내장 함수와 같은 이름의 지역·전역 정의, 재정의 전 함수 값을 잡은 클로저, db95 복구 함수 override를 각각 검사한다. 같은 파일의 두 경로 load는 부작용 1회, 없는 파일은 빌드 실패로 정답 고정한다. 사용자 심볼을 충돌 없이 인코딩하고 어휘적 이름 해석을 내장 분기보다 먼저 적용한다. 재정의의 기존 참조 처리 정책은 기준 평가기 경로를 확인한 뒤 명시한다. 앱에서 이름 바꾸기·의존성 삭제로 우회하지 않는다.

<a id="c02"></a>

### C02 — 함수 값, swap!, 문자열 HTTP 처리 함수

**현재 원문:** X `cgc-swap!`의 기본 분기는 `(str (c-name $op) "(" $deref ... )`다. 직접 `conj`는 `fl_conj`, 직접 `update`는 별도 표준 함수 분기를 타지만 `swap!`의 이름 변환 호출은 이를 공유하지 않는다. D `tcp-server.fls:372`는 `(swap! afj-tcp-metrics update "errors" ...)`를 사용한다. B `store.fl:21`은 `(swap! ... assoc ...)`를 사용한다. X `runtime/http.c`는 처리 함수 이름을 C 이름으로 변환해 `dlsym(RTLD_DEFAULT, ...)`로 찾는다.

**차이·기대:** 함수 값으로 넘긴 내장 함수도 직접 호출과 같은 의미·인자 수를 가져야 한다. `conj`·C 예약어 변환 대상 `update` 등은 직접 호출의 연결과 다르다. 이름 문자열 처리 함수가 네이티브 링크에서 실제로 노출되는지는 미검증이다.

**시험 계획 / 수정 방향:** `(swap! (atom []) conj 1)`, 추가 인자 있는 `assoc`·`update`, 캡처된 람다, map/filter/reduce/any?의 함수 값 전달을 `.fl` 자체 시험으로 고정한다. 이름 문자열의 GET/POST 처리 함수를 등록하고 실제 HTTP 요청으로 검증한다. 컴파일러의 표준 함수 호출·함수 값 래퍼를 통합하고 명시적 함수 등록표 또는 네이티브 링크 심볼 공개를 표준 빌드에 포함한다. Python 빌더로 해결하지 않는다.

<a id="c03"></a>

### C03 — get 기본값, nil, 멤버 존재

**현재 원문:** X `self/cgc-main.fl:1053`은 `get(`에 `(get $args 0)`, `(get $args 1)`만 출력한다. X `runtime/collection.c:331`의 선언은 `FLValue get(FLValue obj, FLValue key)`이며 nil이면 `fl_throw`한다. V `bootstrap.js`의 일반 평가 경로는 `const _getDef = args3.length >= 3 ? args3[2] : null;`이고 Map에는 `has`를 사용한다. X `fl_contains_p`는 `v.tag != FL_NIL`로 키 존재를 판단한다. `has-key?` 공개 연결은 없다.

**차이·기대:** 세 번째 인자를 버리지 말아야 한다. `(get {} "x" 7)`은 7, 존재하는 false·0·빈 문자열은 원래 값이어야 한다. Map의 저장된 nil은 누락과 구분한다. `has-key?`는 nil 값을 저장한 키도 true다. v11의 배열·문자열 접근은 `??` 기본값, Map 접근은 `has`여서 nil 처리 차이가 있다. `get nil`은 v11 `FL_STRICT=1` 여부와 평가 경로에 따라 다르다. 이를 모두 “nil이면 기본값”으로 단순화하지 않는다.

**시험 계획 / 수정 방향:** map/vector/string/nil × 누락/존재/nil/false/0/빈 문자열 × 2·3인자, 기본값 부작용 평가 횟수·예외를 `.fl` 표로 고정한다. strict 모드 계약은 먼저 결정한다. 표준 get 오버로드와 실제 맵 키 존재 primitive를 추가하고 컴파일러 인자 수 검사를 한다. 앱 전역에 nil 검사·대체 함수를 삽입하지 않는다.

<a id="c04"></a>

### C04 — date_parse, now_ms, format_date (L04)

**현재 원문:** V `src/stdlib-time.ts`는 `"now_ms": (): number => Date.now()`, `date_parse`는 `const ts = Date.parse(str);`와 `if (isNaN(ts)) throw new Error(...)` 후 `return ts;`다. `format_date`는 `YYYY MM DD HH mm ss SSS` 토큰을 치환한다. X `runtime/aliases.c:1249`는 `FLValue date_parse(FLValue str_v, FLValue fmt_v)`이며 `strptime`, 실패 nil, `return fl_int((int64_t)mktime(&tm_info));`다. fx의 `now-ms` 구현과 별개로 앱의 `now_ms`·`format_date` 공개 연결은 없다.

**차이·기대:** 앱의 1인자 ISO 입력·밀리초 타임스탬프·오프셋·오류 동작을 언어에서 보존한다. fx의 2인자 strptime·초 단위와 같은 계약이라고 주장하지 않는다. 로컬 시간 기반 표시와 UTC 저장을 구분한다.

**시험 계획 / 수정 방향:** `(date_parse "1970-01-01T00:00:01Z")` 기대 1000, 같은 순간의 `+09:00` 입력도 1000, 잘못된 입력은 고정 오류, 윤일·DST·TZ=UTC/Asia/Seoul·밀리초 소수부를 검사한다. now_ms는 앞뒤 측정 범위와 단위를, format_date는 고정 시각의 토큰을 검사한다. L04에서 native ISO 파서·ms API·시간대 계약을 추가한다. 앱 데이터 나누기/곱하기·기존 날짜 재작성 금지.

<a id="c05"></a>

### C05 — 문자열 API, 인자 수와 순서

**현재 원문:** V `bootstrap.js:27290`의 `str_join`은 `if (Array.isArray(a)) return a.join(String(b ?? ""));` 및 반대 순서를 지원한다. X `runtime/io.c:308`은 `FLValue str_join(FLValue sep, FLValue vec)`이고 두 번째 인자가 벡터가 아니면 빈 문자열이다. V `str_slice`는 `end !== void 0 ? Number(end) : void 0`; X `runtime/aliases.c:488`은 항상 3인자다. V `str-replace-in`은 `String(s).replaceAll(String(oldStr), String(newStr))`다.

**차이·기대:** 앱이 쓰는 벡터 먼저 join, 2인자 slice(끝까지), 음수 인덱스를 지원한다. `str_find`, `str_starts`, `str_ends`, `str_lines`, `str_encode_uri`, `str_decode_uri`, `str-replace-in`의 정확한 공개 이름을 추가한다. 유사 이름의 C 함수만 있는 상태를 완료로 보지 않는다. str_replace는 fx에도 전체 리터럴 치환이 있어 “없음”으로 분류하지 않는다. v11의 선택적 치환 횟수 등은 호출별 확인 대상으로 남긴다.

**시험 계획 / 수정 방향:** `(str_join ["a" "b"] ",")` → `"a,b"`, `(str_slice "#tag" 1)` → `"tag"`, 음수·경계 밖 인덱스, 빈 패턴·반복 치환·빈 줄·CRLF, URI의 한글·공백·`%`·`/`·`#`·잘못된 percent escape의 오류를 고정한다. native stdlib에 정확한 바인딩·오버로드를 추가한다. 앱의 인자 순서 변경 금지.

<a id="c06"></a>

### C06 — 문자열 단위, 숫자 변환, HTML 안전 처리

**현재 원문:** X `collection.c`의 문자열 length/get은 `strlen`과 `char buf[2] = { s[idx], '\0' };`를 쓴다. V 문자열 slice는 JS String.slice다. X `str_to_num`은 `return parse_float(v);`이며 `strtod` 기반이다. V `str-to-num`은 `Number(String(s || "").trim())`과 `isNaN(n) ? null : n`이다. X trim은 ASCII 공백 처리를 사용한다.

**차이·기대:** 한글 길이·자르기를 바이트로 바꾸면 요약·태그·본문이 달라진다. v11 UTF-16 코드 단위와 fx UTF-8 바이트를 구분한다. 숫자 뒤 쓰레기 문자, 빈 문자열, 비숫자의 nil/0·오류 처리를 고정한다. `html-escape`는 fx에도 있으나 동일 출력은 아직 시험하지 않았다.

**시험 계획 / 수정 방향:** `(count "한글")`의 v11 기대 2와 fx 소스상 바이트 길이 6의 차이, slice/get 한글·emoji·NUL·잘못된 UTF-8을 시험한다. emoji의 UTF-16 호환 여부는 API 결정 후 정답을 고정한다. `"12x"`, `""`, `" 12 "`, 음수·지수·overflow, Unicode 공백·대소문자, HTML `&<>"'`·이미 이스케이프한 문자열을 포함한다. 명시적 텍스트/바이너리 API와 native 안전 인코딩을 설계하며 L07 raw API를 UTF-8 변환으로 바꾸지 않는다.

<a id="c07"></a>

### C07 — 정규식

**현재 원문:** V `bootstrap.js`의 regex API는 문자열 먼저, `RegExp` 기반이다. `regex_replace`는 `new RegExp(pattern, "g")` 치환이다. X `runtime/regex.c`의 계약 주석은 `regex_match pattern str`와 `regex_replace pattern str repl → 결과 문자열 (첫 번째만)`이며 POSIX `REG_EXTENDED`를 쓴다. 치환은 리터럴 복사이고 `regex_match_i` 공개 연결은 없다. B editor와 D sql-core는 `\\s`, lazy `.*?`, 캡처 치환 `$1`·`$2`를 사용한다.

**차이·기대:** 순서·문법·대소문자 옵션·전역 치환·캡처·오류가 모두 점검 대상이다. POSIX ERE를 같은 기능이라고 할 수 없다.

**시험 계획 / 수정 방향:** 실제 앱·db95의 모든 리터럴 패턴/치환을 추출해 `.fl` golden 표로 먼저 고정한다. 공백 압축, SQL 괄호/인용부호, HTML·이미지·링크 토큰, 캡처 치환, 잘못된 패턴, 무매치·빈 매치·한글을 포함한다. native C 정규식 엔진의 호환 지원/의존성 비교안을 제시하고 필요한 언어 API를 구현한다. 앱 정규식을 POSIX용으로 바꾸거나 JS 실행기로 넘기지 않는다.

<a id="c08"></a>

### C08 — 배열 API와 append

**현재 원문:** V `bootstrap.js`의 `arr_flatten`은 `return arr.flat();`(한 단계), `arr_unique`는 `new Set(arr.map((x) => JSON.stringify(x)))` 후 JSON.parse다. 정렬은 배열·키를 받는다. X는 `arr_flatten`, `arr_unique`, `arr_sort_by`, `arr_sort_by_desc`의 정확한 공개 연결이 없다. X 컴파일러의 `append` 분기는 `fl_vec_push`를 출력하지만 C `append`는 두 벡터를 이어 붙인다.

**차이·기대:** append가 중첩 배열을 만들면 안 된다. 키 정렬은 안정 정렬과 nil·동점 처리를 보존하고 unique는 기존 JSON 기반 의미와 일반 `=`를 혼동하지 않는다.

**시험 계획 / 수정 방향:** `(append [1] [2 3])` → `[1 2 3]`, 한 단계 flatten, 첫 등장 순서 unique, 중첩 map/vector·키 순서·동점 정렬·비숫자 키를 고정한다. native 표준 함수와 컴파일러 연결을 추가한다. 기존 `fl_sort_by(fn, vec)`·distinct를 이름만 바꿔 완료로 하지 않는다.

**추가 원문·계획:** X `fl_conj`는 벡터 외 입력이면 새 벡터에 item만 넣는다. V 일반 `conj`는 map 병합·키/값 쌍도 받는다. X 컴파일러의 `conj`·`obj-merge`·`concat`은 두 인자만 출력한다. V `concat`에는 문자열 누적 경로가 있고 D `sql-core.fls:416`은 배열에 사용하므로 평가 경로 확인 없이 호환이라고 할 수 없다. `.fl`에서 map conj, 3인자 conj/merge/concat, nil·혼합 타입, 배열 concat과 문자열 concat을 각각 검사하고 실제 사용한 경로의 정답을 먼저 고정한다. X `flatten-1`은 한 단계 flatten에 연결되어 있어 이 이름까지 누락이라고 세지 않는다.

<a id="c09"></a>

### C09 — 기본 문법·수·비교·출력

**현재 원문:** X `runtime/core.c`의 truthy는 숫자 0을 false로 취급하고, `fl_div`는 정수끼리 정수 나눗셈을 한다. `fl_eq`는 기본 타입 외 default false다. V `bootstrap.js` 일반 `=` 경로는 `v0 === v1`, or의 `flFalsy`는 nil/undefined/false를 다룬다. v11에 여러 평가 경로가 있으므로 JS truthiness로 일괄 대체하지 않는다.

**차이·기대:** 같은 객체 참조 비교와 서로 다른 동형 객체 비교를 구분한다. v11의 `=`를 구조적 동등성으로 단정하지 않는다. 나눗셈·integer?·숫자 태그·nil·short circuit이 db95 검증과 페이지 계산에 영향을 준다. `define/defn/let/fn/do/if/loop/recur/doseq/try/catch/throw` 등은 함수 누락 목록이 아닌 문법 점검 대상이다.

**시험 계획 / 수정 방향:** `(or 0 99)`, `(/ 3 2)`, 같은 map 참조 vs 별도 map, 1 vs 1.0, float 파싱 후 integer?, 여러 인자 산술·부작용 순서·꼬리 재귀·예외·호출 인자 수를 기준 경로별로 고정한다. 사용 중인 concat/conj/take/drop/range/flatten-1/map/filter/reduce/keys/vals/dissoc/obj-merge/타입 술어/println도 회귀 표에 넣는다. 현재 범위의 range 호출이 1인자라는 사실만으로 다른 오버로드를 필수라고 늘리지 않는다. 컴파일러·런타임 공통 의미를 수정한다.

<a id="c10"></a>

### C10 — 파일·디렉터리, 텍스트/바이너리 경계 (L07)

**현재 원문:** V `src/stdlib-file.ts`의 read는 `fs.readFileSync(..., "utf-8")`, write/append/copy는 부모 디렉터리를 만들고 true를 반환하며 IO 실패는 throw한다. delete는 없는 파일에 false다. X L07의 raw 바이트 읽기/쓰기·checked API는 별도 계약이다. 앱의 `file_read/file_write/file_append/file_copy/file_delete/file_is_file/file_size/dir_create/dir_list` 정확한 공개 연결은 없다(`file_exists` 등 유사 기능은 별도 존재).

B `images.fl:34` 원문은 PNG에 `(str_starts content "�PNG")`, JPEG에 `(str_starts content "���")`다. v11의 손실 있는 UTF-8 디코딩에 의존한다. raw PNG 첫 바이트 `0x89`와 U+FFFD 문자열은 같지 않다.

**차이·기대:** L07 raw 바이트 무손실을 보존하면서 기존 text API의 디코딩·반환·오류·경로 계약을 별도로 정의해야 한다. raw alias만 붙이면 이미지 검증이 호환되지 않는다. FL_FILE_BASE 검사 범위와 copy의 경로 검사 누락 여부까지 확인해 보안 계약을 강화하되 허용 경로는 보존한다. WAL 64 MiB 초과 읽기는 L07 상한과의 차이를 별도 결정한다.

**시험 계획 / 수정 방향:** 합성 파일에서 없는 파일·없는 부모·권한·copy/delete/rename·디렉터리·순서 비의존 목록·경로 이탈·심볼릭 링크를 검사한다. PNG/JPEG/GIF의 NUL·0x80~0xFF·invalid UTF-8와 L07 roundtrip을 동시에 시험한다. 언어에 명시적 text decoder/기존 text API 호환 계약을 추가하는 안을 우선 검토하고 binary signature primitive 대안과 비교해 결정받는다. 운영 이미지나 WAL을 다시 저장해 맞추지 않는다. 스트리밍/큰 파일은 L05와 연결한다.

<a id="c11"></a>

### C11 — fd와 WAL 내구성 (L05)

**현재 원문:** D `tcp-server.fls:376` 부근은 `fd_open` → `fd_write` → 조건부 `fd_fsync` → `fd_close` 순서다. V `src/stdlib-fd.ts`는 실제 `fs.fsyncSync`를 호출한다. X에 `fd_open/fd_write/fd_fsync/fd_close` 공개 연결이 없다. fflush/fclose는 fsync와 같은 보장이 아니다.

**기대 / 시험 계획 / 수정 방향:** native 핸들 API에 UTF-8/바이트 수·부분 쓰기·EINTR·invalid/closed handle·fsync/close 오류를 명시한다. `.fl`에서 합성 WAL을 append하고 native 재시작·truncate된 끝·추가 후 삭제·반복 재생을 검사한다. C fault injection은 보조 증거이고 `.fl` 호출 시험을 대신하지 못한다. fd API·내구성·잠금을 언어에 추가한다. file_append 치환·fsync 삭제·기록 제거 금지.

<a id="c12"></a>

### C12 — JSON·해시와 기존 WAL

**현재 원문:** `json_str` 공개 연결은 없지만 fx의 `json_parse`, `json-parse`, `json-stringify`는 있다. X JSON 숫자 출력에는 `%.17g`가 사용된다. D는 `sha256 (json_str ...)`로 WAL 체크섬을 만든다. `auth_sha256` 정확한 공개 연결은 없고 native SHA 구현은 있다.

**기대 / 시험 계획 / 수정 방향:** native JSON 직렬화의 키 순서·숫자·문자열 escape·nil/false·Unicode를 기존 fixture 바이트와 비교한다. 파싱 성공만으로 WAL 호환이라 판정하지 않는다. `.fl` golden에 1/1.0/소수/지수/큰 정수/중첩 map·빈 배열·제어 문자와 잘못된 JSON의 오류 계약을 고정한다. 합성 기존 WAL의 체크섬 검증·재시작을 포함한다. native `json_str`·해시 바인딩 및 호환 직렬화를 추가한다. 기존 WAL 체크섬 재계산·원문 재작성 금지.

<a id="c13"></a>

### C13 — 인증·CSRF·난수 (L02/L05)

**현재 원문:** V `src/stdlib-auth.ts`는 `$scrypt$N=16384,r=8,p=1$` 형식(키 64바이트·salt 16바이트)과 기존 `salt_hex:sha256_hex`를 검증한다. CSRF는 timestamp·HMAC과 유효 기간을 사용한다. fx에 `auth_bearer_extract`, `auth_csrf_token`, `auth_csrf_verify`, `auth_hash_password`, `auth_password_needs_rehash`, `auth_random_token`, `auth_sha256`, `auth_verify_password`의 정확한 공개 연결이 없다.

**기대 / 시험 계획 / 수정 방향:** native 암호 라이브러리로 두 합성 password fixture 형식의 검증·오답·손상·rehash 판단을 보존한다. `.fl`에서 난수 길이/hex·중복 방지 표본, Bearer/누락/형식 오류, CSRF 변조·만료·미래 timestamp·constant time 경로를 확인한다. CSPRNG·scrypt·HMAC·상수 시간 비교·checked 오류를 언어에 추가한다. 실제 비밀번호·해시를 증거에 남기거나 약한 해시·고정 토큰으로 우회하지 않는다.

<a id="c14"></a>

### C14 — HTTP 요청·응답·미들웨어·정적 파일 (L02)

**현재 원문:** X에 `server_use/server_static/server_body_limit/server_header/server_set_cookie/server_redirect_cookie/server_req_cookie/server_req_json` 공개 연결이 없다. `runtime/http.c`의 route matcher에는 앱의 `/*` 패턴 의미가 없고, 요청 헤더 조회는 map 키의 대소문자를 그대로 쓴다. V 서버 헤더는 소문자화한다. X 응답 전송은 문자열 `strlen` 경로를 사용한다.

**차이·기대:** 스케줄 tick·이미지 권한 미들웨어의 순서/단락, wildcard 404, Host 기반 블로그 구분, 실제 status/Content-Type/복수 Set-Cookie/redirect를 보존한다. binary 이미지의 NUL에서 응답이 잘리면 안 된다. 처리 함수 이름 연결은 C02다.

**시험 계획 / 수정 방향:** `.fl` 서버에 실제 loopback HTTP로 GET/POST·404·401·redirect·쿠키·헤더 case·중복·query/path percent decode·JSON 오류·static traversal·권한 거부를 시험한다. 모든 바이트가 든 이미지의 응답 길이·해시·MIME을 고정한다. native middleware·route·cookie·header API, 길이를 받는 binary response를 추가한다. 권한 미들웨어 삭제·파일 공개 우회 금지.

<a id="c15"></a>

### C15 — 폼·multipart의 L06 이후 경계

**현재 원문:** X L06는 URL 인코딩 필드를 body map으로 제공하고 `server_req_fields/server_req_files`를 제공한다. V multipart body는 `fields/files` 구조이며 비multipart `server_req_fields`의 null과 fx 빈 map은 구분해야 한다. fx는 처리 함수가 끝나면 임시 파일을 정리한다. 앱은 업로드 파일을 처리한 뒤 별도로 지우며 파일 5242880바이트 정책을 둔다.

**기대 / 시험 계획 / 수정 방향:** L06 언어 자체 시험 통과와 blog1 통합 호환은 별개다. `.fl` HTTP 시험으로 빈/중복 필드·한글·plus/percent·파일 여러 개·15 MiB/64 MiB/4개 경계·수신 중단·bad boundary·handler 오류·앱의 조기 삭제/복사 후 정리·임시 파일 0개를 확인한다. `server_body_limit`의 앱 정책과 런타임 hard cap을 함께 검사한다. 부족한 body/accessor/null 계약은 언어에서 보완하고 파서나 보안 상한을 앱에 복제하지 않는다.

<a id="c16"></a>

### C16 — HTTP 클라이언트

**현재 원문:** V `src/stdlib-http.ts`의 public `http_request`는 method/url/headers/body 순서다. X에는 다른 이름 `http_req`가 method/url/body/headers 순서로 있고 `http_request` 공개 연결이 없다. `http_get`은 존재하나 오류·body 인코딩의 일치는 미실행이다.

**기대 / 시험 계획 / 수정 방향:** native stdlib에 기존 public 계약을 추가한다. `.fl`에서 loopback echo에 GET/POST·헤더·빈 body·한글·status/response map·연결 실패·timeout을 검사한다. TLS는 로컬 시험 의존성이 갖춰진 경우만 수행하고 아니면 미실행으로 남긴다. 앱/시험에서 인자를 뒤집거나 Node HTTP proxy를 쓰지 않는다.

<a id="c17"></a>

### C17 — RSS·sitemap

**현재 원문:** V `src/stdlib-feed.ts`에 `rss_feed(meta, items)`, `sitemap_xml(baseUrl, routes)`가 있다. X의 정확한 public 연결은 없다.

**기대 / 시험 계획 / 수정 방향:** native serializer에 XML escape·날짜·절대 URL·빈 목록을 추가한다. `.fl`에서 합성 글의 한글/`&<>`·인용부호·timezone·선택 필드·정렬의 고정 XML을 검사한다. 앱에 문자열 XML 생성 우회를 추가하지 않는다.

<a id="c18"></a>

### C18 — 환경·오류·프로세스·식별자

**현재 원문:** 외부 호출 목록의 `env-or/error_message/process-exit/uuid_short`는 현재 선택 컴파일러/런타임의 정확한 public 연결이 없다. `env-get`·uuid 관련 native 기능은 존재하므로 별도 의미 확인이 필요하다. V `bootstrap.js`에는 `error_message` 구현과 별칭이 있다.

**추가 원문:** V `bootstrap.js:32801`의 env_or는 `val === void 0 || val === "" ? defaultVal : val`, env_get은 같은 조건에 null이다. X `_fl_env_get`은 `return v ? fl_str_val(v) : fl_nil();`이라 존재하는 빈 문자열을 보존한다. V `process_exit`는 `process.exit(code ?? 0)`, `uuid_short`는 `nodeCrypto.randomBytes(4).toString("hex")`다. error_message는 Error.message, 문자열 그대로, truthy object.message의 String 변환, 나머지 String(err) 순서다. 이름의 밑줄/하이픈 별칭은 V의 이름 해석 경로까지 고정해야 한다.

**기대 / 시험 계획 / 수정 방향:** `.fl`에서 환경 변수 누락/빈 값/기본값 평가, throw 값의 문자열·map·중첩 오류, 자식 native 과정 기본 0/명시 7 종료, 짧은 UUID 8자리 hex·충돌 표본을 고정한다. `(error_message "sample")`은 `"sample"`이다. 일반 map의 표시 문자열·nil 오류 문자열은 native에서 임의로 정하지 않고 기준 평가 경로와 정답을 먼저 고정한다. native stdlib 바인딩/오류 모델을 추가하고 종료 시험은 시험 자식만 대상으로 한다.

<a id="c19"></a>

### C19 — 타이머·TCP·TLS (L03/L05)

**현재 원문:** D가 호출하는 `set-timeout/fl-yield/tcp-server-raw/tcp-server-tls/tcp-write/tcp-drop/tcp-server-stop`의 선택 네이티브 빌드용 공개 연결이 없다. V `bootstrap.js`에는 `set-timeout`·`fl-yield` 평가 분기가 있다. 블로그가 TCP 서버를 실제 시작하지 않더라도 load한 함수 본문은 컴파일 대상이다.

**기대 / 시험 계획 / 수정 방향:** `.fl`에서 타이머 1회·순서·취소/종료·캡처 수명·오류 전달, loopback TCP 연결/부분 쓰기/끊김/재시작, TLS 인증서 오류/정상 연결을 계획한다. 타이머의 스핀 대체·함수 삭제·미사용 선언 잘라내기로 숨기지 않는다. 언어에 상주 작업·native 소켓·오류/수명 계약을 추가한다. 선택 빌드에서 제공되지 않는 TLS는 미실행으로 남긴다.

<a id="c20"></a>

### C20 — atom 수명·동시성·지속 저장 (L05)

**현재 원문:** X `runtime/math.c:195`의 atom은 `fl_heap_copy`로 값을 보관하며 reset도 heap copy한다. 따라서 “heap 보존이 없다”는 주장은 틀리다. 다만 deref/read-modify-reset에 명시적 잠금이 없고, 컴파일러 swap!도 read 후 reset이다. `fl_sort_by`의 비교 함수 저장은 전역 상태다.

**기대 / 시험 계획 / 수정 방향:** `.fl` HTTP 서버에서 요청 사이 atom 값 유지, 반복 할당·재시작·동시 요청의 유실 업데이트·use-after-free·정렬 간섭·WAL 일관성을 시험한다. 필요하면 언어 atom의 원자적 갱신·수명 보호와 저장 동기화 primitive를 구현한다. worker 1개로 제한한 시험만으로 동시성 완료를 선언하지 않는다. 일반 HTTP와 L06 병렬 업로드 시험이 저장소 동시성 증거를 대신하지 못한다.

## 5. 구현 전 정답 고정과 진행 순서

1. 이 초안의 미결정 계약(6장)을 먼저 결정한다. 실제 운영 실행기와 비교 기준 버전이 다르면 소스 기준을 갱신한다. 금지 실행기로 baseline을 새로 만들지 않는다.
2. C01·C02의 최소 `.fl` 재현과 expected fixture를 먼저 커밋한다. 그 다음 C03/C05/C06/C08/C09처럼 다른 시험의 토대가 되는 언어 의미를 순서대로 고친다.
3. L07 위 C10, L06 위 C14·C15, L04 위 C04, L05 위 C11·C12·C13·C20, L03 위 C19를 연결한다. 정규식·HTTP 클라이언트·feed·환경 API도 남김없이 포함한다. 브라우저 출력은 L01 J안(컴파일러 생성 JS 중심)으로 별도 진행하며 사람이 쓰는 새 JS는 금지한다.
4. 각 묶음마다 **실패 재현 `.fl`와 정답 먼저 커밋 → 수정 전 실패 원문·종료 코드 → 언어 수정 → 자체 시험 통과 원문·종료 코드 → 회귀** 순서로 진행한다. 정답 변경으로 구현에 맞추지 않는다. C 직접 호출만으로 완료 판정하지 않는다.
5. 합성 fixture로 모든 blog1 시험을 각각의 진입점에서 실행하고 복사본 서버로 HTTP·로그인·이미지 업로드/표시·WAL 추가 후 삭제·재시작을 검증한다. 운영 데이터는 변경하지 않는다. 언어 자체 시험과 블로그 통합 결과를 별도로 집계한다.
6. native 빌드·전체 자체 회귀·`verify-native-bootstrap.sh`를 수행한다. 각 명령을 `strace -f -e trace=execve`로 추적해 Node·JS 실행기·Python 미호출을 확인한다. 단순 확장자 검색이 아니라 실행 파일/argv를 검토하고 명령·원문·종료 코드·trace 해시·커밋 SHA를 제출한다. 의존성이 없어 미실행한 시험은 전체 완료에 포함하지 않는다.
7. 화면 검증이 필요한 통합 단계는 PRD 기준 768×1024 캡처를 남긴다. 언어 CLI 자체 시험에 무관한 화면 캡처를 증거로 대체하지 않는다.
8. 작업 가지에서만 커밋하고 Forgejo → GitHub 순서로 같은 가지를 정상 푸시한다. 두 원격 ref의 전체 HEAD SHA를 재조회해 보고한다. 합치기·배포는 하지 않는다.

## 6. 사용자 결정·확인 필요

| 항목 | 제안 / 대안 | 결정 전 상태 |
|---|---|---|
| 기준 v11 실행 경로 | 실제 사용했던 evaluator·FL_STRICT 설정을 비밀 없는 증거로 고정. 없으면 소스 계약과 제안 정답을 구분 | 미확인. baseline 실행 출력 없음 |
| get nil·기본값 평가 | strict/비strict와 map/vector 차이를 명시적으로 보존 / 향후 일관 API는 별도 이름 | 결정 필요 |
| Unicode 문자열 인덱스 | 기존 text API의 UTF-16 호환 / Unicode scalar API 별도 제공. raw byte API는 L07 유지 | emoji·잘못된 UTF-8 정책 결정 필요 |
| text 파일·이미지 signature | 언어의 legacy UTF-8 replacement decoder와 명시적 binary API 공존 / binary signature API 신규 계약 | 기존 앱 변경 없이 호환 가능한 안을 우선 설계, 결정 필요 |
| 정규식 엔진 | C native 엔진의 JS 문법 호환 범위·의존성 비교 후 선택 | JS 호출·앱 패턴 변경은 불가 |
| 날짜·JSON·WAL 큰 파일 | ms/시간대·직렬화 byte 계약·streaming/상한을 fixture로 고정 | 운영 자료 변경 불가, 비운영 fixture 증거 필요 |
| 재정의·동일 참조·출력·오류 | 기준 평가 경로의 소스와 `.fl` 정답 표를 먼저 고정 | 실행으로 같다고 아직 판정하지 않음 |

초안 당시 미결정 항목은 아래 사용자 결정으로 갱신한다. 각 차수에 명시된 승인 범위만 착수한다.

### 6장 사용자 결정 확정 — L08 1차 구현 승인

- get 기본값은 v11 소스 계약을 보존한다. 일반 평가기의 map 존재 여부·배열 nil 기본값·기본값의 선행 평가·FL_STRICT nil 오류를 기준으로 정답을 고정한다. Node로 기준 출력을 만들지 않는다.
- 기존 문자열 길이/인덱스 이름은 v11 호환 글자 단위, 바이트는 L07 API로 분리한다. 이번 C03의 get 문자열 인덱스만 다루고 다른 문자열 API는 착수하지 않는다.
- 텍스트/바이너리 API를 공존시킨다. 이번에는 파일 API를 변경하지 않는다.
- 정규식은 C 자체 엔진으로 blog1 사용 패턴부터 지원한다. 이번에는 착수하지 않는다.
- 기준은 v11 소스 계약이며 새 TS·JS·Python·fl-build.sh 및 Node 실행은 금지한다.
- 1차 승인 범위는 C01·C02·C03, 가지는 `feat/l08-c01-c03`이다. 다른 C 항목은 구현하지 않는다.

### 6장 사용자 결정 확정 — L08 2차 구현 승인

- get 기본값은 v11 소스 동작을 보존한다.
- 기존 문자열 길이·인덱스 이름은 v11 호환 글자 단위(소스 기준 UTF-16 코드 단위)로, 바이트는 L07 API로 구분한다.
- 텍스트/바이너리 API를 공존시킨다. 이번에는 텍스트 연산을 다루며 L07 raw 계약은 유지한다.
- 정규식은 C 자체 엔진으로 blog1 사용 패턴부터 지원한다. C07은 이번 승인 범위가 아니므로 구현하지 않는다.
- 정답은 v11 소스 계약으로 고정하고 Node 실행은 금지한다.
- 승인 범위는 C05·C06·C08·C09와 1차에 남은 숫자 map 키·소수 표기다. 한글·이모지·잘못된 UTF-8을 포함한다.
- 가지는 feat/l08-c05-c09이며 다른 C 항목·서버 23·25 접속·합치기·배포·강제 푸시는 금지한다.

### 6장 사용자 결정 확정 — L08 3차 구현 승인

- 승인 범위는 C04 날짜(date_parse·now_ms·format_date, 밀리초·한국 시간 포함 시간대), C07 C 자체 정규식 엔진, C10 텍스트/바이너리 파일 API 공존·이미지 무손실·경로 이탈/심볼릭 링크 차단이다.
- 정규식은 blog1과 실제 load한 의존 소스의 패턴 전수 목록을 먼저 고정한다. 외부 정규식 라이브러리가 필요하면 이름·라이선스·이유를 보고하고 사용자 승인 전에 커밋하지 않는다.
- 정답은 v11 원문과 해당 표준 계약으로 고정한다. 정답 정정 시 근거와 이력을 기록하고 구현에 맞춰 바꾸지 않는다.
- 실패 재현 `.fl`·정답 선행 커밋, 수정 전 실패, 구현, 자체 시험, L06·L07·L08 1·2차 회귀, 변경하지 않은 native bootstrap 순서다.
- 가지는 `feat/l08-c04-c07-c10`이며 다른 C 번호·서버 23·25 접속/작업·합치기·배포·강제 푸시·TS/JS/Python/fl-build.sh는 금지한다. Forgejo → GitHub 순서로 같은 가지를 푸시하고 두 HEAD를 실제 조회한다.

## 7. 확인한 것 / 미확인

**확인한 것:** 고정 커밋 소스 대조, 74개 파일의 정적 호출 목록, 정확한 이름이 없는 API와 동일 이름의 인자/단위/연결 차이, 함수 값 전달·사용자 심볼 충돌, L06/L07 이후에도 남은 통합 경계를 문서화했다. 이번 가지의 변경은 Markdown뿐이다.

**미확인:** fx로 blog1을 컴파일·실행하지 않았다. `.fl` 재현·전체 회귀·native bootstrap·strace·TLS·HTTP·화면 검증은 이번 문서 작업에서 실행하지 않았다. 소스상 예상되는 컴파일 오류는 실제 오류 출력으로 보고하지 않았다. 운영 기준 v11 실행 경로·운영 데이터 호환·계정 호환은 미검증이다. 따라서 L08 구현 완료·이전 가능 판정은 하지 않는다.

## 8. 원문 파일 바로가기

인용 문맥을 재검토할 때 사용하는 고정 SHA 링크다. 전체 함수 목록의 첫 사용 위치·정확한 C 선언 원문·컴파일러 행 링크는 호출 부록에 있다.

| 점검 | v11 기준 원문 | fx 현재 원문 |
|---|---|---|
| C01~C03·C08·C09 이름·문법·기본값·컬렉션 | [bootstrap 일반 평가기·별칭](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/bootstrap.js) | [컴파일러](https://github.com/kimjindol2025/freelang-v11-fx/blob/7566eb1aa3c07de617e71482e0a051587d718725/self/cgc-main.fl), [get·length](https://github.com/kimjindol2025/freelang-v11-fx/blob/7566eb1aa3c07de617e71482e0a051587d718725/runtime/collection.c#L331), [비교·수](https://github.com/kimjindol2025/freelang-v11-fx/blob/7566eb1aa3c07de617e71482e0a051587d718725/runtime/core.c#L53) |
| C04 날짜 | [stdlib-time](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/src/stdlib-time.ts#L48) | [date_parse](https://github.com/kimjindol2025/freelang-v11-fx/blob/7566eb1aa3c07de617e71482e0a051587d718725/runtime/aliases.c#L1249) |
| C05~C08 문자열·정규식·배열 | [bootstrap 문자열](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/bootstrap.js#L27243), [배열](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/bootstrap.js#L27400), [정규식](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/bootstrap.js#L28041) | [문자열 별칭](https://github.com/kimjindol2025/freelang-v11-fx/blob/7566eb1aa3c07de617e71482e0a051587d718725/runtime/aliases.c#L68), [join](https://github.com/kimjindol2025/freelang-v11-fx/blob/7566eb1aa3c07de617e71482e0a051587d718725/runtime/io.c#L308), [regex](https://github.com/kimjindol2025/freelang-v11-fx/blob/7566eb1aa3c07de617e71482e0a051587d718725/runtime/regex.c) |
| C10~C11 파일・fd | [stdlib-file](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/src/stdlib-file.ts), [stdlib-fd](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/src/stdlib-fd.ts) | [io](https://github.com/kimjindol2025/freelang-v11-fx/blob/7566eb1aa3c07de617e71482e0a051587d718725/runtime/io.c), [L07 계약](https://github.com/kimjindol2025/freelang-v11-fx/blob/7566eb1aa3c07de617e71482e0a051587d718725/docs/L07-contract.md) |
| C12 JSON | [기준 bootstrap](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/bootstrap.js) | [json](https://github.com/kimjindol2025/freelang-v11-fx/blob/7566eb1aa3c07de617e71482e0a051587d718725/runtime/json.c) |
| C13 인증 | [stdlib-auth](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/src/stdlib-auth.ts) | [public runtime 선언](https://github.com/kimjindol2025/freelang-v11-fx/blob/7566eb1aa3c07de617e71482e0a051587d718725/runtime/runtime.h) |
| C14~C15 HTTP 서버 | [stdlib-http-server](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/src/stdlib-http-server.ts) | [http](https://github.com/kimjindol2025/freelang-v11-fx/blob/7566eb1aa3c07de617e71482e0a051587d718725/runtime/http.c), [L06 계약](https://github.com/kimjindol2025/freelang-v11-fx/blob/7566eb1aa3c07de617e71482e0a051587d718725/docs/L06-multipart-instruction-draft.md) |
| C16 HTTP 클라이언트 | [http_request](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/src/stdlib-http.ts#L399) | [http_client](https://github.com/kimjindol2025/freelang-v11-fx/blob/7566eb1aa3c07de617e71482e0a051587d718725/runtime/http_client.c) |
| C17 feed | [stdlib-feed](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/src/stdlib-feed.ts) | [public runtime 선언](https://github.com/kimjindol2025/freelang-v11-fx/blob/7566eb1aa3c07de617e71482e0a051587d718725/runtime/runtime.h) |
| C18 환경·오류·식별자 | [stdlib-process](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/src/stdlib-process.ts), [error_message](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/bootstrap.js#L26085), [uuid_short](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/bootstrap.js#L28033) | [env](https://github.com/kimjindol2025/freelang-v11-fx/blob/7566eb1aa3c07de617e71482e0a051587d718725/runtime/io.c#L285) |
| C19~C20 TCP·타이머·수명·동시성 | [db95 TCP 사용부](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/tcp-server.fls#L296) | [atom heap 보존](https://github.com/kimjindol2025/freelang-v11-fx/blob/7566eb1aa3c07de617e71482e0a051587d718725/runtime/math.c#L195), [정렬 공유 상태](https://github.com/kimjindol2025/freelang-v11-fx/blob/7566eb1aa3c07de617e71482e0a051587d718725/runtime/aliases.c#L678) |
