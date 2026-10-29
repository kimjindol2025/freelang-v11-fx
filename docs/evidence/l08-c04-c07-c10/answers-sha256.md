# 최종 고정 정답 SHA-256

기본 자체 시험의 소스/정답은 최종 구현 `a06b883d6d8a6eb17eacdb12f343b7e996e45f7c` 전 고정했다. clock은 추가 독립 시험 `986d3050e5ff1a6bd2347e1ad7708d5a039484fc`이다.

| 정답 | SHA-256 |
|---|---|
| [base-symlink.expected](https://github.com/kimjindol2025/freelang-v11-fx/blob/986d3050e5ff1a6bd2347e1ad7708d5a039484fc/tests/l08-phase3/base-symlink.expected) | `21d91041c5d1a59661f8936571ee5c87e96f6f0c7a532b52ec17c5b9bea8220b` |
| [clock.expected](https://github.com/kimjindol2025/freelang-v11-fx/blob/986d3050e5ff1a6bd2347e1ad7708d5a039484fc/tests/l08-phase3/clock.expected) | `a17fcf0a2f50e2d495e4f90ce263410edc183add6c62699a2facbccf60410f74` |
| [dates.expected](https://github.com/kimjindol2025/freelang-v11-fx/blob/986d3050e5ff1a6bd2347e1ad7708d5a039484fc/tests/l08-phase3/dates.expected) | `a21791478d88b8baf621deee0217782c8e6244e7336e81fcabcd1f250b73b3a2` |
| [files.expected](https://github.com/kimjindol2025/freelang-v11-fx/blob/986d3050e5ff1a6bd2347e1ad7708d5a039484fc/tests/l08-phase3/files.expected) | `c2e21467ccbb01f5660da563b0c4add93ef477c2869a5f5b65afca8a7937e380` |
| [images.expected](https://github.com/kimjindol2025/freelang-v11-fx/blob/986d3050e5ff1a6bd2347e1ad7708d5a039484fc/tests/l08-phase3/images.expected) | `666d202695e722afe3facd74367d34a605fb7ab1006c8fac5fd4213b20bfbd90` |
| [regex-guards.expected](https://github.com/kimjindol2025/freelang-v11-fx/blob/986d3050e5ff1a6bd2347e1ad7708d5a039484fc/tests/l08-phase3/regex-guards.expected) | `a0dc92061fb04ef7187aff5a8af4947f7fad83ebc808c7d69a74e97fe68ef282` |
| [regex-patterns.expected](https://github.com/kimjindol2025/freelang-v11-fx/blob/986d3050e5ff1a6bd2347e1ad7708d5a039484fc/tests/l08-phase3/regex-patterns.expected) | `942a8401298ed2b21912dd4c5dafe4f6c8410003f0e9c66ad3c34688ceaf077e` |
| [regex.expected](https://github.com/kimjindol2025/freelang-v11-fx/blob/986d3050e5ff1a6bd2347e1ad7708d5a039484fc/tests/l08-phase3/regex.expected) | `fcfa6cd56f1841a504316b9e5c9abda36d207c3ff622e247e169b8ab9663b849` |
| [security.expected](https://github.com/kimjindol2025/freelang-v11-fx/blob/986d3050e5ff1a6bd2347e1ad7708d5a039484fc/tests/l08-phase3/security.expected) | `354170ac93b4263e5e8f679b87a706e9c70058acdd68ca3bfbb5ffc56c14ba98` |
| [timezone-newyork.expected](https://github.com/kimjindol2025/freelang-v11-fx/blob/986d3050e5ff1a6bd2347e1ad7708d5a039484fc/tests/l08-phase3/timezone-newyork.expected) | `11d61dec19516f80f8a542478aaeffb8990f9b47ea57ca568fe24a81bf4b9009` |
| [timezone-seoul.expected](https://github.com/kimjindol2025/freelang-v11-fx/blob/986d3050e5ff1a6bd2347e1ad7708d5a039484fc/tests/l08-phase3/timezone-seoul.expected) | `38d303983225502c82591381effb2a96f945c642bbb4aaabd8b302b15b722cdd` |
| [timezone-utc.expected](https://github.com/kimjindol2025/freelang-v11-fx/blob/986d3050e5ff1a6bd2347e1ad7708d5a039484fc/tests/l08-phase3/timezone-utc.expected) | `69b83e64f730c61e537094b46fe552cbedfedd840aa1bbcc1cd2d23aa286112e` |

최초 `$` 추론 정정, CLI fixture 수정, 시험 강화와 추가는 계약 문서·Git 이력에 기록했다. 의미상 정답을 전부 처음부터 불변으로 유지했다고 주장하지 않는다. 최종 기본 fixture manifest는 모두 OK다.
