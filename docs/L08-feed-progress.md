# C17 RSS·사이트맵 진행 보고

## 확인한 것

v11 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4`의 `src/stdlib-feed.ts`를 읽고 RSS/sitemap 고정 XML을 먼저 커밋했다. 공개 API가 없어 `FEED_BEFORE_BUILD_EXIT=1`인 것을 보존한 뒤 `.fl` 공개 연결과 C serializer를 구현했다. source TS는 텍스트로만 읽었으며 Node로 실행하지 않았다.

구현은 `7963a2df81ea2f352202aa8a878957b28a54caea`다. 기존 날짜·UTF-16/UTF-8·숫자 변환을 재사용했다. XML 다섯 문자 escape, 한글, 입력 항목 순서, 빈 목록·nil·기본 language, ISO +09:00·숫자 밀리초·invalid date, 숫자 priority와 선택 필드를 검사했다. 날짜 formatter는 locale과 무관하게 GMT 영어 형식을 출력한다. CDATA의 닫는 경계를 분할해 원문을 보존하는 보호를 정답 커밋부터 명시했다.

최초 구현 빌드에서는 UTF-16 배열 helper와 byte buffer helper의 형식을 잘못 연결해 C 컴파일이 실패했다. [실패 원문](evidence/l08-platform/feed-initial-build-failure.txt)을 남겼으며, 정답을 바꾸지 않고 기존 UTF-8 encoder를 올바르게 호출하도록 고쳤다.

```bash
CPATH=<가림> LIBRARY_PATH=<가림> L08_FEED_OUT=<가림> strace -f -e trace=execve -o <가림> bash scripts/verify-l08-feed-self.sh
bash scripts/audit-l07-execve.sh <가림>
```

```text
FEED_UTC_PROGRAM_EXIT=0
FEED_Asia-Seoul_PROGRAM_EXIT=0
RSS_FULL_XML=PASS
RSS_EMPTY_XML=PASS
SITEMAP_FULL_XML=PASS
SITEMAP_EMPTY_XML=PASS
CDATA_CONTENT_PRESERVED=PASS
FEED_CLOCK_PROGRAM_EXIT=0
FEED_CLOCK_XML=PASS
L08_FEED_PASS=9 FAIL=0
FEED_SELF_EXIT=0
NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0
```

[수정 전](evidence/l08-platform/feed-before.txt), [수정 후 원문](evidence/l08-platform/feed-after.txt), [고정 계약](../tests/l08-platform/feed-contract.md). 독립 C 프로그램 xmllint/libxml `21205`는 시험에서만 실행하며 runtime 의존성에 추가하지 않았다. 네 개 XML과 기본 시계 출력이 파싱되며, CDATA XPath 내용은 `before]]>after` 그대로다. URI는 source처럼 조합하며 임의 정렬·주소 재작성은 하지 않는다.

## 전체 블로그 컴파일 재확인

운영 데이터·설정을 넣지 않은 source 복사본의 전체 `app/main.fl` C 변환을 다시 수행했다. `BLOG_TRANSLATE_EXIT=0`, `BLOG_C_SYNTAX_EXIT=1`이다. API 미연결 오류는 아래 `8`개이며 HTTP·RSS·sitemap 이름은 목록에서 없어졌다.

```text
fl_yield
set_timeout
tcp_drop
tcp_write
tcp_server_tls
tcp_server_raw
tcp_server_stop
auth_verify_password
```

[전체 C 구문 검사 출력](evidence/l08-platform/blog-after-http-feed-compile.txt). 이는 링크·실행 성공이 아니며 실행 파일을 만들거나 운영 상태를 읽지 않았다. 기존 내장 editor JS는 source에 남아 있고 이번에 생성·수정·실행하지 않았다. 브라우저 J안 검증은 별도다.

## 미확인

이 코드의 전체 회귀·bootstrap은 아직 미확인이다. C14 전체 회귀는 앞선 코드의 별도 실행이므로 이 코드의 통과로 대체하지 않는다. 전체 driver에 feed 자체 단계를 추가했다. 형식 계약은 배열/nil 입력, 기존 C04 ISO 날짜 범위와 source optional 필드 사용을 대상으로 하며 모든 Node Date 문자열 형식의 일치를 주장하지 않는다.

앱 RSS/sitemap 실제 HTTP·비공개 글 제외·재시작·화면·운영 데이터 호환·백업/복구·댓글·다중 작성자는 여전히 별도 검증이 필요하다. 서버 23·25 접속·운영 데이터 변경·합치기·배포는 하지 않았다.

## 작업 가지 반영

분리 checkout 구현 `7963a2df81ea2f352202aa8a878957b28a54caea`를 작업 가지 구현 `bdf0ab310b7e1f21bb216f3a013d228458103e23`로 반영했다. 이후 client 변경까지 반영한 작업 가지와 최신 전체 시험 checkout의 runtime/self/scripts/tests diff가 비어 있음을 확인했다. 앞선 시험 소스를 수정하지 않고 별도 worktree에서 반영했다.
