# L08 3차 정규식 실제 호출 목록

blog1 `013a5b339068412332ae12a7ef0c412963a3a880`, db95 `88b4aa2ca11e5355240a1d93ece8321315c669cc`의 소스를 실행하지 않고 C lexical scanner로 조사했다. app의 추적된 FreeLang 22개와 실제 load한 db95 파일 5개, 총 27개가 입력이다. 주석·문자열 속 코드와 실행하지 않는 vendor 시험은 제외한다. 내부 중첩 regex 호출도 포함한다.

호출 위치 59개, literal 패턴 34종, 동적 생성 2종이다. 동적 생성은 SQL keyword 상수 44종의 `^keyword$`, TCP 알려진 오류 코드 21종의 `.*code.*`이며 자체 시험에 전부 고정한다. literal 34종도 각각 양성 시험으로 고정하고 lazy·공백·캡처·SQL lookahead·부정 입력은 별도 golden으로 검사한다.

| 위치 | 호출 | 종류 | 패턴 원문 |
|---|---|---|---|
| [app/admin.fl:512](https://github.com/kimjindol2025/blog1/blob/013a5b339068412332ae12a7ef0c412963a3a880/app/admin.fl#L512) | `regex_match_i` | literal | `"^[a-z0-9]+(-[a-z0-9]+)*$"` |
| [app/ai-api.fl:85](https://github.com/kimjindol2025/blog1/blob/013a5b339068412332ae12a7ef0c412963a3a880/app/ai-api.fl#L85) | `regex_match_i` | literal | `"^[0-9]{1,5}$"` |
| [app/ai-api.fl:114](https://github.com/kimjindol2025/blog1/blob/013a5b339068412332ae12a7ef0c412963a3a880/app/ai-api.fl#L114) | `regex_match_i` | literal | `"^[0-9a-f]{64}$"` |
| [app/ai-api.fl:165](https://github.com/kimjindol2025/blog1/blob/013a5b339068412332ae12a7ef0c412963a3a880/app/ai-api.fl#L165) | `regex_match_i` | literal | `"^[a-z0-9]+(-[a-z0-9]+)*$"` |
| [app/ai-api.fl:311](https://github.com/kimjindol2025/blog1/blob/013a5b339068412332ae12a7ef0c412963a3a880/app/ai-api.fl#L311) | `regex_match_i` | literal | `"^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}Z$"` |
| [app/blog-backup.fl:6](https://github.com/kimjindol2025/blog1/blob/013a5b339068412332ae12a7ef0c412963a3a880/app/blog-backup.fl#L6) | `regex_match_i` | literal | `"^[a-z0-9][a-z0-9-]{0,63}$"` |
| [app/blog-backup.fl:114](https://github.com/kimjindol2025/blog1/blob/013a5b339068412332ae12a7ef0c412963a3a880/app/blog-backup.fl#L114) | `regex_match_i` | literal | `"^[a-z]{2}(-[a-z]{2})?$"` |
| [app/blog-settings.fl:52](https://github.com/kimjindol2025/blog1/blob/013a5b339068412332ae12a7ef0c412963a3a880/app/blog-settings.fl#L52) | `regex_match_i` | literal | `"^[0-9]{1,2}$"` |
| [app/blog-settings.fl:63](https://github.com/kimjindol2025/blog1/blob/013a5b339068412332ae12a7ef0c412963a3a880/app/blog-settings.fl#L63) | `regex_match_i` | literal | `"^[a-z]{2}(-[a-z]{2})?$"` |
| [app/editor.fl:10](https://github.com/kimjindol2025/blog1/blob/013a5b339068412332ae12a7ef0c412963a3a880/app/editor.fl#L10) | `regex_replace` | literal | `"\\s+"` |
| [app/editor.fl:11](https://github.com/kimjindol2025/blog1/blob/013a5b339068412332ae12a7ef0c412963a3a880/app/editor.fl#L11) | `regex_replace` | literal | `"<[^>]+>&#124;\\[\\[(image&#124;link):.*?\\]\\]"` |
| [app/editor.fl:22](https://github.com/kimjindol2025/blog1/blob/013a5b339068412332ae12a7ef0c412963a3a880/app/editor.fl#L22) | `regex_replace` | literal | `"\\s+#"` |
| [app/editor.fl:24](https://github.com/kimjindol2025/blog1/blob/013a5b339068412332ae12a7ef0c412963a3a880/app/editor.fl#L24) | `regex_find_all` | literal | `"#[가-힣A-Za-z0-9_-]+"` |
| [app/editor.fl:24](https://github.com/kimjindol2025/blog1/blob/013a5b339068412332ae12a7ef0c412963a3a880/app/editor.fl#L24) | `regex_replace` | literal | `"<[^>]+>&#124;\\[\\[link:.*?\\]\\]"` |
| [app/editor.fl:34](https://github.com/kimjindol2025/blog1/blob/013a5b339068412332ae12a7ef0c412963a3a880/app/editor.fl#L34) | `regex_replace` | literal | `"(^&#124;\\n)([^<>\\n]+)$"` |
| [app/editor.fl:34](https://github.com/kimjindol2025/blog1/blob/013a5b339068412332ae12a7ef0c412963a3a880/app/editor.fl#L34) | `regex_replace` | literal | `"\\s+$"` |
| [app/image-admin.fl:20](https://github.com/kimjindol2025/blog1/blob/013a5b339068412332ae12a7ef0c412963a3a880/app/image-admin.fl#L20) | `regex_match_i` | literal | `"^[0-9a-f]{16}([.][a-z0-9]{1,20})?$"` |
| [app/images.fl:22](https://github.com/kimjindol2025/blog1/blob/013a5b339068412332ae12a7ef0c412963a3a880/app/images.fl#L22) | `regex_match_i` | literal | `"^[0-9a-f]{32}$"` |
| [app/menus.fl:6](https://github.com/kimjindol2025/blog1/blob/013a5b339068412332ae12a7ef0c412963a3a880/app/menus.fl#L6) | `regex_match_i` | literal | `"^https?://[a-z0-9][a-z0-9.-]*(:[0-9]{1,5})?([/?#][^\\s]*)?$"` |
| [app/pages.fl:15](https://github.com/kimjindol2025/blog1/blob/013a5b339068412332ae12a7ef0c412963a3a880/app/pages.fl#L15) | `regex_match_i` | literal | `"^[a-z0-9]+(-[a-z0-9]+)*$"` |
| [app/public.fl:99](https://github.com/kimjindol2025/blog1/blob/013a5b339068412332ae12a7ef0c412963a3a880/app/public.fl#L99) | `regex_match_i` | literal | `"^[1-9][0-9]{0,5}$"` |
| [app/render.fl:26](https://github.com/kimjindol2025/blog1/blob/013a5b339068412332ae12a7ef0c412963a3a880/app/render.fl#L26) | `regex_match_i` | literal | `"^https?://[a-z0-9][a-z0-9._~:/?#@!$&%()*+,;=+-]*$"` |
| [app/render.fl:33](https://github.com/kimjindol2025/blog1/blob/013a5b339068412332ae12a7ef0c412963a3a880/app/render.fl#L33) | `regex_find_all` | literal | `"\\[\\[link:.*?\\]\\]"` |
| [vendor/afj-db95/src/tcp-server.fls:362](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/tcp-server.fls#L362) | `regex_match_i` | dynamic | `(str ".*" (get item 0) ".*")` |
| [vendor/afj-db95/src/tcp-server.fls:367](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/tcp-server.fls#L367) | `regex_match_i` | literal | `".*(unsupported&#124;not supported).*SELECT.*"` |
| [vendor/afj-db95/src/engine.fls:78](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/engine.fls#L78) | `regex_match` | literal | `"^-?[0-9]+$"` |
| [vendor/afj-db95/src/engine.fls:196](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/engine.fls#L196) | `regex_match` | literal | `"^-?[0-9]+$"` |
| [vendor/afj-db95/src/engine.fls:733](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/engine.fls#L733) | `regex_split` | literal | `"\\."` |
| [vendor/afj-db95/src/engine.fls:757](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/engine.fls#L757) | `regex_match` | literal | `"^-?[0-9]+$"` |
| [vendor/afj-db95/src/engine.fls:760](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/engine.fls#L760) | `regex_match` | literal | `"^-?[0-9]+$"` |
| [vendor/afj-db95/src/engine.fls:850](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/engine.fls#L850) | `regex_match` | literal | `"^-?[0-9]+$"` |
| [vendor/afj-db95/src/sql-core.fls:8](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L8) | `regex_match_i` | dynamic | `(str "^" expected "$")` |
| [vendor/afj-db95/src/sql-core.fls:63](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L63) | `regex_split` | literal | `"\\s+(?=(?:[^']*'[^']*')*[^']*$)"` |
| [vendor/afj-db95/src/sql-core.fls:105](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L105) | `regex_split` | literal | `"="` |
| [vendor/afj-db95/src/sql-core.fls:155](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L155) | `regex_split` | literal | `"="` |
| [vendor/afj-db95/src/sql-core.fls:156](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L156) | `regex_split` | literal | `"="` |
| [vendor/afj-db95/src/sql-core.fls:256](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L256) | `regex_match` | literal | `"^[A-Za-z_][A-Za-z0-9_]*(,[A-Za-z_][A-Za-z0-9_]*)*$"` |
| [vendor/afj-db95/src/sql-core.fls:257](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L257) | `regex_split` | literal | `"="` |
| [vendor/afj-db95/src/sql-core.fls:259](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L259) | `regex_split` | literal | `","` |
| [vendor/afj-db95/src/sql-core.fls:278](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L278) | `regex_split` | literal | `","` |
| [vendor/afj-db95/src/sql-core.fls:302](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L302) | `regex_split` | literal | `"="` |
| [vendor/afj-db95/src/sql-core.fls:305](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L305) | `regex_split` | literal | `","` |
| [vendor/afj-db95/src/sql-core.fls:325](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L325) | `regex_match` | literal | `"^[A-Za-z_][A-Za-z0-9_]*$"` |
| [vendor/afj-db95/src/sql-core.fls:336](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L336) | `regex_split` | literal | `"\\s+"` |
| [vendor/afj-db95/src/sql-core.fls:355](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L355) | `regex_split` | literal | `"\\("` |
| [vendor/afj-db95/src/sql-core.fls:356](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L356) | `regex_split` | literal | `"\\)"` |
| [vendor/afj-db95/src/sql-core.fls:357](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L357) | `regex_split` | literal | `"REFERENCES"` |
| [vendor/afj-db95/src/sql-core.fls:358](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L358) | `regex_split` | literal | `"\\("` |
| [vendor/afj-db95/src/sql-core.fls:360](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L360) | `regex_split` | literal | `"\\)"` |
| [vendor/afj-db95/src/sql-core.fls:363](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L363) | `regex_match_i` | literal | `"^CHECK"` |
| [vendor/afj-db95/src/sql-core.fls:364](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L364) | `regex_split` | literal | `"\\("` |
| [vendor/afj-db95/src/sql-core.fls:365](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L365) | `regex_split` | literal | `"\\)"` |
| [vendor/afj-db95/src/sql-core.fls:377](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L377) | `regex_match_i` | literal | `"^UNIQUE"` |
| [vendor/afj-db95/src/sql-core.fls:378](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L378) | `regex_split` | literal | `"\\("` |
| [vendor/afj-db95/src/sql-core.fls:381](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L381) | `regex_split` | literal | `"\\)"` |
| [vendor/afj-db95/src/sql-core.fls:390](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L390) | `regex_split` | literal | `"\\("` |
| [vendor/afj-db95/src/sql-core.fls:493](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L493) | `regex_match` | literal | `"^[A-Za-z_][A-Za-z0-9_]*$"` |
| [vendor/afj-db95/src/sql-core.fls:507](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L507) | `regex_match_i` | literal | `"^(VARCHAR\\([1-9][0-9]*\\)&#124;TEXT&#124;INT&#124;INTEGER&#124;BOOLEAN)$"` |
| [vendor/afj-db95/src/sql-core.fls:529](https://github.com/kimjindol2025/afj-db95/blob/88b4aa2ca11e5355240a1d93ece8321315c669cc/src/sql-core.fls#L529) | `regex_match` | literal | `"^[A-Za-z_][A-Za-z0-9_]*$"` |

## 입력 파일

- `app/admin.fl`
- `app/ai-api.fl`
- `app/ai-pages.fl`
- `app/backup-admin.fl`
- `app/blog-backup.fl`
- `app/blog-settings.fl`
- `app/editor.fl`
- `app/image-admin.fl`
- `app/images.fl`
- `app/main.fl`
- `app/menus.fl`
- `app/pages.fl`
- `app/public.fl`
- `app/render.fl`
- `app/review.fl`
- `app/setup.fl`
- `app/store.fl`
- `app/themes.fl`
- `app/themes/basic/style.fl`
- `app/themes/basic/view.fl`
- `app/themes/reader/style.fl`
- `app/themes/reader/view.fl`
- `vendor/afj-db95/src/tcp-server.fls`
- `vendor/afj-db95/src/engine.fls`
- `vendor/afj-db95/src/sql-core.fls`
- `vendor/afj-db95/src/protocol.fls`
- `vendor/afj-db95/src/auth.fls`

이 목록은 패턴 요구사항이며 구현 통과 보고가 아니다. 추출기 소스는 같은 가지의 inventory.c이다. SQL 실행기·JSON·WAL·fd 기능을 이번에 구현하지 않는다.

표 안의 `&#124;`는 Markdown 열 구분을 피하기 위해 표시한 원문의 `|`이다. 실행하는 실제 `.fl` 패턴에는 원문의 `|`를 사용하며, 각 행의 고정 SHA 소스 링크에서 원문을 확인할 수 있다.
