# B15 CDP·PRD·본체 메모리: 수정 전 증거

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
