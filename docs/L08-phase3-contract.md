# L08 3차 고정 계약

기준 fx `2828966042d56bcd7e083decdae40bd425d5177b`, v11 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4`. 가지는 feat/l08-c04-c07-c10이다. 이번 승인 범위는 C04·C07·C10만이며 서버 23·25 접속·작업·합치기·배포·강제 푸시를 금지한다. 앱과 운영 데이터는 변경하지 않는다.

- [v11 날짜 원문](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/src/stdlib-time.ts): date_parse는 Date.parse의 ms, 오류는 `date_parse: invalid date string "..."`. now_ms는 epoch ms. format_date는 프로세스 로컬 시간이며 각 토큰 YYYY/MM/DD/HH/mm/ss/SSS를 순서대로 첫 1회만 치환한다. 날짜만 있는 ISO는 UTC, offset 없는 ISO 시각은 TZ의 로컬 시간이다. UTC·Asia/Seoul·America/New_York의 DST와 음수 ms를 고정한다. 이번 입력 계약은 ISO 날짜·시각이며 비표준 RFC/지역별 Date.parse 입력의 완전한 에뮬레이션은 주장하지 않는다.
- [v11 정규식 원문](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/bootstrap.js#L28041): regex_*는 문자열 먼저·패턴 다음, replace는 global, replace_first는 첫 1회, 캡처/치환 기호·빈 매치·무매치·UTF-16 단위를 고정한다. 실제 패턴 전수 목록은 별도 문서에 먼저 고정한다. C 자체 엔진을 작성하고 기존 POSIX C ABI는 유지한다. 외부 정규식 라이브러리를 도입할 경우 사용자 승인 전에 커밋하지 않는다.
- [v11 파일 원문](https://github.com/kimjindol2025/freelang-v11/blob/b49cc9194a00649b8cf58e1d6e8ee38ffce770d4/src/stdlib-file.ts): file_read는 UTF-8 대체 디코딩 텍스트, file_write/file_append는 UTF-8 텍스트, file_copy는 원본 bytes 복사, 부모 자동 생성, delete 누락 false, 작업 실패 예외, 목록 순서는 보장하지 않는다. PNG/JPEG/GIF 원본은 copy와 L07 raw API로 cmp·SHA-256 확인한다. 텍스트를 다시 encode해서 원본 이미지라고 하지 않는다.
- **API 공존:** v11의 underscore 파일 이름은 텍스트/복사·안전 경로 API로 추가한다. L07의 kebab-case file-read/file-write 및 checked result API와 _fl_file_*는 기존 raw byte 계약 그대로다. 같은 이름으로 덮어쓰지 않는다. 그 외 이름 해석 규칙은 바꾸지 않는다.
- **승인된 경로 강화:** C10에서는 FL_FILE_BASE 경계·입력 NUL·`..`·경로 요소 및 최종 심볼릭 링크를 차단한다. 읽기·쓰기·copy 양쪽·delete·목록·메타데이터 모두 fd 기준으로 검사해 check/open 사이 이탈을 막는다. v11의 copy·exists·directory 검사 누락과 symlink 허용을 그대로 재현하지 않는다. 상대 텍스트 경로는 v11처럼 cwd 기준이며, L07 raw 상대 경로의 base 기준 계약과 구분한다. 조회 exists/is_file는 차단/누락 false, 수정/읽기/목록 실패는 예외로 고정한다.
- C10은 L07과 같은 64 MiB 상한을 유지한다. 큰 WAL·streaming·fd(C11)에는 착수하지 않는다. OS별 오류 문자열 전체의 동일성은 주장하지 않고 작업 실패 예외와 경로 차단을 검사한다.

시험 `.fl`과 정답을 먼저 커밋한 뒤 수정 전 실패를 기록한다. Node·TS·JS·Python·fl-build.sh 실행 금지. 정답 정정이 필요하면 v11 원문 근거·변경 이유·커밋을 남기며 구현에 맞춰 정답을 바꾸지 않는다. 자체 시험 뒤 기존 L06·L07·L08 1·2차 회귀와 변경하지 않은 verify-native-bootstrap.sh를 실행한다. strace 실행물·원문 출력·종료 코드·SHA를 보관하고 한국어로 확인한 것/미확인을 구분한다. 보고에 로컬 경로를 공개하지 않는다. 푸시는 Forgejo → GitHub 순서이며 두 HEAD는 실제 ref 조회 후 보고한다.
