# 블로그 B06 — HTTPS 예약 루프 수정 검증

제품 소스 `45923b8b1f23dad3cac729fad2c550878ef57f8b`를 검증했다. 재현 시험은 `6d78a9f0ff4de1594d30f5c6b3b22d380e1ccd20`에서 먼저 고정했다. 기존 승인 C 기반의 결함 수정이며 새 API·의존성·정책은 없다. 운영 배포는 하지 않았다.

HTTPS accept 루프에 HTTP와 같은 scheduler tick·기한 대기·shutdown 처리가 빠져 있었다. 기존 타이머 API를 연결했으며 `runtime/http.c`의 실행 변경은 8줄 추가·1줄 삭제다. 요청 방문 없이 같은 5000ms 경계의 재현은 수정 전 FAIL, 수정 후 PASS다. 시험용 키는 로컬 임시 영역에서만 만들고 검사 뒤 삭제했다.

| 검증 | 실제 결과 |
|---|---|
| 전체 native 관문 | 종료 0, L07 221/0 |
| 기존 누수 그룹 | 2+1+1+2+1+5 = 12/12 |
| 실제 Chrome 언어 관문 | 타이머·예외·문자/값·이벤트·DOM·블로그 모델·HTTP·취소 8개 묶음 PASS |
| HTTP / 취소 | 각각 Chrome 20/20 |
| CLI 수명 | 4/4 |
| 작은 누수 / 워커 정리 | 기존 러너 종료 0 |
| 브라우저 저장소 / native 이미지 API | 각각 20/20 |
| J_UTF16_LENGTH | PASS |
| 원래 시험·기대값·러너·seed 변경 | 0 |

생성 C stage2=3=4:

```
f38f373890d0898552cbe89768b992f934e9c4a567bf27cdf66248a3c1b5cb81
f38f373890d0898552cbe89768b992f934e9c4a567bf27cdf66248a3c1b5cb81
f38f373890d0898552cbe89768b992f934e9c4a567bf27cdf66248a3c1b5cb81
```

고정 C seed 시작/끝:

```
f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c
f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c
```

정식 compiler ELF SHA-256은 `62bf4232d41df948a3660e880eaa40082f48ac926ac16308c4d7790e34b91133`이다. 판정은 ELF가 아닌 생성 C로 했다. bootstrap은 strace 없이 실행했고 감사는 정적 감사다. `BOOTSTRAP_DYNAMIC_EXEC_COUNTS=UNOBSERVED`를 실행 수 0으로 바꾸지 않는다. 추적한 회귀/브라우저 빌드 범위에서는 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다. 브라우저 안의 승인된 생성 JS 실행은 별도다.

무거운 작업은 공통 flock과 실제 메모리 2147483648바이트·swap 0 제한 아래 순차 실행했다. Chrome/ASan에는 가상 주소 예약 제한을 적용하지 않았다. 앞선 준비 실행은 가상 주소 제한 때문에 Chrome 종료 133이었으며, 기존 러너·정답을 바꾸지 않고 환경을 바로잡은 뒤 위 결과를 확인했다.

[전체 원문](evidence/blog-b06-tls/native-bootstrap.txt), [언어 결과](evidence/blog-b06-tls/language-browser-api.txt), [수정 전](evidence/blog-b06-tls/tls-before.txt), [수정 후](evidence/blog-b06-tls/tls-after.txt), [시험 키 삭제](evidence/blog-b06-tls/temporary-key-cleanup.txt)를 보존한다. 경로·주소는 가렸다. 이는 언어 수정 검증이며 블로그 B06 전체 PASS를 뜻하지 않는다. 블로그의 성능 인수는 별도다.
