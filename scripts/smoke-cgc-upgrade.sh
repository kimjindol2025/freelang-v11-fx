#!/usr/bin/env bash
# Build a candidate cgc-bin from current self/cgc-main.fl and compile smoke apps with it.
# Does NOT replace the shared freelang-afj/bin/cgc-bin.
set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
RUNTIME_DIR="${RUNTIME_DIR:-$ROOT_DIR/runtime}"
SEED_CGC="${CGC_BIN:-$ROOT_DIR/../freelang-afj/bin/cgc-bin}"
TMP="${TMPDIR:-/tmp}/fx-cgc-smoke-$$"
CANDIDATE="$TMP/cgc-bin-candidate"
# Empty = accept whatever SHA the current self/cgc-main.fl converges to (gen-a must equal gen-b).
EXPECT_SHA="${EXPECT_FIXPOINT_SHA:-}"

mkdir -p "$TMP"
cleanup() { rm -rf -- "$TMP"; }
trap cleanup EXIT

RUNTIME_SRCS=(
  "$RUNTIME_DIR/core.c" "$RUNTIME_DIR/collection.c" "$RUNTIME_DIR/io.c"
  "$RUNTIME_DIR/math.c" "$RUNTIME_DIR/error.c" "$RUNTIME_DIR/process.c"
  "$RUNTIME_DIR/json.c" "$RUNTIME_DIR/aliases.c" "$RUNTIME_DIR/user-fns.c"
  "$RUNTIME_DIR/cgc-bridge.c" "$RUNTIME_DIR/gc.c" "$RUNTIME_DIR/http.c"
  "$RUNTIME_DIR/websocket.c" "$RUNTIME_DIR/sqlite.c" "$RUNTIME_DIR/mariadb.c"
  "$RUNTIME_DIR/debug.c" "$RUNTIME_DIR/http_client.c" "$RUNTIME_DIR/regex.c"
  "$RUNTIME_DIR/smtp.c" "$RUNTIME_DIR/sse.c"
)

APPS=(
  "templates/hello/server.fl"
  "fx-demo/server.fl"
  "fx-kv/server.fl"
  "fx-auth/server.fl"
  "fx-chat/server.fl"
  "fx-world/server.fl"
  "fx-cron/server.fl"
  "fx-queue/server.fl"
  "fx-sse-demo/server.fl"
  "apps/fx-short/server.fl"
)

cd "$ROOT_DIR"
echo "SMOKE_CGC_UPGRADE=START"
echo "SEED_CGC=$SEED_CGC"

if [[ ! -x "$SEED_CGC" ]]; then
  echo "SMOKE_CGC_UPGRADE=FAIL missing seed cgc"
  exit 1
fi

echo "Building candidate from self/cgc-main.fl ..."
"$SEED_CGC" "$ROOT_DIR/self/cgc-main.fl" "$TMP/gen-a.c" >/tmp/fx-cgc-smoke-seed.log 2>&1
GEN_SHA="$(sha256sum "$TMP/gen-a.c" | awk '{print $1}')"
echo "GEN_A_SHA=$GEN_SHA"
if [[ -n "$EXPECT_SHA" && "$GEN_SHA" != "$EXPECT_SHA" ]]; then
  echo "SMOKE_CGC_UPGRADE=FAIL gen-a SHA mismatch want=$EXPECT_SHA"
  exit 1
fi

gcc -O2 -w -I "$RUNTIME_DIR" -o "$CANDIDATE" "$TMP/gen-a.c" "${RUNTIME_SRCS[@]}" \
  -lm -lpthread -ldl -lsqlite3 -lssl -lcrypto -lcurl
chmod +x "$CANDIDATE"
# second generation from candidate
"$CANDIDATE" "$ROOT_DIR/self/cgc-main.fl" "$TMP/gen-b.c" >/tmp/fx-cgc-smoke-genb.log 2>&1
GEN_B_SHA="$(sha256sum "$TMP/gen-b.c" | awk '{print $1}')"
echo "GEN_B_SHA=$GEN_B_SHA"
if [[ "$GEN_B_SHA" != "$GEN_SHA" ]]; then
  echo "SMOKE_CGC_UPGRADE=FAIL candidate not at fixpoint"
  exit 1
fi
BIN_SHA="$(sha256sum "$CANDIDATE" | awk '{print $1}')"
echo "CANDIDATE_BIN_SHA=$BIN_SHA"

PASS=0
FAIL=0
for rel in "${APPS[@]}"; do
  src="$ROOT_DIR/$rel"
  name="$(echo "$rel" | tr '/' '_')"
  out="$TMP/${name}.elf"
  log="$TMP/${name}.log"
  if [[ ! -f "$src" ]]; then
    echo "COMPILE $rel=MISSING"
    FAIL=$((FAIL + 1))
    continue
  fi
  if CGC_BIN="$CANDIDATE" bash "$ROOT_DIR/fl-build.sh" "$src" "$out" >"$log" 2>&1; then
    echo "COMPILE $rel=PASS"
    PASS=$((PASS + 1))
  else
    echo "COMPILE $rel=FAIL"
    tail -15 "$log" | sed 's/^/  /'
    FAIL=$((FAIL + 1))
  fi
done

# HTTP smoke: hello on ephemeral port if we can rewrite — skip live ports;
# just ensure hello binary runs --help or starts briefly on high port via env if supported.
# Minimal: binary exists and is executable for hello (already counted in COMPILE).

echo "SMOKE_SUMMARY compile_pass=$PASS compile_fail=$FAIL total=${#APPS[@]}"
if [[ "$FAIL" -eq 0 ]]; then
  echo "SMOKE_CGC_UPGRADE=PASS"
  echo "NEXT=bash scripts/upgrade-cgc-bin.sh   # replaces shared freelang-afj/bin/cgc-bin"
  # keep candidate path for upgrade script via stamp file outside TMP? write to /tmp stable
  cp -f "$CANDIDATE" /tmp/fx-cgc-bin-candidate
  echo "$BIN_SHA" >/tmp/fx-cgc-bin-candidate.sha256
  echo "CANDIDATE_COPIED=/tmp/fx-cgc-bin-candidate"
  exit 0
fi
echo "SMOKE_CGC_UPGRADE=FAIL"
exit 1
