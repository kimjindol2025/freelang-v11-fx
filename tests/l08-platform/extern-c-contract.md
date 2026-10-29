# native C 명시 선언 계약

실제 고정 `09bf29622098199f1136a58df1915ed865419b6b` 전체 실행은 fd-concurrent.h에서 선언하고 fd-concurrent.c에서 구현·link한 fd-test-barrier를 실행 시 registry에서 찾다 실패했다. 새로운 지연 조회가 알려지지 않은 C 확장 호출까지 바꾼 회귀다. 기존 8-thread/800-record 기대값과 `.fl` 시험은 바꾸지 않는다.

C backend에 prefix option `--extern SOURCE_NAME`을 추가해 외부 C 호출을 명시한다. 여러 이름은 옵션을 반복한다. source 이름은 ASCII letter/underscore로 시작하고 나머지는 letter/digit/underscore/hyphen이다. C 이름은 기존 c-name 변환을 따른다. 이 옵션은 C 구현이나 type을 만들지 않는다. 실제 FLValue 함수 prototype을 C header로 제공하고 구현을 link해야 한다. 일반 lexical binding·v11 builtin 우선순위와 선언하지 않은 함수의 실행 시 조회는 유지한다. 범위는 해당 compiler CLI 과정이며 다음 compiler 과정으로 넘어가지 않는다.

정답은 위 실제 C 함수의 연산 `value + 100`과 상수 `17`에서 고정한다. 두 함수를 실제로 link해도 선언하지 않은 source 호출은 registry의 함수 미발견 오류여야 한다. 선언하면 `23 → 123`, `-7 → 93`, zero args `17`이어야 한다. 두 옵션 반복·선언 없는 재실행·잘못된 이름·누락된 이름·실제 C 구현 없는 link 실패도 확인한다. C helper를 runtime stub으로 추가하거나 fd 시험의 barrier·thread 수·출력 정답을 낮추지 않는다.

이 기능은 fx C 확장 선언이며 v11 Node interpreter의 표준 기능이라고 주장하지 않는다. C 타입/ABI 검사는 실제 header와 C compiler가 담당한다. 직접 호출 범위이며 외부 C 함수의 first-class 값이 필요하면 명시적인 FreeLang fn adapter를 사용한다. TS·JS·Python·fl-build.sh를 사용하지 않는다.
