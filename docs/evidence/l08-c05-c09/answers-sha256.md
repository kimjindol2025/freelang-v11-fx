# 고정 정답 SHA-256

최종 구현 커밋 `87763c7a3bdb3095406599a3d16c5799fbbad2e8` 기준. 정답 정정 2건은 contract 문서와 Git 이력에 기록했다.

| 정답 | SHA-256 |
|---|---|
| [arrays.expected](https://github.com/kimjindol2025/freelang-v11-fx/blob/87763c7a3bdb3095406599a3d16c5799fbbad2e8/tests/l08-phase2/arrays.expected) | `57e6eee713d8945c5c9957106d7343cf1a331f4e4e631259febe87f085b7a072` |
| [bytes.expected](https://github.com/kimjindol2025/freelang-v11-fx/blob/87763c7a3bdb3095406599a3d16c5799fbbad2e8/tests/l08-phase2/bytes.expected) | `ea3a554c8e0b73c07d60bf4a1d357e74215ab58d7698dde7b1f373b42dee8d81` |
| [contracts.expected](https://github.com/kimjindol2025/freelang-v11-fx/blob/87763c7a3bdb3095406599a3d16c5799fbbad2e8/tests/l08-phase2/contracts.expected) | `fef0b676b7e48446d398d5576d440ab53e13ff52666910213e2e522f56d0292f` |
| [core.expected](https://github.com/kimjindol2025/freelang-v11-fx/blob/87763c7a3bdb3095406599a3d16c5799fbbad2e8/tests/l08-phase2/core.expected) | `09b6897884e1ef1ffc9d91cdbb4f371096ec2651fd49312692c6252049b54167` |
| [edges.expected](https://github.com/kimjindol2025/freelang-v11-fx/blob/87763c7a3bdb3095406599a3d16c5799fbbad2e8/tests/l08-phase2/edges.expected) | `6b064542067830c088e25bb9767669525d367fc1b364d23fe86fa22783959c82` |
| [literals.expected](https://github.com/kimjindol2025/freelang-v11-fx/blob/87763c7a3bdb3095406599a3d16c5799fbbad2e8/tests/l08-phase2/literals.expected) | `e189b8b3121e4dcfe0218ca9ea5f98dc880e2defdb603bea6006fab5993ec75c` |
| [numbers.expected](https://github.com/kimjindol2025/freelang-v11-fx/blob/87763c7a3bdb3095406599a3d16c5799fbbad2e8/tests/l08-phase2/numbers.expected) | `b978cb2d9674bb13ef52a465a76d617594026155e90c3ed335ff54f382c4ebc2` |
| [output.expected](https://github.com/kimjindol2025/freelang-v11-fx/blob/87763c7a3bdb3095406599a3d16c5799fbbad2e8/tests/l08-phase2/output.expected) | `36e82322bdcde27cd005c85f2272798ed02c3945f5f2f52f7fdbf51b0d2ae7fa` |
| [source-edge.expected](https://github.com/kimjindol2025/freelang-v11-fx/blob/87763c7a3bdb3095406599a3d16c5799fbbad2e8/tests/l08-phase2/source-edge.expected) | `3803538c52bdf62c4af6e185be453257dcb8bcac2e02c87a7fd833a25920c598` |
| [strings.expected](https://github.com/kimjindol2025/freelang-v11-fx/blob/87763c7a3bdb3095406599a3d16c5799fbbad2e8/tests/l08-phase2/strings.expected) | `f5bc7262208d2229a1121040ff724cae8eeb904003e03d4e1911f92fd68d9b0f` |
| [unicode.expected](https://github.com/kimjindol2025/freelang-v11-fx/blob/87763c7a3bdb3095406599a3d16c5799fbbad2e8/tests/l08-phase2/unicode.expected) | `adc0650ca8b8ff84e44a044aeecb6cca39c4aaf6749429aa37527b28b51493a4` |

경계 시험까지의 기존 9개 manifest와 최종 11개 manifest 모두 `sha256sum -c`에서 전부 OK였다. 중간 10개 manifest의 contracts 정답은 소스 근거로 공개 정정했으므로 그 manifest가 불변이라고 주장하지 않는다. 기존 L06·L07·L08 1차 정답·시험 소스와 기존 native bootstrap 검증기는 시작점 대비 diff가 없다.
