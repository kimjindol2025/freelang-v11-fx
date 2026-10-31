# 브라우저 target 거부 계약

각 reject-*.fl은 compiler 종료 1, 입력 byte 보존, 새 출력 파일 없음이 정답이다. compiler 전용 j-host/jcore 이름은 underscore 별칭도 막는다. native file I/O·eval·잘못된 함수/바인딩·loop 밖 recur를 JS로 우회하거나 nil로 바꾸지 않는다. 입력/출력 같은 파일과 잘못된 출력 확장자도 거부한다. 성공 문구·부분 출력으로 통과시키지 않는다.

runtime 오류와 다른 compiler 오류로 구분한다. browser host의 없는 node/잘못된 타입과 일반 언어 동적 오류는 실제 browser fixture에서 별도로 시험한다. 이 항목만으로 모든 언어·보안 경계를 인수했다고 보고하지 않는다.
