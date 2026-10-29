# C18 non-tail 부작용 누락 후속 재현

기존 고정 TCP 시험은 큰 binary 입력의 모든 chunk를 echo해야 한다. tcp-echo.fl의 handler는 do의 중간 if에서 echo-bytes를 실행하고 그 뒤 close 분기를 평가한다. tail 기능을 넣은 native `7fbe7698478d13ff5d6cfa8bf16c69e0aabb92d4` 전체 실행에서 NUL·모든 byte는 통과하지만 이어진 chunk가 없는 큰 echo와 TLS 재연결 검사가 실패했다. tail token이 생성된 뒤 마지막 표현식 결과에 덮여 실행되지 않는 경우를 최소화한다. 기존 TCP 정답을 낮추거나 chunk를 버리지 않는다.

v11 eval-special-forms.ts:1248~1298의 do는 각 표현식을 순서대로 평가하며 if의 주석 계약은 ‘꼬리 위치’ 사용자 함수 호출이다. 다만 해당 v11 소스의 do loop에는 non-last tcoMode guard가 명시되어 있지 않다. v11 전체 실행에서 이 조합이 어떻게 처리되는지는 Node 금지로 실행 확인하지 않았다. source의 이 부분과 의도한 꼬리 위치 규칙을 혼동해 완전 호환이라고 보고하지 않는다.

이 정답은 먼저 고정된 실제 TCP echo·부분 쓰기·회복 요구와 순차 평가의 부작용 보존 요구를 유지한다. 중간 if의 함수 호출은 실제로 실행돼야 하며 마지막 식의 결과만 반환해야 한다. do 내부와 defn의 여러 body 표현식을 모두 확인한다. 수정은 AST/emitter의 tail 위치 구분에 하며 앱·TCP fixture를 바꾸지 않는다. 일반 tail 5000회와 오류 문맥 정답도 함께 유지해야 한다.
