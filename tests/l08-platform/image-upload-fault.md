# 실제 HTTP 이미지 업로드와 불확실 commit

관리자 POST /admin/images와 AI POST /api/ai/images를 실제 app/main으로 실행한다. test binary의 loopback 진단 경로만 앞에 등록하며 운영 소스에 test endpoint를 넣지 않는다. worker는 4개다. 각 경로의 정상 업로드와 commit fsync·close 오류를 각각 새 합성 WAL에서 실행한다. fault helper의 선택 상태는 atomic이며 대상 inode만 주입하고 private marker에 발생 횟수만 기록한다.

시험은 합성 세션·시험용 AI key로 인증한다. 실제 비밀번호 로그인 시험이 아니다. key·Cookie·CSRF 값은 mode 600 curl config와 환경에만 두고 출력·execve 인자에 넣지 않는다. 관리자 CSRF는 실제 이미지 화면에서 받는다. multipart에는 GIF prefix·NUL·ff를 가진 raw fixture를 사용하므로 완전한 GIF decoder나 브라우저 이미지 렌더링 증거를 뜻하지 않는다.

정답: 정상 관리자 editor_upload 응답 200·AI 201. 결과가 불확실하거나 durable 뒤 close 오류인 경우는 503과 재시작 필요 알림이며, 잠재적으로 commit된 이미지를 삭제하지 않는다. 각 경우 실제 과정 재시작 2회 후 image metadata 1행, 공개 이미지 HTTP 200, raw byte SHA-256 일치, read-only 접근 뒤 WAL 불변이다. helper fault marker는 정상 0·주입 1이어야 한다. 원래 WAL·거래·정답·worker 수를 줄이지 않는다. 실제 전원 차단·상위 directory 내구성과 사용자 비밀번호 인증은 별도 범위다.

정상·commit 오류 여섯 경우의 원래 18개 정답은 그대로 유지한다. prepare 전체 write 실패·일부 write 실패를 두 경로에 추가한다. 이 네 경우는 503·재시작 안내 후 새 과정 2회에서 metadata 0행·영구 이미지 폴더 비어 있음·WAL 불변이 정답이다. 해당 hash/이미지 HTTP는 NA로 표시한다. 이미 같은 소스에서 빌드한 seed/server를 재사용하면 L08_IMAGE_UPLOAD_BIN_DIR과 실행물 SHA-256을 기록한다. 기본 실행은 두 프로그램을 새로 빌드한다.

각 POST 뒤 현재 과정의 공개 홈도 요청한다. 정상은 200, DB가 복구 필요인 모든 오류는 503이 정답이다. 오류 뒤 stale 조회나 추가 DB 작업을 시도하다 일반 500을 내는 것으로 통과시키지 않는다. 원래 30개 업로드/재시작 정답을 유지하고 이 10개 가용성 확인을 추가한다.
