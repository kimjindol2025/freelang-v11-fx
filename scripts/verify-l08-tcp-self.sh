#!/usr/bin/env bash
set -Eeuo pipefail
umask 077
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_TCP_OUT:-$(mktemp -d)}"
mkdir -p "$out/root/pki" "$out/root/images"
command -v openssl >/dev/null || { echo TCP_TLS=UNEXECUTED_MISSING_OPENSSL;exit 2; }
command -v jq >/dev/null || { echo TCP_EVENT_AUDIT=UNEXECUTED_MISSING_JQ;exit 2; }
l07_build_compiler "$out" > "$out/compiler-build.log" 2>&1
for name in tcp-control tcp-conflict;do l07_build_app "$out" "$ROOT/tests/l08-platform/$name.fl" "$name" > "$out/$name-build.log" 2>&1;done
"$out/compiler" "$ROOT/tests/l08-platform/tcp-echo.fl" "$out/echo.c" > "$out/echo-build.log" 2>&1
gcc -O2 -w -I "$ROOT/runtime" -o "$out/echo" "$out/echo.c" "$ROOT/tests/l08/http-port.c" "$ROOT/tests/l08-platform/tcp-socket-pressure.c" "${L07_SRCS[@]}" -Wl,--wrap=listen -Wl,--wrap=accept -Wl,--wrap=send -Wl,--wrap=SSL_write "${L07_LIBS[@]}" >> "$out/echo-build.log" 2>&1
gcc -O2 -Wall -Wextra "$ROOT/tests/l08-platform/tcp-client.c" -o "$out/client" -lssl -lcrypto -lpthread > "$out/client-build.log" 2>&1
gcc -O2 -Wall -Wextra "$ROOT/tests/l08-phase3/fixtures.c" -o "$out/fixtures"
"$out/fixtures" "$out/root/images"
pass=0;fail=0
if (cd "$out/root";FL_FILE_BASE="$out/root" timeout 20 "$out/tcp-control") > "$out/control.actual" 2> "$out/control.stderr";then rc=0;else rc=$?;fi
cat "$out/control.actual" "$out/control.stderr";printf 'TCP_CONTROL_PROGRAM_EXIT=%s\n' "$rc"
if [[ "$rc" == 0 && ! -s "$out/control.stderr" ]] && diff -u "$ROOT/tests/l08-platform/tcp-control.expected" "$out/control.actual";then pass=$((pass+1));else fail=$((fail+1));fi
pki="$out/root/pki"
cat > "$pki/server.ext" <<'EXT'
basicConstraints=critical,CA:FALSE
keyUsage=critical,digitalSignature,keyEncipherment
extendedKeyUsage=serverAuth
subjectAltName=DNS:localhost
EXT
cat > "$pki/client.ext" <<'EXT'
basicConstraints=critical,CA:FALSE
keyUsage=critical,digitalSignature
extendedKeyUsage=clientAuth
EXT
for ca in ca other-ca;do
 openssl req -x509 -newkey rsa:2048 -nodes -days 1 -subj "/CN=$ca" -addext basicConstraints=critical,CA:TRUE -keyout "$pki/$ca.key" -out "$pki/$ca.pem" >> "$out/pki-build.log" 2>&1
done
for identity in server client other-client;do
 openssl req -new -newkey rsa:2048 -nodes -subj "/CN=$identity" -keyout "$pki/$identity.key" -out "$pki/$identity.csr" >> "$out/pki-build.log" 2>&1
 ca=ca;extension=client;[[ "$identity" != server ]] || extension=server;[[ "$identity" != other-client ]] || ca=other-ca
 openssl x509 -req -in "$pki/$identity.csr" -CA "$pki/$ca.pem" -CAkey "$pki/$ca.key" -CAcreateserial -days 1 -sha256 -extfile "$pki/$extension.ext" -out "$pki/$identity.pem" >> "$out/pki-build.log" 2>&1
done
( cd "$pki";sha256sum *.key *.pem ) > "$out/pki-before.sha256"
pid=""
stop_owned(){ if [[ -n "$pid" ]];then kill "$pid" 2>/dev/null || true;wait "$pid" 2>/dev/null || true;pid="";fi; }
trap stop_owned EXIT
start(){
 label="$1";shift;mkdir -p "$out/root/$label"
 L08_PORT_FILE="$out/root/$label/port" FL_FILE_BASE="$out/root" "$out/echo" "$out/root/$label" "$@" > "$out/$label.stdout" 2> "$out/$label.stderr" &
 pid=$!
 for attempt in {1..200};do [[ -s "$out/root/$label/port" ]] && break;sleep .05;done
 [[ -s "$out/root/$label/port" ]]
 port=$(cat "$out/root/$label/port")
}
client(){ timeout 90 "$out/client" "$1" "$port" "$out/root/images" "${2:-$pki/ca.pem}" "${3:-}" "${4:-}"; }
finish(){
 for attempt in {1..300};do kill -0 "$pid" 2>/dev/null || break;state=$(ps -p "$pid" -o stat= 2>/dev/null || true);[[ "$state" != Z* ]] || break;sleep .01;done
 if kill -0 "$pid" 2>/dev/null && [[ ${state:-} != Z* ]];then echo TCP_OWN_SERVER_EXIT_TIMEOUT=FAIL;return 1;fi
 if wait "$pid";then rc=0;else rc=$?;fi;pid=""
 printf '%s_SERVER_EXIT=%s\n' "$label" "$rc"
 [[ "$rc" == 0 ]]
 jq -e 'group_by(.id) | all(.[]; (map(select(.event=="connect"))|length)==1 and (map(select(.event=="close"))|length)==1 and .[0].event=="connect" and .[-1].event=="close")' "$out/root/$label/events.json" > "$out/$label-event-audit.log"
 echo "${label}_EVENT_ORDER=PASS"
}
start raw raw
FL_FILE_BASE="$out/root" "$out/tcp-conflict" "$port" > "$out/conflict.actual" 2> "$out/conflict.stderr"
cat "$out/conflict.actual";[[ ! -s "$out/conflict.stderr" ]]
client ping | tee "$out/raw-ping.actual"
if client raw | tee "$out/raw-client.actual" && finish;then pass=$((pass+1));else fail=$((fail+1));stop_owned;fi
if rg -q '^RAW_SEND_EAGAIN_COUNT=[1-9][0-9]*$' "$out/raw.stderr";then echo RAW_PARTIAL_WRITE_PRESSURE=PASS;pass=$((pass+1));else echo RAW_PARTIAL_WRITE_PRESSURE=FAIL;fail=$((fail+1));fi
start raw-restart raw
if client raw | tee "$out/raw-restart-client.actual" && finish;then echo RAW_PROCESS_RESTART=PASS;pass=$((pass+1));else fail=$((fail+1));stop_owned;fi
start tls tls "$pki/server.pem" "$pki/server.key" "" optional-client
client tls-reject "$pki/other-ca.pem" | tee "$out/tls-untrusted.actual"
client tls-hostname | tee "$out/tls-hostname.actual"
client abort | tee "$out/tls-abort.actual"
client tls-ping | tee "$out/tls-recovered.actual"
if client tls | tee "$out/tls-client.actual" && finish;then pass=$((pass+1));else fail=$((fail+1));stop_owned;fi
if rg -q '^SSL_WRITE_WANT_COUNT=[1-9][0-9]*$' "$out/tls.stderr";then echo TLS_PARTIAL_WRITE_PRESSURE=PASS;pass=$((pass+1));else echo TLS_PARTIAL_WRITE_PRESSURE=FAIL;fail=$((fail+1));fi
start mtls tls "$pki/server.pem" "$pki/server.key" "$pki/ca.pem" require-client
client tls-reject | tee "$out/mtls-missing.actual"
client tls-reject "$pki/ca.pem" "$pki/other-client.pem" "$pki/other-client.key" | tee "$out/mtls-other-ca.actual"
if client tls "$pki/ca.pem" "$pki/client.pem" "$pki/client.key" | tee "$out/mtls-client.actual" && finish;then pass=$((pass+1));else fail=$((fail+1));stop_owned;fi
( cd "$pki";sha256sum *.key *.pem ) > "$out/pki-after.sha256"
if cmp -s "$out/pki-before.sha256" "$out/pki-after.sha256";then echo TEST_PKI_FILES_UNCHANGED=PASS;pass=$((pass+1));else fail=$((fail+1));fi
printf 'L08_TCP_PASS=%s FAIL=%s\n' "$pass" "$fail"
[[ "$fail" == 0 ]]
