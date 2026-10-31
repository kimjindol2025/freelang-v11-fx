# CLI의 잘못된 출력 대상 선택 방지

J안 미지원 재현에서 원래 compiler가 --browser-js를 입력 이름으로, 뒤의 .fl을 출력으로 취급하여 성공 0을 반환하며 시험 복사본을 C로 덮어썼다. 원본 저장소 파일은 사용하지 않았다.

알 수 없는 --option은 파일을 읽거나 출력하기 전에 종료 1로 거부해야 한다. --definitely-invalid/--ast-typo/--help-typo 세 경우의 입력 hash 불변·출력 파일 없음과 기존 --ast 성공/입력 보존을 네 묶음으로 고정한다. 기존 --extern 및 native positional 동작은 회귀에서 유지한다. 별도 --browser-js의 실제 지원은 L01 target 구현으로 다루며 성공/실패를 옵션의 존재만으로 보고하지 않는다.

정답/시험 → 원래 compiler의 실패 원문 → CLI 수정 → 네 묶음 및 기존 native 동작 검증 순서다. 정답을 낮추거나 source 파일을 지워 복구했다고 주장하지 않는다.
