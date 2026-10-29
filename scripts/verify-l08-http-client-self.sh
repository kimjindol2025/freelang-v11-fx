#!/usr/bin/env bash
set -Eeuo pipefail
umask 077
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_HTTP_CLIENT_OUT:-$(mktemp -d)}"
mkdir -p "$out/root"
command -v openssl >/dev/null || { echo HTTP_CLIENT_TLS=UNEXECUTED_MISSING_OPENSSL;exit 2; }
printf 'A\377B\000C' > "$out/root/raw.bin"
l07_build_compiler "$out" > "$out/compiler-build.log" 2>&1
l07_build_app "$out" "$ROOT/tests/l08-platform/http-client.fl" client > "$out/client-build.log" 2>&1
for name in http-client-boundaries http-client-raw;do l07_build_app "$out" "$ROOT/tests/l08-platform/$name.fl" "$name" > "$out/$name-build.log" 2>&1;done
"$out/compiler" "$ROOT/tests/l08-platform/http-client-server.fl" "$out/server.c" > "$out/server-build.log" 2>&1
gcc -O2 -w -I "$ROOT/runtime" -o "$out/server" "$out/server.c" "$ROOT/tests/l08/http-port.c" "${L07_SRCS[@]}" -Wl,--wrap=listen "${L07_LIBS[@]}" >> "$out/server-build.log" 2>&1
pid=""
stop_server(){ if [[ -n "$pid" ]];then kill "$pid" 2>/dev/null || true;wait "$pid" 2>/dev/null || true;pid="";fi; }
trap stop_server EXIT
start_server(){
 local label="$1";shift
 mkdir -p "$out/$label"
 L08_PORT_FILE="$out/$label/port" FL_WORKERS=4 FL_DEBUG=2 FL_LOG_FILE="$out/$label/debug.log" FL_FILE_BASE="$out/root" "$out/server" "$out/root" "$@" > "$out/$label/stdout" 2> "$out/$label/stderr" &
 pid=$!
 for attempt in {1..200};do [[ -s "$out/$label/port" ]] && break;sleep .05;done
 [[ -s "$out/$label/port" ]]
 port=$(cat "$out/$label/port")
}
pass=0;fail=0
run_case(){
 local label="$1" expected="$2";shift 2
 if "$@" > "$out/$label.actual" 2> "$out/$label.stderr";then rc=0;else rc=$?;fi
 cat "$out/$label.actual" "$out/$label.stderr"
 printf '%s_PROGRAM_EXIT=%s\n' "$label" "$rc"
 if [[ "$rc" == 0 && ! -s "$out/$label.stderr" ]] && diff -u "$ROOT/tests/l08-platform/$expected.expected" "$out/$label.actual";then pass=$((pass+1));else fail=$((fail+1));fi
}
start_server http
run_case HTTP_CLIENT http-client "$out/client" "http://127.0.0.1:$port" http
curl -q --noproxy '*' -fsS "http://127.0.0.1:$port/raw" -o "$out/raw-wire.bin"
if cmp -s "$out/raw-wire.bin" "$out/root/raw.bin";then echo RAW_SERVER_WIRE_EXACT=PASS;pass=$((pass+1));else echo RAW_SERVER_WIRE_EXACT=FAIL;fail=$((fail+1));fi
run_case HTTP_CLIENT_RAW http-client-raw "$out/http-client-raw" "http://127.0.0.1:$port"
run_case HTTP_CLIENT_BOUNDARIES http-client-boundaries "$out/http-client-boundaries" "http://127.0.0.1:$port"
stop_server
openssl req -x509 -newkey rsa:2048 -nodes -days 1 -subj /CN=localhost -addext subjectAltName=DNS:localhost -keyout "$out/root/key.pem" -out "$out/root/cert.pem" > "$out/cert-build.log" 2>&1
start_server tls tls "$out/root/cert.pem" "$out/root/key.pem"
run_case HTTP_CLIENT_TLS_UNTRUSTED http-client-tls-untrusted env -u SSL_CERT_FILE -u CURL_CA_BUNDLE "$out/client" "https://localhost:$port" tls-untrusted
run_case HTTP_CLIENT_TLS_TRUSTED http-client-tls-trusted env SSL_CERT_FILE="$out/root/cert.pem" "$out/client" "https://localhost:$port" tls-trusted "https://127.0.0.1:$port"
stop_server
printf 'L08_HTTP_CLIENT_PASS=%s FAIL=%s\n' "$pass" "$fail"
[[ "$fail" == 0 ]]
