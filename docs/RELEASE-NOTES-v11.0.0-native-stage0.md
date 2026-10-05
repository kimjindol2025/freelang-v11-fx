# FreeLang FX v11.0.0-native-stage0

> 릴리즈 노트 초안 — 공식화 커밋 `c18b9e2561acc8845608d5a1b30f0d84b934d6b9` 기준

FreeLang FX의 Native Stage 0 bootstrap을 공식 compiler 경로로 승격합니다.
검증된 C seed와 시스템 C compiler만으로 native compiler chain을 재생성할 수
있으며, Stage 0 실행 이후의 compiler chain에는 Node.js, npm, TypeScript,
JavaScript runtime, 기존 `cgc-bin`, AFJ가 참여하지 않습니다.

## 주요 변경

- Native Stage 0 fixed-point bootstrap을 공식 검증 경로로 지정
- 필수 CI gate 추가: `.github/workflows/native-stage0.yml`
- 검증 명령 추가: `./scripts/verify-native-bootstrap.sh`
- 기본 compiler 설치 경로 지정: `bootstrap/stage0-bin`
- `fl-build.sh`가 `CGC_BIN` 미지정 시 Native Stage 0를 자동 설치·사용
- 명시적 대체 compiler 경로는 `CGC_BIN=/path/to/compiler`로 지정 가능
- 공식 상태 문서 추가: [`NATIVE-STAGE0.md`](NATIVE-STAGE0.md)

## 검증 범위

```text
stage0.c → stage1.c → stage2.c → stage3.c
```

- `stage2 == stage3` canonical SHA fixed point
- 배열형 loop 회귀 테스트 통과
- clean checkout 재현
- Node/npm/TypeScript/JavaScript runtime 호출 0건
- 기존 `cgc-bin` 및 AFJ 파일 접근 0건

재현 명령:

```bash
./scripts/verify-native-bootstrap.sh
```

## 사용 예

```bash
./fl-build.sh app/server.fl app-bin
```

네트워크 의존성 없이 빌드하려면:

```bash
./fl-build.sh app/server.fl app-bin --no-net
```

## 범위와 제한

- 이번 릴리즈는 Native Stage 0 bootstrap 경로를 공식화하는 릴리즈입니다.
- 언어 문법, 런타임 API, 애플리케이션 호환성의 전면 개편을 포함하지 않습니다.
- 기존 compiler를 자동으로 삭제하거나 덮어쓰지 않습니다.
- Native Stage 0의 공식 선언과 상세 provenance는
  [`NATIVE-STAGE0.md`](NATIVE-STAGE0.md)에서 확인합니다.

## 발행 전 확인

이 문서 작성 시점의 체크아웃에서는 공식화 커밋의 파일과
`v11.0.0-native-stage0` 태그가 현재 작업 트리에 확인되지 않았습니다. 실제
릴리즈 발행 전에 다음을 확인해야 합니다.

```bash
git show c18b9e2561acc8845608d5a1b30f0d84b934d6b9
git tag v11.0.0-native-stage0
./scripts/verify-native-bootstrap.sh
```

