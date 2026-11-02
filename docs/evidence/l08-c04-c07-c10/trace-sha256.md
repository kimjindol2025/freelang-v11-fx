# strace 원본 SHA-256

논리 이름이며 로컬 기준 경로는 생략했다. 원본은 시험 보관본이다.

| 추적 | SHA-256 |
|---|---|
| `before/execve.log` | `4add216a39bf15f98d1af118691bb7a4627cdd640ca9c09653d9b46c8e011cfb` |
| `development-first/files-fixed.execve.log` | `718d16c3ad989e7d81a2c686010115347f78a101ee5a3f3c3cb6042723796766` |
| `development-second/dst-check.execve.log` | `9e941bcc4b48a296b247635c582814c11b898d9ea4fc280bd1d57075196090f5` |
| `development-second/regex-guards-fixed.execve.log` | `d0767e2931c34b71a1ed0bea7a0cfae64bf2a1f42d11b33fb0b16aafe72fee9f` |
| `development-interim/date-nul-before.execve.log` | `010c385b0559911c9151b570363db34b2b1623260cba5e8c8f84d0e83d70b33a` |
| `clock/execve.log` | `cf1c67d8bc0a216065433dce8cc0a0212900e864c6fe5f5f823431581a4d7b4a` |
| `sanitizer-link/execve.log` | `f01ade25eeb83359980c564cd57404f710cccc86cd6428f770018228307f80db` |
| `final/phase2/phase1/http.execve.log` | `c9d6621d3b45ee59d1d3ab83a97eb7eb49894bd557a364312d6bfe6b97b0696e` |
| `final/phase2/phase1/regressions/HTTP.execve.log` | `ae4f9944ca1e033fa8e808a1848ad40a5a5796716f542b77290b255d0a0784ce` |
| `final/phase2/phase1/regressions/REGRESSIONS.execve.log` | `1b95fa606e624a42f96cb1c555a4b8186defc1f39e71b00ec9dbbc29139f78ee` |
| `final/phase2/phase1/regressions/REPRO.execve.log` | `bb32144c80f84df011be9db43a2092372ff98be56292c8ade7e88ba8c9dfbd9c` |
| `final/phase2/phase1/regressions/bootstrap/traces/ARRAY_LOOP.execve.log` | `a3bc32633860bd32385c3514f01be4dddd998ed6d82e25a71a4ec8b7fb6bce24` |
| `final/phase2/phase1/regressions/bootstrap/traces/ARRAY_LOOP_GENERATE.execve.log` | `0ecacf12227ebeb900b9838d934c3c1f2cdd37fe104b00a7711f498a08601887` |
| `final/phase2/phase1/regressions/bootstrap/traces/STAGE0_TO_STAGE1.execve.log` | `ac6783db1771748f40cc4775e095df76c1ae20445d1a8d531aeccf453c0519f2` |
| `final/phase2/phase1/regressions/bootstrap/traces/STAGE1_TO_STAGE2.execve.log` | `04a153bab2343a73c82b1943aa0ab6ca92b996d65bfd1a6efd1075e65aa47f67` |
| `final/phase2/phase1/regressions/bootstrap/traces/STAGE2_TO_STAGE3.execve.log` | `3bf21a3b928746851b096f1c2f38605d604c467a0ba8fa0a0b15b76a91fd9abf` |
| `final/phase2/phase1/regressions/bootstrap/traces/gcc-129.execve.log` | `2dc7a853d2dad15578cdd619cfc79c6511053b7dbd2173162ff8c591d8759416` |
| `final/phase2/phase1/regressions/bootstrap/traces/gcc-19.execve.log` | `01c3822912ed3b8d43d05964fdf4f661f3921ad4e5dd25c0ca32aa16f0773686` |
| `final/phase2/phase1/regressions/bootstrap/traces/gcc-194.execve.log` | `6fa774ba3cae97b21630beccee3720d90be02e08ba99439c67af9bf615835059` |
| `final/phase2/phase1/regressions/bootstrap/traces/gcc-74.execve.log` | `a49947c1cd17c6f64eb1afc766a28f2498d429a30934ca594b8cd4cef7a05da3` |
| `final/phase2/phase1/self.execve.log` | `53018dff2c92290e3b7b7fe3906aac9b36dce21b7164508b903a08ee6e347792` |
| `final/phase2/self.execve.log` | `3b59ad590bc1d09dde5f1b5151877463afe9d6adccb931dfa2ce6afbaabb3ec2` |
| `final/self.execve.log` | `1be242e30eb9c6d9457c69e80aae6c3a126d3c6804d849fb82e5acc6272c3d7a` |

실행물 목록은 execve 시도 횟수이며 실패한 PATH 탐색도 포함한다. 성공한 과정 수로 세지 않는다. gzip은 basename을 보존하고 별도로 미호출 감사를 다시 수행했다. 추가 sanitizer trace는 링크 실패까지의 기록이며 sanitizer 프로그램 실행 증거로 세지 않는다.
