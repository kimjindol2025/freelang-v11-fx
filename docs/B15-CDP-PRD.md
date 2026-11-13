# B15 CDP·PRD·본체 메모리 검수

B15=보류. 본체 생성은47.48초·1592696KiB로 목표1677722KiB 이하를 충족했다. 새 생성 C 고정점·J_UTF16_LENGTH·API20·누수12는 통과했다. PRD 파생 판정은1/24, B8은2/4이며 전체 TLS 회귀 세 구성은 승인 대기 미실행이다. 아래에 수정 전 실패·수정 근거·최종 실행 범위를 구분한다.

출발 HEAD=6e21df51a09453d7be7b45d10e7551c90b75b555, CLEAN에서 fix/b15-cdp-prd를 만들었다. 실제 경로·소유자·주소·합성 인증값은 <가림>이다. 기존 시험·기대값·제한시간·러너·PRD 원표·seed는 변경하지 않았다. 무거운 작업 직전에 ps와 다른 실행 상태를 확인했고, 공용 flock·2147483648바이트 cgroup·swap 0·주소 공간 2097152KiB·core 0 아래 순차 실행했다. TMPDIR와 산출물은 작업 전용 영역 <가림>에만 두었다.

SEED_SHA_BEFORE=f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c

## CDP의 정확한 실패 지점

원래 C CDP 검사와 원래 앱/seed 실행 파일을 그대로 쓰고, 임시 native libcurl 관찰기에서 메시지 번호·메서드·nodeId·오류 코드와 문서 이벤트만 기록했다. 실제 URL·쿠키·본문·세션 값은 기록하지 않는다. 송수신을 바꾸지 않고 원래 함수를 그대로 호출한다. 서버·브라우저 접속은 성공했고 다음 순서가 확인됐다.

```text
TX id=8 method=Page.navigate node=-1
RX id=8 method=Page.navigate node=-1 error=-1
TX id=9 method=DOM.getDocument node=-1
EVENT Page.frameNavigated
EVENT DOM.documentUpdated
RX id=9 method=DOM.getDocument node=1 error=-1
TX id=10 method=DOM.querySelector node=1
RX id=10 method=DOM.querySelector node=37 error=-1
TX id=11 method=CSS.getComputedStyleForNode node=37
EVENT DOM.documentUpdated
EVENT Page.loadEventFired
RX id=11 method=CSS.getComputedStyleForNode node=-1 error=-32000
```

원래 메시지는 Could not find node with given id, EDITOR_UI_CDP_FAILURE=protocol_error, 종료 1이었다. JSON 응답의 nodeId 37은 요청에 그대로 들어갔으므로 nodeId 직렬화 손상이 아니다. 연결·포트 실패도 아니다. 문서 세대가 바뀌는 동안 검사기가 이전 nodeId를 사용한 브라우저/CDP 시간 문제다. B14의 수정 전 전체 C 대조에서도 같은 메시지가 났던 별도 문제와 일치한다. 원래 C 검사기는 querySelector의 -32000만 재조회하고 CSS 단계는 즉시 종료한다. 러너 수정 금지에 따라 이를 바꾸지 않았다. 미관측 단언 27개를 통과로 세지 않는다.

## 같은 입력의 새 기준 측정

B14에서 고정한 같은 입력·식 경계를 그대로 썼다. 1/2 입력은 정확한 절반보다 다음 완전한 최상위 식까지 커진 244409바이트다. source-only 모듈을 펼친 진단 입력이고 원래 load 경로 본체와 구분한다.

| 입력 | bytes | 수정 전 초 | 수정 전 최대 RSS KiB | 종료 |
| --- | --- | --- | --- | --- |
| 1/4 | 112998 | 6.67 | 536568 | 0 |
| 1/2 | 244409 | 19.25 | 1117888 | 0 |
| 전체 | 450607 | 48.56 | 1966292 | 0 |

원래 load 경로 본체의 str 계측 실행은 52.24초·1968768KiB·종료 0이었다. 아래는 arena 요청량이며 물리 RSS나 동시에 생존하는 바이트가 아니다.

| 남은 문자열 str 요청량 상위 | 호출 수 | arena 요청 bytes | heap scratch 요청 bytes |
| --- | --- | --- | --- |
| read-string-iter | 116886 | 16358539 | 151903280 |
| cgc-eval-args | 23555 | 6348101 | 55426134 |
| cgc-if | 1235 | 4399922 | 40022146 |

전체 arena 계측의 같은 본체 출력 C는 일반 str 계측과 byte 동일했다. 53.53초·1966820KiB·종료 0, PROFILE_OVERFLOW=0이다. 함수별 누적 요청량은 includes-item=6228694440bytes, advance=476620392bytes, peek-at=246333888bytes였다. 누적량을 RSS로 해석하지 않는다.

advance는 매 문자에 상태 map을 만든다. runtime/v11-compat.h의 fl_v11_map_from_pairs는 이미 평가한 literal 쌍을 다시 fl_map_set으로 하나씩 넣으며 매번 이전 map 전체를 복사한다. runtime/collection.c에는 한 번에 저장 공간을 만드는 기존 fl_map_from_pairs가 있다. 새 map의 key 정규화·중복 key의 마지막 값·첫 key 순서를 유지하면서 이 기존 API를 재사용할 수 있다. 문자열 누적만으로 모든 메모리를 설명하지 않고 이 별도 반복 복사도 수정 대상으로 확인했다.

문자열 reader는 UTF-16 단위와 escape를 유지하며 기존 지역 builder에 조각을 넣고 한 번 합치는 방향으로 확인한다. seed의 legacy join이 FL_TEXT 표식을 잃지 않는지도 기존 J_UTF16_LENGTH로 확인해야 한다. B13을 제거하거나 seed를 바꾸지 않는다.

## 원표 선행 인수와 아직 결정되지 않은 정책

G-AUTH는 실제 합성 로그인·실패/만료/CSRF/로그아웃, G-STORE는 native 거래/긴 WAL/재시작/실패 원자성, G-BROWSER는 DOM/이벤트/선택/입력/비동기/취소/저장·재접속과 재생성 증거, G-IMAGE는 JPEG/PNG/GIF 원본 보존과 검증된 처리 출력/상한, G-TIME은 타이머와 UTC/한국 시간/날짜 경계, G-FILES는 15MiB/64MiB/4개 한도와 HTTP/TLS·정리/경로/hash, G-NATIVE는 정확한 새 소스의 전체 언어/앱/고정점이다. 원표의 기존 수치와 관계를 유지한다.

D02 commit 불확실 상태 계약, D03 작성자 직접 발행, D04 초안/이력 기간·용량·로그아웃 정리, D05 이미지 의존성·GIF·pixel/예산, D06 댓글·미디어 권한 정책은 원표에서 미정이다. D04/D05의 값을 사용자에게 요청했고 답 전에는 새 정책/한도를 적용하지 않는다. 기존 TLS 회귀 세 곳이 합성 임시 키 파일을 만드는 사실도 확인해 이번 키 생성 금지의 예외 여부를 요청했다. 승인 전에는 해당 시험을 실행하지 않는다.

## 구현과 후보 확인

- self/cgc-main.fl의 read-string-iter는 기존 지역 vec-builder에 UTF-16/escape 조각을 저장하고 한 번 freeze/join한다. cgc-eval-args는 마지막 배열 선언도 같은 builder에 넣어 prefix를 다시 복사하지 않는다.
- 문자마다 같은 key 문자열을 만들던 peek-at·at-end?·advance·emit는 같은 내용의 다섯 불변 전역 key를 재사용한다. 문자 위치·상태 필드·원래 key 내용은 그대로다.
- fl_v11_map_from_pairs는 기존 fl_map_from_pairs로 새 map을 한 번 만들고, 그 새 map 안에서만 key 정규화·중복 압축을 한다. 첫 key 위치와 마지막 값 규칙을 유지하고 기존 map을 변경하지 않는다.
- runtime/math.c의 legacy join에서 FL_TEXT가 있으면 기존 v11 text join으로 보낸다. 고정 C seed 경로에서도 서로게이트 표식을 잃지 않게 하며, 일반 legacy byte 문자열 경로는 그대로다.

첫 후보는 본체 47.48초·1696412KiB로 목표보다 컸다. 수정한 다음 후보의 결과는 다음과 같다. 첫 후보의 수치를 숨기거나 목표를 바꾸지 않았다.

| 입력 | 수정 전 초/KiB | 최종 후보 초/KiB |
| --- | --- | --- |
| 1/4 | 6.67 / 536568 | 6.79 / 441124 |
| 1/2 | 19.25 / 1117888 | 19.22 / 907864 |
| 전체 펼친 입력 | 48.56 / 1966292 | 48.40 / 1595488 |

원래 load 경로 본체는 48.17초·1594128KiB·종료 0이었다. 1677722KiB 이하를 충족한다. 네 입력의 생성 C는 수정 전과 모두 byte 동일했다. 후보의 기존 native 정답 37개·거부 10개·J_UTF16_LENGTH는 통과했다. 원래 8192바이트 입력은 0.44초·86572KiB·종료 0이었다. 이 값은 아직 정식 seed→stage4 재검증 전 후보 측정이다.

임시 조정 스크립트 한 번은 Bash 함수 선언 공백 누락으로 시작 전 종료 2였다. native compiler나 시험을 실행하기 전의 조정 오류로 보존했고, 임시 스크립트 문법을 바로잡은 뒤 실행했다. 프로젝트 실행기는 수정하지 않았다.

## 첫 고정 소스의 정식 검수

검증 소스 1dd1586에서 정식 bootstrap은 137.97초·573876KiB·종료 0이었다. 생성 C stage2=3=4는 db60d36fd1b43e240e56e5d8547e94f580d858c3f7190f2eca89a41bedc57935로 일치했다. 후보 C와 stage4 C도 byte 동일했다. seed 경로의 기존 native 37·거부 10·J_UTF16_LENGTH, HTTP20·취소20·저장소 API20·J1 실패18×100 모두 통과했다. 정식 compiler의 본체는 48.10초·1594056KiB였다.

기존 비TLS 관문 20개가 모두 종료 0이었다. CLI·Chrome 6개·L07·렉서/파서 메모리·JSON·과거 단계와 옛 러너를 포함한다. 전체 E08은 합성 TLS 키 생성 예외를 기다려 미실행이며 이 20개를 전체 회귀 완료로 바꾸어 쓰지 않는다. 시험 그룹 최고치는 1688752128바이트였다.

원래 B8 47개·편집 41개×3폭·사진 26개×3폭·단절16·API6도 모두 통과했다. 추가로 기존 조합 입력 HTML과 두 탭 충돌 HTML도 원문 그대로 실행해 통과했다. 각 단계의 기존 30초 단언은 유지했다. 충돌 HTML은 15초 저장 세 번과 마지막 16초 관찰이 필요하므로 최초 실행의 가상 시간 관찰창을 120000ms로 잡았고, 기존 단언 제한은 늘리지 않았다. 임시 관측 명령의 잘못된 stderr 검사기 경로로 종료 127이 한 번 있었으며, 완료된 컴파일/조합 입력 시험은 재실행하지 않고 원래 검사기 경로를 확인해 이어갔다.

방문 없는 예약 발행은 합성 데이터에서 실제 반례를 얻었다. auto_publish=true·기한 경과=true인 pending 글을 넣고, 검사 소유 HTTP 요청 0건인 채 61146ms 뒤 native WAL 조회 결과는 pending이었다. 첫 GET 뒤에는 published가 됐다. 원래 60초 이내 무방문 발행 기준은 실패다. 공개 글 응답은 200, canonical은 존재했고 og:image는 없었다. 관측 명령의 정상 종료를 P03 통과로 세지 않는다.

## 선택 영역·IME 선행 인수의 추가 고정 계약

G-BROWSER의 언어 자체 선택 영역 검증을 채우기 위해 browser-selection-read(element)와 browser-selection-set!(element,start,end[,direction])를 준비했다. 입력/textarea의 value 또는 Element의 textContent를 기준으로 UTF-16 위치를 사용한다. read는 start/end/text/direction을 반환하고 대상 밖 선택은 nil이다. 방향은 forward/backward, 접힌 선택의 read 방향은 none이다. 연결되지 않은 대상·잘못된 arity/대상·정수/범위/방향 오류는 분명한 오류다. 기존 DOM/이벤트/HTTP/취소/저장소 API와 기존 정답을 바꾸지 않는다.

적용 전 별도 임시 입력·HTML·20개 정답을 고정했다. 검증 논리는 .fl에 두며 HTML의 조합 이벤트 발생기는 합성 입력만 제공한다. 새 서버 JS나 수작업 JS 파일은 만들지 않는다. 입력 대상·역방향 textarea·UTF-16·접힌 입력·DOM 단일/중첩/역방향/접힘·빈 DOM·대상 밖·이전 선택 교체·본문 보존·잘못된 대상/arity/음수/소수/범위 초과/순서·방향/분리 대상·IME 이벤트 순서를 20개 고정 판정으로 삼는다. 기존 tests/ 파일·기대값·러너는 수정하지 않고 이 별도 수동 인수 입력은 작업 증거 영역 <가림>에 보존한다.

```text
program.fl=6cc37a2ab6ac4eb659984a9873fafc9432df0678af64adbbc2f58c9c1aa65c53
fixture.html=7fdce210fad2ed3df84330200ca8ff7c01134097e7f974839a48689a787d901b
expected=e47384b017e082c27e7a824bb430a400e15ef8abc7fc3b6f52643cd8f88b1af9
```

## 선택 영역·IME 구현과 실제 결과

self/browser-core.fl에 선택 영역 알고리즘·인자 검사·UTF-16 위치 변환을 구현했고 runtime/browser-target.h에는 필요한 native DOM 메서드 연결만 추가했다. 입력/textarea와 중첩 DOM·역방향·빈 대상·대상 밖 선택을 지원한다. 변경 뒤 read-back으로 적용 여부를 확인해 native 선택이 적용되지 않으면 오류를 낸다. .fl 원본과 기존 도구로 갱신한 C 내장 데이터를 모두 유지했고 BROWSER_CORE_EMBEDDING=PASS였다.

첫 검증 명령은 원래 시험 호스트에 등록되지 않은 경로로 요청해 404를 받아 정답을 실행하지 못했다. 경로를 원래 호스트의 루트로 바로잡았고, 완료한 compiler/host 빌드·JS 동일 재생성은 재실행하지 않았다. 동일한 고정 입력·정답으로 SELECTION_IME_PASS=20 FAIL=0, GENERATED_REPRODUCTION=PASS, BROWSER_OBSERVER=EMPTY를 얻었다. 기존 정답이나 제한시간을 바꾸지 않았다.

별도의 언어 저장소 프로그램은 같은 origin·같은 프로필에서 Chrome 프로세스를 완전히 닫았다 다시 열어 쓰기/다음 실행 읽기/삭제를 3/3 확인했다. 제품 초안의 영속 저장 활성화나 D04 정책 적용은 아니다. 합성 키와 값은 <가림>이며 검사 뒤 해당 시험 값은 지웠다.

```text
BROWSER_CORE_FL_SHA=6faa13f622e06e11feff12732b2c54cfe01ab9075f77540868b598819a021e6c
SELECTION_GENERATED_JS_SHA=e8fde9617be9f31f06c09dd287002b66e68341f0664be1422b63a6aaa7d58499
SELECTION_NATIVE_COMPILER_ELF_SHA=61fc1cb14928c959dab03663fd7c293ea98700dafbe4dfcefe19670912a7f753
STORAGE_REOPEN_WRITE=PASS
STORAGE_REOPEN_READ=PASS
STORAGE_REOPEN_REMOVE=PASS
```

API 구현은 FreeLang이며 새 서버 JS나 수작업 JS 런타임 파일은 없다. 검증 HTML에는 합성 CompositionEvent 발생기와 오류 관측용 inline 코드가 있고, 검증 판단은 .fl 프로그램이 수행했다. 기존 블로그 시각 편집기의 내장 JS는 원래 B8 시험에서 무수정 실행했다고 구분한다. 이후 이 브라우저 라이브러리 확장에 영향받는 원래 Chrome/API/제품 검수를 다시 실행했고 아래에 최종 결과를 기록한다.

## 최종 소스 재검증과 판정 범위

최종 검증 소스 HEAD=7270f5009b2e3c2bc07811d33662dabdca3735c6. 초기 선택 영역 증거 뒤 Range의 시작/끝 container도 대상 안인지 검사하는 방어를 추가했다. 위 6faa13f6/e8fde961/61fc1cb1 해시는 초기 구현 증거이며 최종 해시와 구분한다. 고정된 20개 입력/정답은 그대로 두고, 최종 소스에서 같은 20/20·observer 빈 값·동일 JS 재생성을 다시 확인했다.

```text
BROWSER_CORE_FL_SHA=d9ccd92c074c4b5c0ba08c29633f3575a8ad8fe5ab9ba9e76997ecd4a46e8d13
BROWSER_TARGET_H_SHA=9ceefe4bddd84e1232b46c48b1febc0a9d7860562c5a775ea90b48478ae99acc
BROWSER_CORE_DATA_H_SHA=3750462fe65091b7095f1740da24ad894e9627b13545ab9ba9a3e79786bf749f
SELECTION_GENERATED_JS_SHA=a1b815a43070264da2b2cc88c8da7b86d098e904c65325d37f4b5938437505bb
FINAL_NATIVE_COMPILER_ELF_SHA=3efdd665ab36a2cdb5824b096471623c1a0227cd3941445df031d3f32a02d49d
```

브라우저 생성 옵션은 --browser-js, 선택 영역 Chrome은 --headless --dump-dom --screenshot --window-size=1280,1000 --virtual-time-budget=30000이었다. 프로필은 새 합성 시험 프로필이다. 저장/재접속은 동일 origin·동일 프로필의 별도 Chrome 프로세스 두 번으로 3/3을 확인했다. 기존 DOM31·이벤트26·의미37·타이머20·타이머 예외·블로그 모델21과 기존 HTTP20·취소20·저장 API20, J1 실패18×100을 최종 소스에서 다시 통과했다. 서로 겹치는 native 37개 등을 합산해 고유 시험 총수를 만들지 않는다.

기존 저장소 API20 정답 SHA=70fb285b6777bf480a616bea547cf5bcb6c25fe99f55e826af1c489da61bc1f9다. 새 선택 영역20으로 기존 API20을 대체하지 않았다. 원래 미리보기375/768/1280의 스크린샷 세 장도 직접 확인했다. 제목·본문·사진이 표시됐지만 이것만으로 전체 탐색/접근성 인수를 통과시키지 않았다.

정식 seed→stage4는 137.68초·574008KiB·종료 0, 생성 C 세 개는 모두 다음 값이다.

```text
Stage2=db60d36fd1b43e240e56e5d8547e94f580d858c3f7190f2eca89a41bedc57935
Stage3=db60d36fd1b43e240e56e5d8547e94f580d858c3f7190f2eca89a41bedc57935
Stage4=db60d36fd1b43e240e56e5d8547e94f580d858c3f7190f2eca89a41bedc57935
SEED_SHA_AFTER_FINAL=f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c
```

B14와 다른 생성 C 해시는 self/cgc-main.fl의 reader·key 재사용·인자 문자열 생성 변경 때문이며, 새 stage2=3=4끼리는 byte 동일하다. 브라우저 선택 영역 추가 전/후의 compiler 생성 C도 동일했다. 고정점은 ELF가 아니라 생성 C로 판정했다. bootstrap은 strace 0·정적 금지 토큰 0이며 동적 실행 수는 UNOBSERVED다. 별도 ptrace 감사 범위에서는 NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0이었다. Chrome 내부 생성 JS 실행은 이 외부 실행 파일 수와 다르다.

최종 compiler의 실제 본체 생성은 47.48초·1592696KiB·종료 0이었다. 후보 표의 전체 펼친 입력과 원래 load 경로 본체를 구분한다. 8192바이트는 0.42초·86648KiB·종료 0이었다. 본체 생성 C는 메모리 수정 전과 byte 동일하며 J_UTF16_LENGTH=PASS다. 최종 영향 범위 검수 cgroup 최고치는 1741070336바이트였고, 본체 단일 프로세스 RSS 목표 1677722KiB와 별도 수치다.

선택 영역 추가 전의 최종 native 수정 단위 할당 계측은 51.48초·1594356KiB였고 출력 C byte 동일이었다. 해당 세 native 수정 파일은 1dd1586부터 최종 검증 소스7270f50까지 byte 동일하다. str arena 요청량 상위 세 곳은 다음과 같다. join 요청량은 별도이며 합쳐서 str 요청 순위를 바꾸지 않는다.

| 파일:줄 · 함수 | str 호출 수 | str arena 요청 bytes | join arena 요청 bytes |
| --- | --- | --- | --- |
| self/cgc-main.fl:1624 · cgc-if | 1235 | 4399922 | 0 |
| self/cgc-main.fl:2755 · cgc-eval-args | 23555 | 3933298 | 2999707 |
| self/cgc-main.fl:1019 · cgc-sexpr | 12466 | 3743648 | 0 |

## 원래 선행 인수와 실행 순서

원표 SHA=189514da35ecf929d0b9ec2eb918c2a0965363b109b1759c045470dd005e02bb. 이 JSON 및 그 의존 관계·인수 기준은 수정하지 않았다. 아래는 이번 실행 증거에 따른 파생 판정이며 원표를 재작성한 것이 아니다.

| 선행 인수 | 원래 요구 | 이번 근거와 남은 조건 |
| --- | --- | --- |
| G-AUTH | 실제 로그인과 hash/CSRF/만료/로그아웃 | 원표 기존 통과 유지, 합성 계정 API·비인가 미리보기 재확인 |
| G-STORE | DB 소유 native 저장소·원자성·재시작 | 원표 기존 통과 유지, 새 합성 DB 재시작·API·충돌 확인 |
| G-BROWSER | DOM/이벤트/선택/입력/HTTP/취소/저장·재접속·생성 근거 | 선택/IME20·재접속3과 원래 관문으로 남은 언어 자체 범위 충족. 합성 IME이며 OS 물리 입력 시험은 아님 |
| G-FILES | 15MiB/64MiB/4파일, HTTP/TLS, 정리·경로·hash | L07 등 실행, TLS 키 생성이 필요한 원래 시험은 승인 대기. 전체 완료 아님 |
| G-IMAGE | 형식·처리 출력·원본 hash·상한 | D05 미정. 원본 표시를 리사이즈/처리 통과로 대체하지 않음 |
| G-TIME | 타이머/취소, UTC/한국 시간/경계/모호 시각 | 타이머와 기존 clock 검사. 제품의 무방문 예약은 별도 실패 |
| G-NATIVE | 정확한 최신 소스 전체 회귀·앱·고정점·실행 감사 | 앱/고정점/허용 범위 회귀 완료 부분과 TLS 미실행을 구분 |

순서는 G-BROWSER→E01을 먼저 닫고, E02→E03 및 G-FILES/G-IMAGE→E07·미디어로 진행하는 것이다. D04는 E05/E06/P04, D02는 O01, D03/D06은 U01/댓글을 막는다. E01의 선행 하나를 닫았다고 E03/E07의 선행을 생략하지 않는다.

## PRD 24개 실행 관측과 파생 판정

24개 모두 해당 화면/API를 실제 요청했고, E01/E03/E05/E07 및 충돌·예약·공개 화면에는 아래 추가 동작을 실행했다. **24개 전체 인수 시나리오가 모두 실행 완료됐다는 뜻은 아니다.** 미정 정책·미구현 화면·선행 미완료 때문에 실행하지 못한 하위 기준도 적었다. HTTP 200은 전체 요구 통과가 아니다. 이 제한 때문에 B15의 ‘24개 모두 실제 인수 실행’ 조건은 아직 미달이다.

| 번호 | 요구사항 | 판정 | 실제 근거·실패/미실행 이유 |
| --- | --- | --- | --- |
| E01 | 시각적 문서 편집 | 통과 | 편집41×3폭, 저장/재열기·서식/미지원 원본 보존. 원래 완료2/2와 새 G-BROWSER 증거 충족 |
| E02 | 블록 조작 | 실패 | 편집 화면200·기본 블록은 동작. 모든 블록의 포인터/키보드 이동·복제·접근성·복잡 구조 전체 인수는 미완료 |
| E03 | 사진 삽입·배치·갤러리 | 실패 | 사진26×3폭 통과. 자체 기준2/2를 유지하지만 E02/G-FILES/G-IMAGE 선행 미완료 |
| E04 | 실행 취소·붙여넣기 | 실패 | 조합 입력 원래 HTML 통과. 블록/사진 전체 undo/redo·위험 붙여넣기 전체 인수 및 E02 미완료 |
| E05 | 자동저장·작성 복구 | 실패 | 원래 자동저장14·실제 서버 단절16·저장 API3·충돌 HTML 통과. D04 보관/용량/로그아웃 정책과 모든 상태·중복 보호 인수 미완료 |
| E07 | 비공개 미리보기 | 실패 | 375/768/1280 각11, 작성자 허용/비로그인·타인 거부3 통과. 자체2/2이나 E03 선행 미완료 |
| E06 | 전체 수정 경로 동시 보호 | 실패 | 두 탭 409와 양쪽 작성본 보존 HTML 통과. 기존/블록/AI/페이지 전체 경로와 E05 미완료 |
| P01 | 글 목록·일괄 관리 | 실패 | 목록200이나 없는 검색어 필터에도 합성 글1건이 남음. 일괄 작업 일부 실패 인수 미완료 |
| P02 | 발행 상태 전이 | 실패 | 화면200·합성 발행 글 공개200. 모든 상태/주소/시각·저장 실패 보존과 비공개 전체 경로 인수 미완료 |
| P03 | 방문 없는 예약 발행 | 실패 | 요청0건·61146ms 뒤 기한 경과 pending 유지. 첫 방문 후 published로만 전환. 60초 무방문 기준 실패 |
| P04 | 이력·비교·복원 | 실패 | 편집 화면200·기본 rollback 경로 존재. 두 수정 버전 비교·첫 버전 새 이력 복원 전체 인수와 D04/E06 미완료 |
| P05 | 주소·카테고리·태그 | 실패 | 조사한 분류 관리 경로404. 편집기 분류·태그 입력란은 존재한다. 중복/이전 주소 이동·분류 변경 격리 전체 인수 미완료 |
| M01 | 미디어 목록·재사용 | 실패 | 미디어 화면200이나 없는 검색어에도 합성 이미지1건이 남음. 날짜 필터·선행 E03/G-FILES 미완료 |
| M02 | 미디어 안전·사용처 | 실패 | 미디어 화면200·사진 재사용 부분 확인. 사용처/삭제 영향·전체 악성 경로 및 M01/G-FILES 미완료 |
| M03 | 이미지 처리·원본 보존 | 실패 | 사진 화면200·표시는 확인. 처리 출력 자체 검증·D05/G-IMAGE/M02 미완료 |
| S01 | 페이지·메뉴 | 실패 | 페이지/메뉴/설정 화면200. 공통 편집·메뉴 정렬·비공개 제외 전체 흐름과 P02 미완료 |
| S02 | 공개 화면·탐색 | 실패 | 공개375/768/1280 스크린샷/DOM. 분류·빈 검색·페이지 이동·키보드 전체 인수와 S01/P05 미완료 |
| S03 | 검색 노출·공유 | 실패 | 공개 글200·canonical 있음. 이번 글은 대표 이미지 없는 합성 글이므로 og:image 없음만으로 버그 확정 안 함. 전체 메타/설정/공개 경로와 S02/P02 미완료 |
| O01 | 일관 백업·격리 복구 | 실패 | 관리 화면200. 동일 시점 전체 객체·중단/내구성/불확실 commit 인수와 D02/M02/S01 미완료 |
| O02 | 운영 상태·진단 | 실패 | 상태 화면200. 예약 실패 기록·백업/대상/결과 전체 인수와 P03/O01 미완료 |
| U01 | 다중 작성자·역할 | 실패 | 조사한 사용자 관리 경로404이며 현재 main.fl에 해당 관리 route 등록 없음. 확정 권한표 D03/D06과 전체 직접 API 인수 미완료 |
| U02 | 계정·세션 관리 | 실패 | 조사한 계정 관리 경로404이며 관리 route 등록 없음. 비활성화/초기화 뒤 세션 무효화 전체 인수와 U01 미완료 |
| C01 | 댓글 승인·운영 | 실패 | 조사한 댓글 관리 경로404이며 관리 route 등록 없음. 댓글 상태 전이·비공개 보호 인수와 D06/U01/U02 미완료 |
| C02 | 댓글 남용 방지 | 실패 | 조사한 댓글 관리 경로404이며 관리 route 등록 없음. 등록/제한/중복/위험 입력 인수와 D06/C01 미완료 |

PRD_PASS=1/24. 원표의 기존 통과 개수를 직접 변경한 값이 아니라 이번 보고의 엄격한 파생 집계다. 목표 12/24에는 미달한다. 주요 원인은 (1) E02/G-IMAGE/G-FILES 선행 미완료, (2) D02/D03/D04/D05/D06 정책 미정, (3) 검색 필터·무방문 예약 실패와 분류/계정/댓글 필수 화면 미구현이다.

## B8 요약 재확인

| B8 보충 6 항목 | 결과 | 근거 |
| --- | --- | --- |
| 1 선행 관계와 순서 | 통과 | 원표 의존 관계와 이번 실행 순서 기록 |
| 2 E01·E03 완전 통과 | 미통과 | E01은 이번 통과, E03의 선행은 미완료 |
| 3 E05 단절·버전 보존·기존47 | 통과 | 최종 소스에서 실제 서버 정지/재시작16·API6·47개 유지 |
| 4 PRD 2개 이상·원격 일치 | 미통과 | PRD는1개. 원격 일치만으로 항목 전체를 통과시키지 않음 |

B8_SUMMARY=2/4. B12 재개에서 1/4로 내려간 원인은 본체를 새로 빌드하지 못해 항목3의 제품 시험을 최신 실행물로 재확인하지 못했기 때문이다. B14에서 2/4로 회복했고, 이번에도 최종 본체 새 빌드와 같은 시험으로 이를 유지했다.

언어 성과: map literal의 반복 전체 복사·lexer key 임시 할당·문자열 중복 복사를 줄였고, UTF-16 선택 영역 API를 FreeLang으로 구현했다. 일반 프로그램의 map literal과 브라우저 편집기에도 같은 효과를 제공하며 FL_TEXT·할당 실패 처리·원래 seed를 유지한다.


## 추가 native 관문 완료·누수·실행 감사

최종 소스7270f5009b2e3c2bc07811d33662dabdca3735c6의 추가 비TLS 관문은 27회 모두 종료0이었다. 서로 다른 구성은26개다. 임시 조정기의 FILE_HASH 첫 호출에도 sanitizer 환경을 전달해 두 해시 호출이 같은 sanitizer 구성이 됐다. 두 원문을 보존하되 서로 다른 regular/sanitizer 검증으로 세지 않는다. 기존20개 그룹 및 최종 영향 범위 검수와도 겹치므로 고유 시험 총수로 합산하지 않는다.

| 관문 | 원래 출력 또는 종료 |
| --- | --- |
| CLI 옵션 / 시계 | CLI_OPTION_GROUP_PASS=4 FAIL=0 / CLOCK_RANGE=PASS |
| platform / snapshot | L08_PLATFORM_PASS=5 FAIL=0 / L08_ATOM_SNAPSHOT_PASS=2 FAIL=0 |
| file hash | FILE_HASH_GROUP_PASS=8 FAIL=0, sanitizer 동일 구성2회 |
| tail / define / native spelling | L08_TAIL_PASS=11 FAIL=0 / L08_DEFINE_SEQUENCE_PASS=1 FAIL=0 / L08_NATIVE_SPELLING_SEQUENCE_PASS=1 FAIL=0 |
| concat / repair / extern | CONCAT_MEMORY_SELF_EXIT=0 / REPAIR_IO_GROUP_PASS=2 FAIL=0 / L08_EXTERN_SELF=PASS |
| atom update / lock / sort | 2/0 · 1/0 · 1/0, 각 누수 검사 통과 |
| HTTP atom / atom lifetime / arena cache | HTTP_ATOM_SELF=PASS · 2/0 · 1/0, 일반 LSan 통과 |
| native timer / sanitizer | L08_TIMER_PARTIAL_PASS=9 FAIL=0 / L08_TIMER_SANITIZER_PASS=5 FAIL=0 |
| FD 기본 / 장애·확장 | L08_FD_BASIC_PASS=11 FAIL=0 / L08_FD_EXTENDED_PASS=11 FAIL=0 |
| 인증 / HTTP / feed | L08_AUTH_SELF_PASS=6 FAIL=0 / L08_HTTP_PLATFORM_PASS=35 FAIL=0 / L08_FEED_PASS=9 FAIL=0 |
| 과거 HTTP / L06 재현 | L08_HTTP_PASS=7 / HTTP=200, FORM_MAP=PASS |

기존 E07의 나머지 누수 관문 구성인 atom2+lock1+sort1+lifetime2+arena1+timer5를 모두 실행해 12/12를 재확인했다. LSan을 끄지 않았고 sanitized 프로그램은 기존 native-lsan-trace.sh 방식대로 ptrace 밖에서 실행했다. build의 execve만 한 겹 strace로 기록했다. 기존 CLI 수명4/4, L07 221/0, 작은 누수2개, worker 정리와 과거 단계/옛 러너도 앞의 native 수정 단위에서 통과했다.

첫 추가 실행은 서비스 PATH에 rg가 없어 CLI AST 단언을 수행하지 못해 종료1이었다. 세 잘못된 옵션 정답은 통과했고 AST 출력도 생성됐으나 확인 도구가 없었다. 원문과 해당 종료 코드는 별도 보존했다. 임시 조정기의 PATH만 바로잡아 기존 스크립트·단언을 그대로 재실행했고 4/4 및 뒤의 모든 허용 구성이 통과했다. 이 환경 실패를 없었던 것으로 쓰거나 언어 회귀로 잘못 집계하지 않는다.

추가 검수 종료0, cgroup 최고치590131200바이트·swap0이었다. 추가 execve 감사 원문은 NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0이다. 종료 뒤 seed SHA=f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c로 시작과 같다. 물리 메모리는 전체를2147483648바이트 cgroup으로 제한했고, 직접 native compiler/GCC 실행의 주소 공간 제한과 별도로 sanitizer/Chrome의 큰 가상 주소 예약은 이 물리 상한 안에서 허용했다.

전체 E08 자체는 미실행이다. 원래 verify-l08-tcp-self.sh·verify-l08-http-client-self.sh·verify-l06-http.sh는 합성 TLS 키 파일을 만들므로 B15의 ‘키 파일 복사·생성 금지’에 대한 요청된 예외 승인이 오기 전에는 실행하지 않았다. 이 세 구성의 새 통과를 보고하거나 B14의 과거 통과로 대체하지 않는다. CDP 원래27개도 완료/통과 아님을 유지하며, 시간/문서 세대에 관한 언어 밖 원인 증거만 충족했다. 준비한 CDP readiness 수정은 러너 변경 예외 승인 전이라 적용하지 않았다.

## 최종 범위와 보류 사유

B15=보류. 본체 목표·고정점·UTF-16·속도·기존 API20·누수12와 B8 2/4는 충족했지만, PRD는 엄격한 파생 판정1/24로 목표12/24 미달이며 24개 전체 인수의 모든 하위 시나리오가 실행 완료된 것은 아니다. 전체 회귀의 TLS 세 구성도 미실행이다. 관측한 언어 관문의 새 회귀 실패는0이며 PRD 기능 실패와 환경 준비 실패를 별도로 보존한다.

고친 파일은 self/cgc-main.fl, runtime/math.c, runtime/v11-compat.h, self/browser-core.fl, runtime/browser-target.h, runtime/browser-core-data.h, docs/B15-CDP-PRD.md다. 원래 tests/·scripts/·bootstrap/ 및 PRD 원표는 변경하지 않았다. 고정 C seed 읽기 전용·생성 C 고정점·같은 정답·기존 제한시간을 유지했다. source-only 합성 DB만 사용했고 운영 DB/배포/실제 사이트 발행/외부 AI/메일/금지 서버/보호 프로젝트 작업은 하지 않았다. 이번에 띄운 로컬 프로세스는 종료됐고 지정된 두 포트의 LISTEN 수는0이었다.

커밋 전 전체 변경 diff의 민감정보를 확인했다. GitHub의 기존 fetch/push 목적지가 같은 지정 저장소인 것을 확인했고 해당 작업 가지는 원격에 없었다. Forgejo는 보류한다. 최종 문서 커밋 뒤 GitHub에 정상 푸시하고 원격 HEAD 일치·CLEAN을 별도로 확인해 최종 응답에 남긴다. 문서가 자기 자신의 최종 커밋 해시를 포함할 수 없으므로 아래 HEAD는 실행 검증 소스다.

```text
B15=보류
CDP_원인=시간: 문서 갱신 사이의 오래된 nodeId; 언어 밖
CDP_결과=원인 증명; 원래27개 미완료
PRD_PASS=1/24
본체_C생성=47.48초/1592696KiB
B8_SUMMARY=2/4
회귀=관측 언어 실패0; 전체TLS3구성 미실행
고정점=db60d36fd1b43e240e56e5d8547e94f580d858c3f7190f2eca89a41bedc57935 (Stage2=3=4)
J_UTF16_LENGTH=PASS
API=20/20 (기존 저장소 API)
속도=8192바이트 0.42초/86648KiB
SEED_SHA_시작=f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c
SEED_SHA_끝=f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c
HEAD=7270f5009b2e3c2bc07811d33662dabdca3735c6 (실행 검증 소스)
```
