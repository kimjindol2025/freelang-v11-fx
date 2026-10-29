# 실제 blog 저장소 fixture의 stdout 구분 정정

기준 blog1 `013a5b339068412332ae12a7ef0c412963a3a880` tests/stage1-write.fl·stage1-verify.fl의 PASS marker 정답은 그대로 유지한다. 두 fixture는 tcp-server.fls를 load하며, vendor `88b4aa2ca11e5355240a1d93ece8321315c669cc` engine.fls:1311~1518·sql-core.fls:592/708/711·protocol.fls:66·auth.fls:279의 최상위 자체 시험이 먼저 stdout을 출력한다. 이는 앱 fixture의 결과가 아니라 load의 원래 부작용이다. v11 소스를 기준으로 load된 최상위 표현식은 평가되어야 하며, 이를 native에서 생략하지 않는다.

원래 blog1 tests/b3-full-suite.sh:25~34의 run은 과정 종료 0을 요구하며 전체 로그를 저장하고 fixture의 PASS marker만 골라 표시한다. 따라서 새 native driver가 stdout 전체를 fixture marker 정답과 diff한 것은 원래 stdout 계약보다 엄격한 잘못된 가정이었다. 최초 writer 자체는 `writer_NATIVE_EXIT=0`, `PASS write-read-update`였지만 driver는 추가 prelude 때문에 `BLOG_STORE_TAIL_EXIT=1`이었다. 원문을 보존하고 구분 규칙을 정정한다. 기대 marker·앱·vendor·WAL 기록은 변경하지 않는다.

수정 driver는 과정 종료 0·stderr 비어 있음을 계속 요구한다. stdout 마지막 부분은 고정 fixture 정답과 정확히 비교한다. 앞부분은 원래 소스가 출력하는 `afj-db95 … PASS` 또는 `PASS SQL …` 행만 허용하고 원문 전체를 보관한다. 각 reader의 prelude는 같은 소스 writer의 prelude와 byte 단위로 같아야 한다. 그 밖의 출력은 실패다. reader 10회와 WAL 무변경·SHA-256은 별도 검증이며, writer 0만으로 전체 성공을 판정하지 않는다.
