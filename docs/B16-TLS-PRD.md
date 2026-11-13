# B16 TLS·인증·저장·화면 재검수

B16=보류. 기존 CDP 27개는 러너 수정 없이 완주했고 TLS 세 구성, 기본 인증과 저장 흐름을 실제 확인했다. 엄격한 PRD 파생 집계는 1/24로 목표 6/24 미달이다. 전체 회귀 최종 결과: 원래 E08 상위3묶음 통과3/실패0/미실행0, E08_COMPLETION=PASS.

## 출발과 변경 범위

FX 출발=d232a095e641ee009e037212fe0734b82491616b, blog1 출발=4a63271cf1c926ca7f3ff80c8877a538941719e9. 시작 때 두 작업 트리는 CLEAN이었다. 두 저장소에 각각 fix/b16-tls-prd 작업 가지를 분리했다. 원래 블로그 작업 트리는 변경하지 않았다.

실제 제품 수정은 blog1 app/themes/basic/style.fl의 .editor-topbar에서 position:sticky를 position:relative로 바꾼 한 단어다. 저장 막대가 편집 도구를 가려 실제 포인터 클릭을 막던 현상을 없앴다. 파일 SHA=e2b7aed56c8cae9f11e3a247feea2adf951dfa783a493218c12c34214f021b81, BLOG_HEAD=dcc9e1a6a955874bc7cc0c4258caf085f3d4f89b다. FX에서는 이 보고서만 추가하며 컴파일러·브라우저 언어 코어·원래 tests/scripts/bootstrap·정답·제한시간·seed는 수정하지 않았다.

실제 경로·소유자·주소·합성 인증값은 <가림>이다. 일반 산출물은 작업 전용 저장 영역, 무거운 시험은 ps 확인 뒤 공용 flock·2147483648바이트 cgroup·swap0·core0 아래 순차 실행했다. 별도 본체 생성·직접 native 빌드에는 주소 공간 2097152KiB상한을 추가했고 sanitizer/Chrome의 가상 주소 예약은 같은 물리 상한 안에서 허용했다. 다른 무거운 작업의 잠금을 강제로 풀지 않았다.

SEED_SHA_BEFORE=f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c

## TLS 세 구성과 키 삭제

| 원래 검사 | 실제 원문 | 판정 |
| --- | --- | --- |
| verify-l08-tcp-self.sh | L08_TCP_PASS=8 FAIL=0 | raw/TLS/mTLS, 신뢰·호스트·부분 쓰기·재시작·오류 큐 통과 |
| verify-l08-http-client-self.sh | L08_HTTP_CLIENT_PASS=6 FAIL=0 | raw 경계와 비신뢰/신뢰 TLS·호스트 이름 검사 통과 |
| verify-l06-http.sh | L06_TLS_PASS=110 FAIL=0, L06_HTTP_TLS=PASS | 원래 HTTP·업로드·복구·TLS 구성 전체 실행 |

시험용 키·인증서는 매 구성마다 새로 만든 임시 디렉터리 <가림>에만 생성했다. 관리자 권한 없이 사용자·마운트·PID 네임스페이스에서 일반 출력 디렉터리를 연결하고, 키/인증서 위치만 실제 임시 디렉터리의 별도 파일/디렉터리로 연결했다. 따라서 기존 TLS 파일 격리 경계와 러너를 바꾸지 않으면서 일반 산출물은 실제 작업 저장 영역에, 실제 키 바이트는 사용자 지정 임시 영역에 두었다. 연결 대상의 device/inode 대조와 작업 영역의 빈 자리표시자 크기0을 먼저 확인했다. 이는 키 복사가 아니다.

각 TLS 검사 프로세스가 끝난 뒤 해당 PID 네임스페이스의 자식이 정리된 것을 전제로 키·인증서·CSR·발급 부수 자료를 즉시 삭제했다. 작업 영역의0바이트 자리표시자도 삭제했다. 세 구성 각각의 삭제 원문은 같다.

```text
POST_DELETE_LS_EXIT=0
POST_DELETE_LS_OUTPUT=EMPTY
TMP_ROOT_EXISTS=0
TASK_KEY_BYTES=0
KEY_FILE_COPY=0
```

첫 TLS 실행 전체 종료0, cgroup 최고치399605760바이트였다. 실제 서비스/SSH/기존 개인키는 사용하지 않았다. 원래 전체 회귀 안의 추가 TLS 실행·삭제 결과는 TCP/HTTP_CLIENT/L06 각각 종료0. 세 기록 모두 POST_DELETE_LS_EXIT=0, POST_DELETE_LS_OUTPUT=EMPTY, TMP_MATERIAL_EXISTS=0, TASK_KEY_BYTES=0, KEY_FILE_COPY=0이고 삭제 후 ls 출력 크기는 각각0바이트였다. FULL_UNIT_POST_CLEANUP=PASS, 남은 소유 임시 TLS 디렉터리0이다. 정상/비정상 종료 뒤에도 소유한 임시 디렉터리만 정리하는 종료 처리를 두었다.

저장소 Git 검색 최종 결과: FX/blog1 각각 추적 키 파일명0, 개인키 시작 표지 일치0(검색 종료1은 불일치 의미). 검색은 추적 파일명과 private-key 시작 표지의 존재 여부이며 키 본문을 출력하지 않았다.

## CDP 27개: 원인 분리와 FreeLang 뷰 수정

B15에서 관측한 오래된 Inspector nodeId는 C 검사기 소유였다. 브라우저 언어 코어의 document.getElementById 조회와 C의 CDP nodeId는 다른 객체다. 브라우저 코어를 고쳤다고 가장하거나 JSON 응답을 바꾸지 않았다.

이번 수정 전 원래 검사에서는12개 단언까지 실행돼10개 통과·2개 실패 후 중단됐다. 실패 단언은375의 PREVIEW_IMAGE_VISIBLE,768의 VISUAL_READY였고, 마지막 원문은 다음과 같다.

```text
EDITOR_UI_CDP_METHOD=DOM.scrollIntoViewIfNeeded CODE=-32000
EDITOR_UI_CDP_MESSAGE=Node does not have a layout object
EDITOR_UI_CDP_FAILURE=protocol_error
EDITOR_UI_CLIENT_EXIT=1
```

375 스크린샷에서는 저장 막대가 결과 보기 버튼 위를 덮어 실제 포인터가 의도한 버튼을 누르지 못하는 현상을 확인했다. 프로그램으로 button.click을 부르는 기존 B8 검사의 통과만으로 이 포인터 문제를 발견할 수 없었다. FreeLang 뷰 스타일 한 단어를 고쳐 막대를 문서 흐름에 두었다. 문서/가시성 전환 중의 프로토콜 오류와 실제 버튼 겹침을 같은 언어 코어 버그로 단정하지 않는다.

수정 뒤 원래 C 검사기·원래27개 정답·15초 응답 제한·120초 실행 제한을 유지했다. 메시지 id/method/node/event만 관찰한 실행에서27/27, 이어 관찰기를 제거한 정상 실행에서도27/27이었다. 세 너비375/768/1440의 실제 스크린샷을 남겼고375 수정 전/후 화면을 직접 확인했다. 최종 정상 실행 원문은 다음과 같다.

```text
EDITOR_UI_PASS=27 FAIL=0
EDITOR_UI_DOCUMENT_RETRIES=3
EDITOR_UI_OWN_DRAFT_DIALOGS=2
EDITOR_UI_INJECTED_JS=0
EDITOR_UI_CLIENT_EXIT=0
EDITOR_UI_SUITE=PASS
LD_PRELOAD_OBSERVER=0
```

3회의 문서 루트 재조회는 원래 C 검사기에 있던 동작이다. 새로 nodeId 캐시를 구현한 것이 아니다. 준비했던 C readiness 변경은 적용하지 않았고 예외 요청도 철회했다. 원래 코드·정답 SHA는 수정 전/후 동일했다.

```text
EDITOR_UI_CDP_C_SHA=72fb23092b70614bf083e29f685609991ec04d298783f4cf53d433b7ce756661
EDITOR_UI_EXPECTED_SHA=019aecc9edc52d0edf6501657542b7dad81be7d4dab91c9a7c364e87278b25c8
```

CDP27은 합성 세션으로 화면을 검사했으며 비밀번호 로그인 증거로 쓰지 않는다. 실제 비밀번호 로그인은 다음의 별도 G-AUTH 시험이다. 기존 브라우저 JS는 원래대로 실행됐고 새 손작성 JS 파일이나 JS 주입은 없다.

## G-AUTH → G-STORE 실제 재확인

G-AUTH는 원래 login-ui.sh로 실제 비밀번호 폼 제출·오답·정상 세션·cookie 속성·로그아웃·폐기 cookie 거부를375/768/1440에서21/21 확인했다. native HTTP의 CSRF/비인가/오답/만료 거부8/8과 독립 reader2/2도 통과했다. Runtime 예외0, 정상 로그인 cookie 주입0이었다. 모두 새 합성 계정/자료이고 인증 처리 소스는 이번 편집기 CSS 수정에서 바뀌지 않았다.

G-STORE는 원래 native-store-adoption.sh의 공유 guard·긴 WAL·별도 reader·stage1 반복·index·tail/forensic·WAL 실패 경로가 기존 정답과 일치했다. 긴 WAL은1000개 글을 독립 거래로 저장하고 별도 과정에서 두 번 읽었다.

첫 상위 실행은 마지막 concurrent-http 하위 검사에서 종료1이었다. 제가 전달한 FL_HTTP_LOOPBACK_ONLY 때문에 시작 정보의 주소 표기가 바뀌었고, 원래 stderr 단언은 다른 표기만 허용했다. 데이터 단언 전의 환경 충돌이었다. 원래 검사에는 실제 bind를 loopback으로 강제하는 http-loopback.c가 이미 연결돼 있었다.

완료한 저장 하위 검사는 반복하지 않았다. 실패한 하위 검사만 새 자료에서 환경 변수 하나를 제거해 원래 스크립트/정답 그대로 재실행했다.32개 동시 쓰기·중복409·WAL 무변경·두 재시작이 모두 통과했다. execve/bind/listen 기록과 원래 래퍼 심볼로 실제 서버 bind3건 모두 사용자 지정 loopback임을 확인했다. 로그 수준이나 허용 정답을 바꾸지 않았다.

```text
LOGIN_UI_PASS=21 FAIL=0
LOGIN_HTTP_PASS=8 FAIL=0
LOGIN_READER_PASS=2 FAIL=0
STORE_HTTP_FIXED_ENV_EXIT=0
ACTUAL_SERVER_BINDS=3
ACTUAL_BIND_LOOPBACK_ONLY=PASS
ORIGINAL_STDERR_ORACLE=UNCHANGED
```

따라서 G-AUTH와 G-STORE 최소 흐름 및 실행한 기존 구성은 통과다. 첫 상위 실행의 종료1 원문은 보존하고, 상위 전체 러너가 새로 종료0이었다고 바꿔 쓰지 않는다. 이 두 관문은 원표에서 이미 passed였으므로 재통과로 제품24개의 분자가 늘어나지 않는다.

## 실제 제품 검사와 PRD 24개

원표 SHA=189514da35ecf929d0b9ec2eb918c2a0965363b109b1759c045470dd005e02bb. 원표/의존 관계/인수 기준은 수정하지 않았다. 아래는 실제 증거에 따른 파생 판정이다. 모든 항목의 화면/API를 요청했고 관련 기존 B8 시험도 재실행했다. 모든 하위 인수 시나리오가 실행 완료됐다는 뜻은 아니다. 미실행/불충족 하위 조건을 통과로 세지 않는다. 아래 실패는 인수 전체 미충족을 뜻하며, 실제 오류·미구현·미실행의 이유를 구분했다.

수정본에서 기존47개, 편집41개×3폭, 사진26개×3폭, 실제 서버 정지·같은 포트 재시작·단절 입력/복구 전 버전 보존16개, 자동저장3·미리보기 권한3개가 모두 통과했다. 조합 입력·두 탭 충돌 추가 관찰은 둘 다 PASS였다. 조합 중 요청0/서버 초안0, 확정 뒤 요청1/부분 값0을 확인했다. 두 탭 충돌에서는 오래된 요청 거부·로컬/서버 내용과 revision 보존·성공 오표시0을 확인했다. 기존 HTML의 단계별30초 단언은 유지했다. 시험 페이지는 원래 제품/배포 묶음에서 제외되는 검사를 유지했고 실제 TEST_ENTRY_IN_PRODUCT=ABSENT였다. 공개 화면375/768/1280도 새 자료로 확인했고, 공개 글 응답200·canonical 존재를 관찰했다.

| 번호 | 요구사항 | 판정 | 이번 근거와 남은 조건 |
| --- | --- | --- | --- |
| E01 | 시각적 문서 편집 | 통과 | 편집41×3·저장/재열기/원본 보존, 원래 G-BROWSER 근거 유지, 실제 포인터27도 통과 |
| E02 | 블록 조작 | 실패(인수 미완료) | 화면200. 전체 블록·새 화면 재편집·복잡 구조·모바일 입력/접근성 전체 인수 미실행 |
| E03 | 사진 삽입·배치·갤러리 | 실패(인수 미완료) | 사진26×3·실제 포인터 화면 통과. E02/G-IMAGE 선행 미완료 |
| E04 | 실행 취소·붙여넣기 | 실패(인수 미완료) | 화면200. 텍스트/블록/사진 전체 undo/redo·위험 붙여넣기와 E02 인수 미완료 |
| E05 | 자동저장·작성 복구 | 실패(인수 미완료) | 자동저장14·실제 단절16·저장API3 통과. D04와 모든 상태/중복 재전송 인수 미완료 |
| E07 | 비공개 미리보기 | 실패(인수 미완료) | 세 폭 각11·권한3 통과. E03 선행 미완료 |
| E06 | 전체 수정 경로 동시 보호 | 실패(인수 미완료) | 화면200. 기존/블록/AI/페이지 모든 경로의 조건부 저장·양쪽 보존과 E05 미완료 |
| P01 | 글 목록·일괄 관리 | 실패 | 없는 검색어 필터에도 합성 글1개가 남음. 일괄 일부 실패 보존 인수도 미완료 |
| P02 | 발행 상태 전이 | 실패(인수 미완료) | 관리 화면200. 모든 상태·주소·시각 전이/실패 보존·비공개 전체 경로 인수 미실행 |
| P03 | 방문 없는 예약 발행 | 실패 | 자체 HTTP 요청0 상태로61143ms 관찰해도 pending·auto=true·due=true. 첫 방문 뒤 published. 방문 없는60초 내 발행 조건 미충족 |
| P04 | 이력·비교·복원 | 실패(인수 미완료) | 화면200·기본 rollback 경로. 버전 비교/새 이력 복원·D04/E06 미완료 |
| P05 | 주소·카테고리·태그 | 실패(인수 미완료) | 조사한 관리 경로404. 편집기의 분류·태그 입력과 주소 검증은 존재하며 변경/정리 영향 전체 인수 미완료 |
| M01 | 미디어 목록·재사용 | 실패 | 없는 검색어에도 합성 이미지1개가 남음. 필터·전체 재사용과 E03 인수 미완료 |
| M02 | 미디어 안전·사용처 | 실패(인수 미완료) | 화면200·사진 부분 흐름. 사용처/삭제 영향·전체 악성 입력 및 M01 인수 미완료 |
| M03 | 이미지 처리·원본 보존 | 실패(인수 미완료) | 원본 표시는 확인. 검증된 처리 출력과 D05/G-IMAGE/M02 미완료 |
| S01 | 페이지·메뉴 | 실패(인수 미완료) | 페이지/메뉴/설정200. 공통 편집·정렬·비공개 제외 전체 흐름/P02 미완료 |
| S02 | 공개 화면·탐색 | 실패(인수 미완료) | 기본 화면/크기 관찰만 완료. 전체 탐색·키보드·필터와 S01/P05 미완료 |
| S03 | 검색 노출·공유 | 실패(인수 미완료) | 전체 메타/설정/공개 경로와 S02/P02 미완료. 대표 이미지 없는 글의 og:image 부재만으로 버그를 확정하지 않음 |
| O01 | 일관 백업·격리 복구 | 실패(인수 미완료) | 관리 화면200. 동일 시점 전체 객체·중단/내구성과 D02/M02/S01 미완료 |
| O02 | 운영 상태·진단 | 실패(인수 미완료) | 관리 화면200. 예약 실패/백업·원인/대상/결과 전체 인수 및 P03/O01 미완료 |
| U01 | 다중 작성자·역할 | 실패(미구현·결정 필요) | 조사 경로404·현재 관리 route 없음. D03/D06 확정 권한표 필요 |
| U02 | 계정·세션 관리 | 실패(미구현) | 조사 경로404·관리 route 없음. 비활성화/초기화 후 세션 무효화와 U01 미완료 |
| C01 | 댓글 승인·운영 | 실패(미구현·결정 필요) | 조사 경로404·관리 route 없음. 상태 전이·비공개 보호와 D06/U01/U02 미완료 |
| C02 | 댓글 남용 방지 | 실패(미구현·결정 필요) | 조사 경로404·관리 route 없음. 등록/제한/중복/위험 입력과 D06/C01 미완료 |

PRD_PASS=1/24다. 주요 미통과 원인은 (1) 블록·이미지 선행과 제품 기능 부족, (2) 보관·권한 등 미정 정책, (3) 실제 검색 필터·예약·관리 화면 미완료다. 화면200이나 G-AUTH/G-STORE 재통과를 새 제품 통과로 환산하지 않았다. B8은 선행 목록·단절/기존47개를 유지해 B8_SUMMARY=2/4이고 E01+E03 쌍 및 PRD2개 이상 조건은 미달이다.

## 결정 필요와 나머지 선행의 남은 일

- D04: 초안 기간, 이력 개수/기간, 계정별 용량, 로그아웃 때 브라우저 초안 삭제 여부.
- D03: 작성자 자신의 글 직접 발행 또는 검토 경유.
- D06: 작성자 미디어 공유/수정 경계와 댓글 필드·제한·보관.
- D02: 복구 commit 결과 불확실 상태를 제품 인수에서 인정하는 계약.

값을 가정해 자동 삭제·새 한도·권한을 적용하지 않았다. D05 이미지 의존성/GIF/pixel/예산도 미정이며 이번 구현 범위 밖이다.

| 나머지 선행 | 남은 일 목록 |
| --- | --- |
| G-BROWSER | 기존 선택/IME/재접속 언어 증거와 이번 API·화면 결과의 최종 소스 SHA 연결. 새 언어 코어 구현은 하지 않음 |
| G-FILES | 최신 HTTP/TLS·L07·hash 결과를 원래15MiB/64MiB/4파일·정리/경로/바이너리 항목별로 연결 |
| G-IMAGE | D05 확정 및 원본 byte/hash를 보존하는 처리 출력·상한 자체 인수 |
| G-TIME | UTC/한국 시간·경계·모호/없는 시각 계약과 방문 없는 예약 작업 인수 |
| G-NATIVE | 원래 전체 러너의 최종 소스·원문·실행 감사·고정점·원격 HEAD 연결 |

## 본체 메모리·정식 빌드·API·누수·전체 회귀

| 대상 | 초 | 최대 RSS KiB | 종료 |
| --- | --- | --- | --- |
| B16 수정 전 본체 | 47.61 | 1595292 | 0 |
| FreeLang 뷰 수정 후 본체 | 47.67 | 1595912 | 0 |
| 8192바이트 입력 | 0.42 | 86628 | 0 |
| 첫 정식 bootstrap | 139.48 | 574016 | 0 |

본체1595912KiB는 목표1677722KiB 이하다. 원래8192바이트 생성 C는 B15와 byte 동일이다. J_UTF16_LENGTH, 기존 저장소 API20/20, HTTP20/취소20, J1 실패18×100, 원래 누수12/12가 통과했다. 기존 저장소20개의 정답 SHA는70fb285b6777bf480a616bea547cf5bcb6c25fe99f55e826af1c489da61bc1f9로 유지했고 다른 새 시험으로 대체하지 않았다.

원래 전체 회귀의 첫 실행은 제가 격리를 전체에 적용해 Chrome이 사용자 네임스페이스의 UID0으로 실행되면서 시작을 거부해 종료1이었다. 작은 누수·worker·CLI는 통과했고 Chrome 타이머의 브라우저 단언은 실행되지 않았다. 원문을 보존했다. 수정 후 일반 Chrome은 원래 사용자 환경과 샌드박스를 유지하고 세 native TLS 검사만 필요한 파일/PID 격리에 들어간다. 원래 scripts/verify-e08-completion.sh와 모든 하위 정답·제한은 그대로다.

최종 전체 회귀 원문:

```text
SMALL_LEAKS_EXIT=0
WORKER_EXIT=0
FULL_EXIT=0
E08_COMPLETION=PASS
NATIVE_ALL_EXIT=0
E07_COMPLETION=PASS
REMAINING_LEAK_CASES=12/12
L07_SELF_PASS=221
L07_SELF_FAIL=0
L01_BROWSER_TIMER_PASS=20 FAIL=0
결과: 10 PASS / 0 FAIL / 10 총
```

상위3묶음을 각각 통과/실패/미실행으로 집계하면3/0/0이다. 이는 서로 겹치는 모든 하위 단언을 합산한 수가 아니다. CLI·Chrome6묶음·CLI options·clock·L07·native 전체 및 원래 실행 감사가 모두 종료0이었다.
새 실행의 source/runtime SHA, 최고치, 실행 감사 및 bootstrap 결과: 실행 FX_HEAD=d232a095e641ee009e037212fe0734b82491616b, BLOG_HEAD=dcc9e1a6a955874bc7cc0c4258caf085f3d4f89b. runtime/self/scripts/tests/bootstrap은 FX 출발점과 diff0. 전체 회귀 cgroup 최고치1508384768바이트, NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0. bootstrap의 RESIDUAL_PROCESSES=0, CLEAN_CHECKOUT=PASS, BOOTSTRAP_STATIC_AUDIT=PASS였다.

추가 PRD 관찰 조정 스크립트의 첫 실행도 PATH에 rg가 없어 종료127이었다. 완료된 두 native helper 빌드는 보존하고 PATH와 사전 확인만 보완해 재사용했다. 도구가 없던 첫 실행의 제외 검사 표시는 미검증으로 따로 보존하며 인수 증거로 쓰지 않았다. 원래 프로젝트 러너/기대값은 변경하지 않았다.

최종 생성 C 고정점은 전체 원래 회귀의 마지막 bootstrap에서도 stage2=3=4=db60d36fd1b43e240e56e5d8547e94f580d858c3f7190f2eca89a41bedc57935로 일치했다. B15/이번 최초 빌드와 동일하다. 최초 값은 다음과 같고 컴파일러/seed에는 B16 코드 변경이 없다.

```text
Stage2=db60d36fd1b43e240e56e5d8547e94f580d858c3f7190f2eca89a41bedc57935
Stage3=db60d36fd1b43e240e56e5d8547e94f580d858c3f7190f2eca89a41bedc57935
Stage4=db60d36fd1b43e240e56e5d8547e94f580d858c3f7190f2eca89a41bedc57935
```

bootstrap은 strace0·정적 감사이며 동적 실행 수는 UNOBSERVED로 구분한다. 별도 execve 감사 범위는 NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0이었다. Chrome 안의 생성/기존 JS 실행과 외부 JS 실행 파일 수를 혼동하지 않는다.

SEED_SHA_AFTER=f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c

## 최종 범위와 푸시 확인

FX 변경 파일은 docs/B16-TLS-PRD.md, blog1 변경 파일은 app/themes/basic/style.fl이다. 코드 수정 뒤 원래 CDP27·B8은 통과했지만 PRD6개 이상 목표에는 미달한다. 보관/권한 결정이나 새 제품 기능을 임의로 완료 처리하지 않아 B16=보류다.

언어 성과: FreeLang 앱과 native 런타임으로 TLS·기본 로그인·저장이 검증됐고 실제 포인터 사용을 막던 FreeLang 뷰를 고쳤다. 쇼핑몰 공통 기반으로 재사용할 수 있지만 주문·결제·다중 역할의 인수는 미실행이므로 쇼핑몰 운영 준비가 끝났다고 판정하지 않는다.

실행 검증 FX_HEAD=d232a095e641ee009e037212fe0734b82491616b, BLOG_HEAD=dcc9e1a6a955874bc7cc0c4258caf085f3d4f89b. 최종 FX 문서 커밋 HEAD는 커밋 뒤 정상 GitHub 푸시·원격 일치·CLEAN을 확인해 최종 응답에 남긴다. 두 저장소의 기존 GitHub 목적지를 확인했고 원격을 추가하지 않았다. Forgejo와 운영 배포는 보류한다.

최종 로컬 프로세스/지정 포트/키 검색/작업 트리 검증: 검사 종료 때 실행 중인 B16 서비스0, 지정 두 포트 listener0, 소유 TLS 임시 디렉터리0, 두 저장소의 추적 키 파일명/개인키 표지0. 원래 블로그 작업 트리는4a63271cf1c926ca7f3ff80c8877a538941719e9/CLEAN 그대로다. 문서 커밋·GitHub 원격 HEAD/CLEAN 검증은 최종 응답에 실제 해시와 함께 기록한다.

B16=보류, B8_SUMMARY=2/4, PRD_PASS=1/24
