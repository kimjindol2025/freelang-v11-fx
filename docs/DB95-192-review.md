# DB95 repeat native 검수 변경

기준 HEAD a9ffce18, 기준 가지 fix/db95-23-nocurl. GitHub 전용 검수 브랜치 대상이며 main 병합·배포는 금지한다. 기존 tmp 자료는 보존한다.

runtime/aliases.c의 fl_repeat가 FL_INT만 받아 산술식의 FL_FLOAT 반복 횟수를 0으로 취급했다. 유한하고 소수부가 없는 int64 범위의 FL_FLOAT를 검사 후 변환하도록 최소 수정했다. 기존 vector builder와 정수 입력 동작은 유지한다. 분수·NaN·무한대·범위 밖 실수는 변환 전에 명시적으로 거부한다.

새 tests/db95-191-repeat.fls는 (+ 1048576 1) padding 길이와 양 끝 문자, 산술식으로 만든 3·0·음수 횟수, 잘못된 실수 입력의 오류를 검증한다. 최초 3.0/0.0/-1.0 리터럴이 생성 C에서 FL_INT로 내려간다는 별도 QA 지적을 반영하여 (+ 1 2), (+ 0 0), (- 0 1)로 보완했다. 이후 단독 실행은 REPEAT_QA_EXIT=0이며 별도 검증자가 생성 C·runtime 산술 반환 FL_FLOAT·완료 로그를 대조해 PASS로 확인했다.

기존 db95-26-repeat.fls의 2MiB 및 정수 경계 회귀도 EXIT=0이었다. 기존 시험·기대값·컴파일러 self·seed·Stage0를 변경하지 않았다. DB95 case 7/13은 이 runtime을 링크한 원본 worker로 PASS했다. 관련 DB95 원인·부분 목록 결과는 대응 GitHub 검수 브랜치의 DB95-192-case12-39.md를 따른다.

실행은 고정 Stage4 SHA-256 7bca2ea4c37619bce3f6047d88e9bace717129034c92836c322d2ba845cfdb2a, 2GiB 주소 공간 제한, 120초 worker, flock -o /tmp/fl-heavy.lock을 사용했다. 보존된 Stage2·3·4 생성 C의 e4fb1a898828cd873989008e248cdcadeb5dc7ce72ac4b68d5f3ea1b28c6f487 일치는 기존 자료의 보존 확인이다. 수정 runtime으로 bootstrap을 새로 실행한 것은 아니다.

검증 로그는 docs/evidence/DB95-192에 식별 경로를 가려 보존한다. 별도 QA는 코드·로그·생성 C를 직접 읽었고, 전체 71건 독립 재실행은 하지 않았다.

릴리스 미승인: vector builder의 할당 실패/capacity overflow 보호가 미검증이며 기존 구현에 위험이 남는다. 장애 무결성 시험과 case 9·25·JS HOLD, 전체 71건 gate는 별도 단계다. 현재 변경의 검수 PASS를 상용 릴리스 승인으로 해석하지 않는다.

최종 별도 QA는 이 저장소의 staged 8파일, 실제 로그와 가린 사본의 cmp, SHA manifest, 생성 C의 FL_FLOAT 경로 및 변경 코드를 직접 확인하고 GitHub 전용 review/db95-191-native-fixes 검수 브랜치 제출 PASS를 판정했다. 원격 SHA는 푸시 후 별도 확인·보고하며 Forgejo 푸시·main 병합·배포는 하지 않는다.
