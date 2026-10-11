#!/usr/bin/env bash
# HTTP smoke for trap-server-json-string.
# PASS when /map (map direct) returns non-empty JSON with ok=true (trap resolved),
# and /str also works. Exit non-zero on empty/false-200 style failure.
set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd -P)"
SRC="$ROOT_DIR/tests/traps/trap-server-json-string.fl"
OUT_BIN="${TMPDIR:-/tmp}/fx-trap-server-json-$$"
PORT="${TRAP_SERVER_JSON_PORT:-18091}"
LOG_BUILD="/tmp/fx-trap-server-json-build.log"
LOG_RUN="/tmp/fx-trap-server-json-run.log"

cleanup() {
  if [[ -n "${SRV_PID:-}" ]] && kill -0 "$SRV_PID" 2>/dev/null; then
    kill "$SRV_PID" 2>/dev/null || true
    wait "$SRV_PID" 2>/dev/null || true
  fi
  rm -f -- "$OUT_BIN"
}
trap cleanup EXIT

# pick free port if busy
if ss -ltn 2>/dev/null | grep -q ":${PORT} "; then
  PORT=$((18091 + RANDOM % 200))
fi

cd "$ROOT_DIR"
if ! bash ./fl-build.sh "$SRC" "$OUT_BIN" >"$LOG_BUILD" 2>&1; then
  echo "TRAP_SERVER_JSON_STRING=FAIL build"
  tail -30 "$LOG_BUILD" >&2 || true
  exit 1
fi

TRAP_SERVER_JSON_PORT="$PORT" "$OUT_BIN" >"$LOG_RUN" 2>&1 &
SRV_PID=$!

# wait for listen
ok_listen=0
for _ in $(seq 1 50); do
  if curl -sS -m 0.3 "http://127.0.0.1:${PORT}/str" >/dev/null 2>&1; then
    ok_listen=1
    break
  fi
  if ! kill -0 "$SRV_PID" 2>/dev/null; then
    echo "TRAP_SERVER_JSON_STRING=FAIL server exited early"
    cat "$LOG_RUN" >&2 || true
    exit 1
  fi
  sleep 0.1
done
if [[ "$ok_listen" -ne 1 ]]; then
  echo "TRAP_SERVER_JSON_STRING=FAIL listen timeout port=$PORT"
  cat "$LOG_RUN" >&2 || true
  exit 1
fi

fetch() {
  local path="$1"
  local hdr body code clen
  hdr="$(mktemp)"
  body="$(mktemp)"
  code="$(curl -sS -D "$hdr" -o "$body" -w '%{http_code}' "http://127.0.0.1:${PORT}${path}" || echo 000)"
  clen="$(awk 'BEGIN{IGNORECASE=1} /^Content-Length:/{print $2}' "$hdr" | tr -d '\r')"
  printf '%s\n' "$code|$clen|$(cat "$body")"
  rm -f "$hdr" "$body"
}

MAP_R="$(fetch /map)"
STR_R="$(fetch /str)"
VEC_R="$(fetch /vec)"

echo "MAP=$MAP_R"
echo "STR=$STR_R"
echo "VEC=$VEC_R"

map_code="${MAP_R%%|*}"; rest="${MAP_R#*|}"; map_clen="${rest%%|*}"; map_body="${rest#*|}"
str_code="${STR_R%%|*}"; rest="${STR_R#*|}"; str_clen="${rest%%|*}"; str_body="${rest#*|}"
vec_code="${VEC_R%%|*}"; rest="${VEC_R#*|}"; vec_clen="${rest%%|*}"; vec_body="${rest#*|}"

fail=0
# Old trap symptom: HTTP 200 + empty body / Content-Length 0 on map-direct
if [[ "$map_code" != "200" || -z "$map_body" || "$map_clen" == "0" ]]; then
  echo "STATUS map-direct=STILL_BROKEN code=$map_code clen=$map_clen body_len=${#map_body}"
  fail=1
elif echo "$map_body" | grep -q '"ok"[[:space:]]*:[[:space:]]*true' || echo "$map_body" | grep -q '"ok":true'; then
  echo "STATUS map-direct=OK"
else
  echo "STATUS map-direct=UNEXPECTED_BODY $map_body"
  fail=1
fi

if [[ "$str_code" != "200" || -z "$str_body" ]]; then
  echo "STATUS stringify=FAIL"
  fail=1
else
  echo "STATUS stringify=OK"
fi

if [[ "$vec_code" != "200" || -z "$vec_body" || "$vec_clen" == "0" ]]; then
  echo "STATUS vector-direct=STILL_BROKEN"
  fail=1
else
  echo "STATUS vector-direct=OK"
fi

if [[ "$fail" -eq 0 ]]; then
  echo "TRAP_SERVER_JSON_STRING=PASS"
  echo "INTERPRETATION=runtime server_json auto-stringifies map/vector; old empty-body trap resolved"
  echo "REGRESSION=tests/traps/trap-server-json-string.fl"
  echo "COMMAND=bash tests/traps/run-trap-server-json-string.sh"
  exit 0
fi

echo "TRAP_SERVER_JSON_STRING=FAIL"
echo "INTERPRETATION=map-direct still empty or non-JSON — trap remains active"
exit 1
