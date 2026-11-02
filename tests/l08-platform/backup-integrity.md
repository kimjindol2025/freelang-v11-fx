# 같은 크기인 이미지 손상과 복구 전 검사

실제 blog-backup-export/import를 사용한다. 새 합성 글·한글/이모지 제목과 image metadata 한 건, `GIF89a` prefix·NUL·ff를 가진 raw fixture를 내보낸다. 이 fixture는 이미지 decoder 시험이나 완전한 GIF 형식 검증을 뜻하지 않는다. 원본 백업은 그대로 보관하고 대상에 복사한 archive의 한 byte만 바꾸되 크기는 유지한다.

정답: 정상 복사본은 글·이미지 수 1·1, 글 내용과 raw 이미지 SHA-256을 복원한다. 손상 복사본은 BLOG_BACKUP_INVALID로 거부하고 복구 전 WAL 바이트·metadata·대상 이미지 부재를 유지한다. 새 원본 백업의 image SHA-256 검사가 필요하며 크기만 비교하는 것으로 완료를 판정하지 않는다. 기존 버전 1/2 archive 호환과 새 export의 version 2 계약은 유지한다. 해시 없는 과거 archive에는 이 추가 검증의 증거가 없다는 제한을 명시한다. manifest 자체의 서명·인증을 제공한다고 주장하지 않는다.

추가 신뢰 경계: null/빈 checksum map, 다른 image key, 잘못된 hex·불일치 hash, 잘린 이미지, 경로 이탈 ID, 다른 blog의 행, 예상 밖 파일은 모두 같은 사전 거부·대상 무변경 정답을 적용한다. 과거 version 2와의 호환은 복사한 정상 archive에서 추가 checksum 필드만 제거하고 정상 복원 정답으로 확인한다. 원본 archive는 수정하지 않는다. 기존 같은 소스의 seed/check 실행물을 재사용할 때는 L08_BACKUP_INTEGRITY_BIN_DIR을 명시하고 출처·SHA-256을 기록한다. 기본 실행은 여전히 native 빌드다.
