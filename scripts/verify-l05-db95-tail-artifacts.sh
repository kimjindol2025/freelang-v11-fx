#!/usr/bin/env bash
set -Eeuo pipefail
out="${1:?completed native database tail test output required}"
forensic=0;prefix=0
for kind in prepare-partial commit-partial no-newline invalid-utf8 nul;do
 root="$out/$kind"
 hash=$(sha256sum "$root/original-tail.wal" | cut -d' ' -f1)
 if cmp -s "$root/original-tail.wal" "$root/db.wal.recovery-$hash";then
  echo "DB95_TAIL_FORENSIC=$kind PASS";forensic=$((forensic+1))
 else echo "DB95_TAIL_FORENSIC=$kind FAIL";fi
 expected="$root/seed.wal"
 [[ "$kind" != no-newline ]] || expected="$root/original-tail.wal"
 size=$(wc -c < "$expected")
 if head -c "$size" "$root/db.wal" | cmp -s - "$expected";then
  echo "DB95_TAIL_PREFIX=$kind PASS";prefix=$((prefix+1))
 else echo "DB95_TAIL_PREFIX=$kind FAIL";fi
done
printf 'DB95_TAIL_FORENSIC_PASS=%s FAIL=%s\n' "$forensic" "$((5-forensic))"
printf 'DB95_TAIL_PREFIX_PASS=%s FAIL=%s\n' "$prefix" "$((5-prefix))"
[[ "$forensic" == 5 && "$prefix" == 5 ]]
