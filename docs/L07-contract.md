# L07 바이너리 파일 계약

이 문서는 구현 전 실패 재현 시험과 함께 고정한 계약이다. 블로그를 불러오지 않는 네이티브 FreeLang 시험으로 검증한다.

## 재현 정답

입력 바이트는 `41 00 42`이며 3바이트다. SHA-256은 `76fe3925c7167317f2df68454339f5ec3650e4062178b4f2be219b105a507907`이다. `file-write` 뒤에도 3바이트여야 하고, `50` 한 바이트 뒤에 append하면 `50 41 00 42`의 4바이트가 돼야 한다. 출력은 독립 fixture와 바이트 단위로 비교한다. `tests/l07/repro.expected`는 수정 후에도 바꾸지 않는다.

## 기존 호출과 결과 API

기존 `file-read`·`_fl_file_read`는 문자열 또는 nil, `file-write`·`_fl_file_write`는 nil, `file-append`·`_fl_file_append` 및 `_fl_file_copy`는 불리언의 반환 형식을 보존한다. 정상 문자열·숫자 표시를 쓰던 legacy write도 유지한다. 문자열 파일 데이터에는 실제 FLString 길이를 사용한다. 일반 텍스트 함수의 NUL 처리까지 변경하는 과제는 아니다.

쓰기의 nil만으로 성공과 실패를 구분할 수 없고 나머지 호출도 실패 원인·처리 바이트 수가 부족하므로, 공유 파일 처리 위에 다음 결과 API만 추가한다. 새 bytes 별칭이나 별도의 설정 프레임워크는 만들지 않는다.

| FreeLang 함수 | 인자 | 결과 |
|---|---|---|
| `file-read-result` | path | 성공 시 원본 바이트를 가진 `data`, `bytes` |
| `file-write-result` | path, data | 성공 시 전부 기록한 `bytes` |
| `file-append-result` | path, data | 성공 시 추가 기록한 `bytes` |
| `file-copy-result` | source, destination | 성공 시 복사한 `bytes` |

공통 결과는 `ok`(불리언), `bytes`(실제 처리량), `error`(고정 코드, 성공 시 빈 문자열), `errno`(성공 시 0), `data`(읽기 성공 시 문자열, 그 외 nil)다. checked data는 명시적 길이를 가진 FLString이며 잘못된 자료형을 거부한다. 실패 결과에 비밀·경로를 넣지 않는다.

오류 코드는 `INVALID_PATH`, `INVALID_DATA`, `PATH_OUTSIDE_BASE`, `OPEN_FAILED`, `UNSUPPORTED_FILE`, `SIZE_LIMIT`, `ALLOCATION_FAILED`, `READ_FAILED`, `WRITE_FAILED`, `CLOSE_FAILED`, `SAME_FILE`, `FILE_CHANGED`다. 각 코드의 실제 발생 지점과 OS 오류 번호를 보고한다. 짧은 성공 반환은 남은 바이트를 처리하고, 진행 없는 처리·I/O 오류·flush·close 실패는 실패로 반환한다.

## 경계

한 파일의 상한은 64 MiB다. 읽기·쓰기·복사와 append 후 총 크기에서 확인하고 상한 초과·정수 범위·할당 실패를 검사한다. 경로는 비어 있지 않은 문자열이며 시스템 경로 길이 범위 안에 있어야 하고 내장 NUL을 허용하지 않는다. 데이터는 내장 NUL·비UTF-8을 그대로 보존한다. 잘못된 입력이나 기존 비정규 파일을 대상으로 열어 자르지 않는다. 같은 inode로 복사하는 경우도 자르기 전에 실패한다.

`FL_FILE_BASE`가 설정된 경우 해당 기존 디렉터리를 파일 루트로 사용한다. 상대 경로는 루트 기준, 절대 경로는 그 루트 안에서만 허용한다. 상위 경로·심볼릭 링크를 통한 이탈은 거부하며 디렉터리 fd를 기준으로 no-follow 열기를 사용한다. 변수를 설정하지 않은 기존 프로그램에는 새 루트 제한을 적용하지 않으며 운영체제 권한 범위에서 동작한다. 이 경우 언어가 샌드박스를 제공한다고 보고하지 않는다.

이 루트 제한은 여기서 수정한 read·write·append·copy와 그 결과 API에 적용한다. delete·rename 등 다른 API까지 포함하는 언어 전체 샌드박스는 아니다. 실패 결과의 `bytes`는 stdio가 처리·수락한 양이며 flush·close 실패 시 디스크에 보존된 길이나 내구성을 보장하는 수치가 아니다.

새 파일의 권한은 기존 open/umask 규칙을 유지한다. 일반 파일을 대상으로 하며 특수 파일·디렉터리는 거부한다. 중간 실패에서 출력 일부가 남을 수 있다. 이 계약은 원자적 파일 교체·fsync·WAL 거래를 보장하지 않는다. 해당 보장은 L05 등의 별도 계약이다.

## 자체 시험 범위

빈 파일·일반 텍스트·앞/중간/뒤 NUL·0~255·비UTF-8·8191/8192/8193·다중 블록, checked·legacy·별칭, append·copy·원본 불변, 잘못된 자료형·경로·심볼릭 링크·같은 파일, 크기 상한·할당·짧은 I/O·I/O 오류·flush·close 실패를 시험한다. fault injection은 시험 바이너리에서만 수행하고 운영 런타임에 시험용 스위치를 추가하지 않는다. 네이티브 부트스트랩 검증과 기존 런타임 회귀도 별도로 실행한다. 일부 미실행이면 완료로 보고하지 않는다.
