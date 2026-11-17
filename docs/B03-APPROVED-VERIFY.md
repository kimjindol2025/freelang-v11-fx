# B03 승인된 언어 선행 검증 완료

2026-11-17 사용자 “승인”으로 TLS 합성 키의 작업 임시 영역 사용과 문서 갱신 뒤의
오래된 nodeId만 재조회하는 C 판독기 7줄을 적용했다. 기존 정답·단언·시험 수·제한시간은
유지했다. 과거 중단·실패 원문은 앞선 보고서에 남긴다.

| 실제 실행 | 결과 |
|---|---|
| 최신 언어 전체 native 관문 | 종료 0, `L08_PLATFORM_CURRENT_GATES=PASS` |
| E09 실제 Chrome | `L01_BROWSER_HTTP_PASS=20 FAIL=0`, 문서 재조회 1회 기록 |
| 이미지 API 고정 정답, 최신 전체 runtime 객체로 재실행 | `IMAGE_NATIVE_PASS=20 FAIL=0`, 종료 0 |
| UTF-16 / L07 | `J_UTF16_LENGTH=PASS` / `L07_SELF_PASS=221` |
| TLS | TCP 일반·TLS·상호 인증 완료, HTTP 클라이언트 6/6, L06 HTTP 110/0·TLS 110/0 |
| 격리 bootstrap | strace 0, Stage2=3=4, 잔류 프로세스 0 |
| 시험 키 정리 | TCP·HTTP 클라이언트·L06 TLS 키/인증서 삭제, 합성 CSRF 파일 삭제, 잔여 개인 키 파일 0 |

이미지 구현은 `73885b248f472c49185255b1c7515b3a5fef4c54`, 고정 정답 선커밋은
`ef354a55aa12accea3447031821b9d178c647aec`이다. 이번 전체 실행 소스는
`03342395ab79b6164163c792499b9354c78de12b`이며 해당 커밋은 승인된 C 판독기만 바꿨다.
실제 문서 갱신을 관찰한 상태와 정확한 오류 코드/문구를 모두 요구하고 기존 제한시간
안에서 문서 루트를 다시 찾는다. 오류를 성공으로 바꾸거나 제한시간을 늘리지 않았다.

```text
STAGE2_SHA=f38f373890d0898552cbe89768b992f934e9c4a567bf27cdf66248a3c1b5cb81
STAGE3_SHA=f38f373890d0898552cbe89768b992f934e9c4a567bf27cdf66248a3c1b5cb81
STAGE4_SHA=f38f373890d0898552cbe89768b992f934e9c4a567bf27cdf66248a3c1b5cb81
SEED_BEFORE=f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c
SEED_AFTER=f7425bf086d99d11231ba79296e194fcc36b454379e46d44131bafb0dc74101c
```

고정 seed와 현재 canonical 생성 소스가 다르다는 기존 출력도 보존했다. seed를 바꾸지 않고
읽기 전용 출발점으로 사용했으며 이번 생성 C 고정점은 앞선 이미지 API 검증과 같다.
기존 실행 감사 원문은 `NODE=0 NPM=0 NPX=0 JS=0 PYTHON=0`이다.
이는 수집한 native 실행 trace 범위의 수치다. bootstrap은 정적 감사 PASS와 동적 실행 수
미관측을 구분한다. 의존성 빌드 전체의 동적 실행 수도 미관측이며 전역 0이라고 주장하지 않는다.

전체 작업은 공통 잠금·2GiB 물리 메모리 상한·swap 0 아래 한 번에 실행했다.
ASan에는 물리 상한을 적용하고 가상 주소 공간 예약은 허용했다. bootstrap은 strace 없이
실행했다. 합성 인증 파일 한 개의 정리에서 자동 검토가 `rm -f` 형식을 거부해 실제 파일
한 개만 확인한 뒤 `unlink`로 삭제했다. 비밀 내용은 읽거나 출력하지 않았다.

원문은 경로·주소만 가린 [증거 목록과 SHA](B03-APPROVED-VERIFY.json)에 보존했다.
이 문서는 언어 선행 관문의 완료 기록이다. blog1의 B03 전체 제품 PASS와 B04 진행을
대신하지 않는다. 제품 수정 전 `media-options` 미구현 실패를 별도로 보존한 뒤 앱 연결을 시작했다.
