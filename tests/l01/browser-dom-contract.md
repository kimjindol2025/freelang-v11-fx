# J3 편집기 DOM 구성 계약

FreeLang 블록/사진 UI에 필요한 browser 전용 API다. fixture·정답을 구현 전에 고정하고 실제 loopback HTTP/Chrome으로 검증한다. innerHTML·eval·script 문자열 실행 API는 제공하지 않는다. compiler는 DOM primitive만 낮추고 검증/흐름은 self/browser-core.fl에 작성한다.

| API | 반환/경계 |
|---|---|
| browser-create(tag) | HTML Element. 허용 tag만 정확한 소문자 이름으로 생성 |
| browser-append!(parent,child) | nil. 기존 요소는 복제하지 않고 이동 |
| browser-insert-before!(parent,child,before) | nil. before=nil은 끝. 실제 자식이 아니면 native DOM 오류 |
| browser-remove!(element) | nil. 이미 분리된 요소도 안전 |
| browser-clear!(element) | nil. 자식 제거, HTML 해석 없음 |
| browser-children(element) | 현재 Element 자식의 새 vector snapshot. 이후 이동/삭제로 기존 vector 변경 없음 |
| browser-attribute(element,name) | 문자열 또는 nil. 허용 attribute 및 hidden만 읽기 |
| browser-set-attribute!(element,name,string) | nil. 아래 허용 목록/URL/태그 검사 후에만 변경 |
| browser-set-hidden!(element,boolean) | nil. hidden property 반영. truthy 강제 변환 금지 |

허용 tag는 div, section, article, p, span, strong, em, u, s, blockquote, pre, code, h2, h3, ul, ol, li, hr, br, a, img, figure, figcaption, label, button, input, textarea다. script/iframe/style/base/SVG 및 사용자 tag는 허용하지 않는다.

쓰기 attribute는 id, class, title, alt, width, height, role, aria-label, aria-expanded, tabindex, contenteditable, type, accept, data-block-id, href, src다. style과 on* 속성은 허용하지 않는다. hidden은 전용 boolean API로만 쓴다. href는 A의 http/https URL, src는 IMG의 같은 origin http/https URL만 허용한다. URL parser로 정규화한 절대 href를 저장한다. javascript/data/blob 및 이미지 외부 origin을 거부하며 기존 속성을 보존한다. 새 URL 객체 생성은 네트워크 요청을 하지 않는다. 실제 이미지 decoding/처리는 이 계약에 포함하지 않는다.

모든 API는 정확한 인자 수/타입 검사 후 변경한다. DOM 계층 cycle·다른 부모의 before는 native 오류를 그대로 throw한다. 29개 .fl 정답과 C CDP의 최종 구조·literal HTML 부재 2개, 총 31개가 고정 정답이다. native 의미 29개·compiler 거부 10개는 유지한다. 생성물 재현성·출처·화면·종료 코드·execve 감사를 보관한다. 제품 E02/E03 전체, 브라우저 선택/IME/저장/undo/redo 완료를 의미하지 않는다.
