# C17 구현 전 고정 정답 계약

기준 v11 `b49cc9194a00649b8cf58e1d6e8ee38ffce770d4`, `src/stdlib-feed.ts`의 rss_feed와 sitemap_xml을 텍스트로 조사했다. Node 실행은 금지한다. source XML 줄 순서·개행·5개 escape, truthy optional 필드, 입력 item 순서 유지, 기본 language en, URL 끝 slash 한 개 처리, 숫자 priority만 출력하는 계약으로 정답을 구현 전에 고정한다. 절대 URL을 별도 재해석하거나 item을 정렬하지 않는다.

날짜는 ISO·한국 +09:00·숫자 millisecond를 GMT RFC 형식으로 출력한다. 유효하지 않은 date는 source toUTCString의 Invalid Date다. 기존 C04 date parser를 재사용하되 예외는 이 반환값으로 변환한다. timezone 없는 ISO 시간은 기존 local timezone 계약을 따른다. 시험은 UTC·Asia/Seoul에서 같은 명시 offset 입력의 결과가 같은지 확인한다.

CDATA의 `]]>`는 `]]]]><![CDATA[>`로 분할해 파싱 후 원문은 보존하면서 XML을 닫는 입력을 차단한다. source의 단순 raw 보간과 다른 의도된 XML 안전 처리다. 다른 콘텐츠를 삭제·HTML escape하는 우회는 금지한다. 고정 정답에도 이 차이를 구현 전에 명시한다.

별도 C XML reader로 RSS·sitemap이 XML로 파싱되는지와 CDATA 원문 보존을 확인한다. XML parser는 시험 전용이며 런타임 새 외부 의존성을 도입하지 않는다. 현재 시간 fallback은 고정 clock fixture로 시험한다. 앱 통합 공개/비공개 글 노출 정책·실제 HTTP 응답은 별도 검증하며 자체 serializer 시험으로 대체하지 않는다.
