#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_HTTP_PLATFORM_OUT:-$(mktemp -d)}"
mkdir -p "$out/root/static" "$out/root/outside"
printf KEEP > "$out/root/outside/sentinel"
gcc -O2 -Wall -Wextra "$ROOT/tests/l08-phase3/fixtures.c" -o "$out/fixtures"
"$out/fixtures" "$out/root/static"
gcc -O2 -Wall -Wextra "$ROOT/tests/l08-platform/http-platform-png.c" -o "$out/png"
"$out/png" "$out/root/static/image.png" "$out/root/static/all.png"
ln -s "$out/root/outside/sentinel" "$out/root/static/link.png"
l07_build_compiler "$out" > "$out/compiler-build.log" 2>&1
"$out/compiler" "$ROOT/tests/l08-platform/http-platform.fl" "$out/server.c" > "$out/server-build.log" 2>&1
gcc -O2 -w -I "$ROOT/runtime" -o "$out/server" "$out/server.c" "$ROOT/tests/l08/http-port.c" "${L07_SRCS[@]}" -Wl,--wrap=listen "${L07_LIBS[@]}" >> "$out/server-build.log" 2>&1
L08_PORT_FILE="$out/port" FL_FILE_BASE="$out/root" FL_DEBUG=2 FL_LOG_FILE="$out/http-debug.log" "$out/server" "$out/root" > "$out/server.stdout" 2> "$out/server.stderr" &
server_pid=$!
trap 'kill "$server_pid" 2>/dev/null || true;wait "$server_pid" 2>/dev/null || true' EXIT
for attempt in {1..200};do [[ -s "$out/port" ]] && break;sleep .05;done
[[ -s "$out/port" ]]
port=$(cat "$out/port");url="http://127.0.0.1:$port"
pass=0;fail=0
request(){ code=$(curl -q --noproxy '*' -sS --max-time 10 --path-as-is -D "$out/headers" -o "$out/body" -w '%{http_code}' "$@"); }
check(){ if "$@";then return 0;else return 1;fi; }
text_case(){
 local label="$1" method="$2" path="$3" expected_code="$4" expected_body="$5";shift 5
 request -X "$method" "$url$path" "$@"
 printf '%s=%s\n' "$label" "$code"
 if [[ "$code" == "$expected_code" && $(cat "$out/body") == "$expected_body" ]];then pass=$((pass+1));else printf 'FAIL %s\n' "$label";cat "$out/body";fail=$((fail+1));fi
}
text_case ORDER GET /order 200 first,second,handler
text_case FALSE_MIDDLEWARE GET /false 200 false
text_case GUARD_DENIED GET /guard/ok 401 DENIED
text_case GUARD_ALLOWED GET /guard/ok 200 GUARDED -H 'X-Allow: yes'
text_case GUARD_BEFORE_FALLBACK GET /guard/missing 401 DENIED
expected='{"id":"a%2Fb","q":"한글 tag","all":{"q":["한글 tag","second"],"flag":""},"header":"first, second","host":"Blog.Example:8080","cookie":"a=b","encoded":"a%20b","method":"GET","path":"/request/a%2Fb"}'
text_case PATH_QUERY_HEADERS GET '/request/a%2Fb?q=%ED%95%9C%EA%B8%80+tag&q=second&flag' 200 "$expected" -H 'Host: Blog.Example:8080' -H 'X-Repeat: first' -H 'x-repeat: second' -H 'Cookie: other=x; token= a=b ; encoded=a%20b'
pad=$(printf '%6000s' '' | tr ' ' x)
text_case LONG_QUERY GET "/long?pad=$pad&last=end" 200 '{"length":6000,"last":"end"}'
text_case LONG_PARAM GET "/param/$pad" 200 6000
text_case HTML GET /html 201 '<b>HTML</b>'
if rg -qi '^Content-Type: text/html; charset=utf-8\r?$' "$out/headers";then pass=$((pass+1));else fail=$((fail+1));fi
text_case JSON_CREATED GET /created 201 '{"n":0.1}'
text_case JSON_CONFLICT GET /conflict 409 '{"error":"conflict"}'
text_case JSON_ECHO POST /echo 200 '{"json":{"n":0.1,"__proto__":{"safe":true},"s":"\u0000"},"body":{"n":0.1,"__proto__":{"safe":true},"s":"\u0000"}}' -H 'Content-Type: application/json' --data-binary '{"n":0.1,"__proto__":{"safe":true},"s":"\u0000"}'
text_case JSON_MALFORMED POST /echo 200 '{"json":null,"body":"{"}' -H 'Content-Type: application/json' --data-binary '{'
text_case JSON_TEXT_FALSE POST /echo 200 '{"json":false,"body":"false"}' -H 'Content-Type: text/plain' --data-binary false
text_case JSON_NULL POST /echo 200 '{"json":null,"body":null}' -H 'Content-Type: application/json' --data-binary null
text_case RESPONSE_HEADER GET /header 200 BASE
if rg -qi '^X-Label: added\r?$' "$out/headers";then pass=$((pass+1));else fail=$((fail+1));fi
text_case IMMUTABLE_RESPONSE GET /base 200 BASE
if ! rg -qi '^X-Label:' "$out/headers";then pass=$((pass+1));else fail=$((fail+1));fi
text_case COOKIES GET /cookies 302 ''
if [[ $(rg -ic '^Set-Cookie:' "$out/headers") == 2 ]] && rg -q '^Location: /after\r?$' "$out/headers" && rg -q '^Set-Cookie: one=a%20b; Max-Age=0; Path=/; HttpOnly; SameSite=Lax\r?$' "$out/headers" && rg -q '^Set-Cookie: two=x%3Dy; Path=/; HttpOnly; Secure; SameSite=Strict\r?$' "$out/headers";then pass=$((pass+1));else cat "$out/headers";fail=$((fail+1));fi
text_case HEADER_INJECTION GET /bad-header 500 '{"error":"Internal error"}'
if ! rg -qi '^Injected:' "$out/headers";then pass=$((pass+1));else fail=$((fail+1));fi
text_case STATIC_GATE GET /assets/all.png 403 NO_IMAGE
for spec in all.png image.jpg image.gif;do
 request "$url/assets/$spec" -H 'X-Image-Allow: yes'
 type=image/png;[[ "$spec" != image.jpg ]] || type=image/jpeg;[[ "$spec" != image.gif ]] || type=image/gif
 size=$(wc -c < "$out/root/static/$spec")
 if [[ "$code" == 200 ]] && cmp -s "$out/root/static/$spec" "$out/body" && rg -qi "^Content-Type: $type\r?$" "$out/headers" && rg -q "^Content-Length: $size\r?$" "$out/headers";then echo "STATIC_$spec=PASS";pass=$((pass+1));else echo "STATIC_$spec=FAIL";fail=$((fail+1));fi
done
text_case STATIC_TRAVERSAL GET /assets/../outside/sentinel 403 Forbidden -H 'X-Image-Allow: yes'
text_case STATIC_SYMLINK GET /assets/link.png 403 Forbidden -H 'X-Image-Allow: yes'
text_case STATIC_MISSING GET /assets/missing 404 'Not found' -H 'X-Image-Allow: yes'
large=$(printf '%513s' '' | tr ' ' x)
request -X POST "$url/echo" -H 'Content-Type: text/plain' --data-binary "$large"
if [[ "$code" == 413 ]];then echo BODY_LIMIT=413;pass=$((pass+1));else echo "BODY_LIMIT=$code";fail=$((fail+1));fi
text_case WILDCARD GET /unregistered/deep 404 FALLBACK
if [[ $(cat "$out/root/outside/sentinel") == KEEP ]];then echo OUTSIDE_SENTINEL=PASS;pass=$((pass+1));else fail=$((fail+1));fi
if L08_HTTP_LOG_OUT="$out/privacy" L08_HTTP_LOG_SERVER="$out/server" L08_HTTP_LOG_ROOT="$out/root" bash "$ROOT/scripts/verify-l08-http-log.sh";then pass=$((pass+1));else fail=$((fail+1));fi
printf 'L08_HTTP_PLATFORM_PASS=%s FAIL=%s\n' "$pass" "$fail"
[[ "$fail" == 0 ]]
