# 실제 HTTP 저장소 동시 쓰기와 재시작

승인된 source-only blog의 실제 app/store.fl·vendor를 load한 native 서버에 loopback HTTP 요청을 보낸다. 공개 제품 API가 아닌 시험 전용 경로이며 관리자 인증 완료로 집계하지 않는다. 새 합성 WAL만 사용한다.

- 운영 파일 접근 금지. OS 자동 할당 포트, 실제 FL_WORKERS=4, curl 동시 10개. native 첫 4개 handler barrier로 overlap/worker 4개 확인.
- 서로 다른 ID 32개를 HTTP POST. 모두 200, 응답 32개, 실제 live 행의 ID가 1..32와 정확히 같고 제목이 한🙂인지 확인.
- 동일 ID 1을 다시 8번 동시 POST: 전부 409, WAL 바이트와 live 32행 불변. 실패를 성공 응답으로 바꾸지 않는다.
- 자기 시험 서버만 종료·재시작 2회. 매번 /health 200, 32개의 정확한 행·제목·WAL 무변경 확인. 서버 오류·timeout·비정상 종료는 실패.
- ASan/UBSan, strace로 native 호출만 확인. 비밀번호 로그인·브라우저 실행·성능 p95·WAL/fsync 장애 원자성은 별도 남은 시험이다.
