# Native Stage 0 공식 상태

FreeLang FX는 검증된 C seed와 시스템 C compiler만으로 native compiler를
재생성할 수 있다. JavaScript/TypeScript는 seed 생성 단계에만 사용되며,
stage0 실행 이후의 compiler chain에는 참여하지 않는다.

## 공식 증거

- 기준 seed: `bootstrap/stage0.c`
- seed 생성 provenance: `bootstrap/STAGE0-PROVENANCE.md`
- 검증기: `scripts/verify-native-bootstrap.sh`
- 회귀 입력: `tests/native-bootstrap/array-loop.fl`
- 공식 tag: `v11.0.0-native-stage0`

## GitHub Actions 검증 기록

GitHub master에서도 동일한 필수 gate를 실행해 PASS를 확인했다.

```text
WORKFLOW=Native Stage 0
RUN_ID=36330069714
HEAD=94a477402b7de8715519cf5c4ee4c680e4e5d859
STATUS=completed
CONCLUSION=success
DURATION=56s
```

실행 기록: https://github.com/kimjindol2025/freelang-v11-fx/actions/runs/36330069714

Checkout, 의존성 설치, `verify-native-bootstrap.sh`, Complete job이 모두
성공했다. Node.js 20 deprecated 및 Ubuntu 26 예정 안내는 runner 경고이며
검증 실패가 아니다.

검증 명령:

```bash
./scripts/verify-native-bootstrap.sh
```

검증 대상은 다음과 같다.

```text
stage0.c + runtime C → stage0-bin
stage0-bin + self/cgc-main.fl → stage1.c
stage1-bin + self/cgc-main.fl → stage2.c
stage2-bin + self/cgc-main.fl → stage3.c
```

stage1·stage2·stage3는 canonical fixed-point SHA로 수렴한다. stage0은
JS-assisted seed라 canonical SHA와 다를 수 있지만, stage2와 stage3가
동일하면 native fixed point를 통과한 것으로 판정한다.

검증 과정은 `PATH=/usr/bin:/bin`으로 native ELF를 실행하고 `execve` 및
`openat`를 기록한다. Node, npm, TypeScript/JavaScript runtime, 기존
`cgc-bin`, AFJ 파일 접근은 0건이어야 한다.

## 기본 compiler 설치 경로

기본 native compiler 경로는 다음이다.

```text
bootstrap/stage0-bin
```

필요하면 명시적으로 설치한다.

```bash
./scripts/install-native-stage0.sh
```

`fl-build.sh`는 `CGC_BIN` 환경변수가 없는 경우 이 Native Stage 0 경로를
자동으로 설치·사용한다. 기존 compiler를 임시로 지정해야 할 때만 다음처럼
명시한다.

```bash
CGC_BIN=/path/to/compiler bash fl-build.sh app/server.fl app-bin
```

이 경계는 기존 사용자 compiler를 자동 삭제하거나 덮어쓰지 않는다.
