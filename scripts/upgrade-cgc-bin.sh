#!/usr/bin/env bash
# Replace shared freelang-afj/bin/cgc-bin with the candidate from smoke-cgc-upgrade.sh.
# Requires: /tmp/fx-cgc-bin-candidate from a PASS smoke run (or rebuilds if missing).
set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
TARGET="${CGC_BIN_TARGET:-$ROOT_DIR/../freelang-afj/bin/cgc-bin}"
CANDIDATE="${1:-/tmp/fx-cgc-bin-candidate}"
EXPECT_SHA="01cf5f5b6c25863f46e0aa4c0d12daac1f140df3f3907ffb9a40bbd270ca4ebb"
RUNTIME_DIR="${RUNTIME_DIR:-$ROOT_DIR/runtime}"

if [[ ! -x "$CANDIDATE" ]]; then
  echo "UPGRADE_CGC=FAIL missing candidate $CANDIDATE — run scripts/smoke-cgc-upgrade.sh first"
  exit 1
fi

# Verify candidate still produces fixpoint SHA
TMP=$(mktemp -d /tmp/fx-cgc-upgrade-XXXXXX)
trap 'rm -rf -- "$TMP"' EXIT
"$CANDIDATE" "$ROOT_DIR/self/cgc-main.fl" "$TMP/out.c" >/tmp/fx-cgc-upgrade-verify.log 2>&1
GOT="$(sha256sum "$TMP/out.c" | awk '{print $1}')"
echo "CANDIDATE_COMPILES_TO=$GOT"
if [[ "$GOT" != "$EXPECT_SHA" ]]; then
  echo "UPGRADE_CGC=FAIL candidate does not emit expected fixpoint SHA"
  exit 1
fi

if [[ ! -f "$TARGET" ]]; then
  echo "UPGRADE_CGC=FAIL target missing $TARGET"
  exit 1
fi

STAMP="$(date +%Y%m%d-%H%M%S)"
BAK="${TARGET}.bak-${STAMP}"
OLD_SHA="$(sha256sum "$TARGET" | awk '{print $1}')"
NEW_SHA="$(sha256sum "$CANDIDATE" | awk '{print $1}')"

echo "TARGET=$TARGET"
echo "OLD_BIN_SHA=$OLD_SHA"
echo "NEW_BIN_SHA=$NEW_SHA"
echo "BACKUP=$BAK"

cp -f "$TARGET" "$BAK"
cp -f "$CANDIDATE" "$TARGET"
chmod +x "$TARGET"

# Post-check: fl-build hello without CGC_BIN override
HELLO_OUT="$TMP/hello-post.elf"
if ! bash "$ROOT_DIR/fl-build.sh" "$ROOT_DIR/templates/hello/server.fl" "$HELLO_OUT" >/tmp/fx-cgc-upgrade-hello.log 2>&1; then
  echo "UPGRADE_CGC=FAIL post hello build — restoring backup"
  cp -f "$BAK" "$TARGET"
  exit 1
fi

echo "UPGRADE_CGC=PASS"
echo "RESTORED_ON_FAIL=no"
echo "FIXPOINT_SHA=$EXPECT_SHA"
echo "BACKUP_KEPT=$BAK"
