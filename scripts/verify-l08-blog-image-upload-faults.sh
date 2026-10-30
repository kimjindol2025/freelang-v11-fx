#!/usr/bin/env bash
set -Eeuo pipefail
umask 077
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
blog="${L08_BLOG_SOURCE_ROOT:?approved source-only copy required}"
compiler="${L08_BLOG_NATIVE_COMPILER:?}"
out="${L08_IMAGE_UPLOAD_OUT:-$(mktemp -d)}"
[[ -f "$blog/app/main.fl" && ! -e "$blog/data/blog.wal" ]]
mkdir -p "$out/source/tests"
ln -s "$blog/app" "$out/source/app"
ln -s "$blog/src" "$out/source/src"
flags=(-O1 -g -w -D_GNU_SOURCE -I "$ROOT/runtime")
if [[ -n "${L08_IMAGE_UPLOAD_SANITIZER_LIBDIR:-}" ]];then
 flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer -L "$L08_IMAGE_UPLOAD_SANITIZER_LIBDIR")
 export LD_LIBRARY_PATH="$L08_IMAGE_UPLOAD_SANITIZER_LIBDIR" ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1
fi
for phase in seed server;do
 if [[ -n "${L08_IMAGE_UPLOAD_BIN_DIR:-}" ]];then
  [[ -x "$L08_IMAGE_UPLOAD_BIN_DIR/$phase" ]]
  ln -s "$L08_IMAGE_UPLOAD_BIN_DIR/$phase" "$out/$phase"
  continue
 fi
 cp "$ROOT/tests/l08-platform/image-upload-fault-$phase.fl" "$out/source/tests/$phase.fl"
 extra=();support=()
 if [[ "$phase" == server ]];then
  extra=(--extern store-fault-arm)
  support=(-include "$ROOT/tests/l08-platform/store-wal-fault.h" "$ROOT/tests/l08-platform/store-wal-fault.c" "$ROOT/tests/l08/http-port.c" "$ROOT/tests/l08-platform/http-loopback.c" -Wl,--wrap=listen,--wrap=bind,--wrap=write,--wrap=close,--wrap=fsync)
 fi
 (cd "$out/source/tests";"$compiler" "${extra[@]}" "$phase.fl" "$out/$phase.c") > "$out/$phase.build.log" 2>&1
 gcc "${flags[@]}" -o "$out/$phase" "$out/$phase.c" "${support[@]}" "${L07_SRCS[@]}" "${L07_LIBS[@]}" >> "$out/$phase.build.log" 2>&1
done
printf 'GIF89a\0\xffimage-http' > "$out/proof.gif"
image_hash=$(sha256sum "$out/proof.gif" | cut -d' ' -f1)
image_size=$(wc -c < "$out/proof.gif")
export BLOG_TEST_SESSION_TOKEN="$(openssl rand -hex 32)" BLOG_TEST_AI_DRAFT_KEY="$(openssl rand -hex 32)" BLOG_CSRF_SECRET="$(openssl rand -hex 32)"
export BLOG_PORT=0 BLOG_COOKIE_SECURE=0 FL_WORKERS=4 AFJ_DB_MODE=development AFJ_DB_BIND_HOST=127.0.0.1
pid=""
stop(){
 [[ -n "$pid" ]] || return 0
 kill -TERM "$pid" 2>/dev/null || true
 for attempt in {1..200};do state=$(ps -p "$pid" -o stat= 2>/dev/null || true);[[ -n "$state" && "$state" != Z* ]] || break;sleep .05;done
 if [[ -n "${state:-}" && "$state" != Z* ]];then echo IMAGE_UPLOAD_OWN_SHUTDOWN=FAIL;kill -KILL "$pid" 2>/dev/null || true;wait "$pid" 2>/dev/null || true;pid="";return 1;fi
 if wait "$pid";then rc=0;else rc=$?;fi
 pid="";printf 'IMAGE_UPLOAD_OWN_SERVER_EXIT=%s\n' "$rc";[[ "$rc" == 0 ]]
}
trap 'stop || true' EXIT
start(){
 local generation="$1"
 export L08_PORT_FILE="$root/port-$generation"
 (cd "$root";exec "$out/server") > "$root/server-$generation.stdout" 2> "$root/server-$generation.stderr" &
 pid=$!
 for attempt in {1..300};do [[ ! -s "$L08_PORT_FILE" ]] || break;kill -0 "$pid" 2>/dev/null || break;sleep .05;done
 if [[ ! -s "$L08_PORT_FILE" ]];then echo IMAGE_UPLOAD_START=FAIL;return 1;fi
 base="http://127.0.0.1:$(cat "$L08_PORT_FILE")"
}
get(){
 curl -q --noproxy '*' -sS --max-time 15 -o "$2" -w '%{http_code}' "$base$1"
}
passed=0;failed=0
: > "$out/result"
for kind in admin ai;do
for mode in none prepare-write prepare-partial commit-sync commit-close;do
 root="$out/$kind-$mode";mkdir -p "$root/tmp"
 export FL_FILE_BASE="$root" TMPDIR="$root/tmp" AFJ_DB_WAL="$root/blog.wal" L08_UPLOAD_FAULT_MODE="$mode" L08_WAL_FAULT_MARKER="$root/fault-marker"
 (cd "$root";timeout 90 "$out/seed") > "$root/seed.stdout" 2> "$root/seed.stderr"
 [[ ! -s "$root/seed.stderr" && $(tail -n 1 "$root/seed.stdout") == IMAGE_UPLOAD_SEED=PASS ]]
 start initial
 # Sensitive values stay in private curl configs, never execve arguments.
 printf 'header = "Cookie: blog_session=%s"\n' "$BLOG_TEST_SESSION_TOKEN" > "$root/session.config"
 status=$(curl -q --noproxy '*' -sS --max-time 15 --config "$root/session.config" -o "$root/form.html" -w '%{http_code}' "$base/admin/images")
 [[ "$status" == 200 ]]
 csrf=$(sed -n "s/.*name='csrf' value='\([^']*\)'.*/\1/p" "$root/form.html" | head -n 1)
 [[ "$csrf" =~ ^[0-9]+\.[0-9a-f]+$ ]]
 {
  printf 'url = "%s%s"\n' "$base" "$([[ "$kind" == admin ]] && printf /admin/images || printf /api/ai/images)"
  if [[ "$kind" == admin ]];then printf 'header = "Cookie: blog_session=%s"\n' "$BLOG_TEST_SESSION_TOKEN";else printf 'header = "Authorization: Bearer %s"\n' "$BLOG_TEST_AI_DRAFT_KEY";fi
  printf 'form = "csrf=%s"\n' "$csrf"
  printf 'form = "editor_upload=1"\nform = "alt=시험 한🙂"\nform = "image=@%s;type=image/gif"\n' "$out/proof.gif"
 } > "$root/upload.config"
 if [[ "$mode" != none ]];then [[ $(get /__l08_upload_arm "$root/arm.body") == 200 && $(cat "$root/arm.body") == ARMED ]];fi
 if status=$(curl -q --noproxy '*' -sS --max-time 30 --config "$root/upload.config" -o "$root/upload.body" -w '%{http_code}');then curl_rc=0;else curl_rc=$?;fi
 notice=NA;wanted=200;[[ "$kind" != ai ]] || wanted=201
 if [[ "$mode" != none ]];then
  wanted=503;notice=FAIL
  if [[ "$kind" == admin ]];then rg -q '재시작' "$root/upload.body" && notice=PASS || true;else jq -e '.error == "restart_required"' "$root/upload.body" > /dev/null 2> "$root/body-parse.stderr" && notice=PASS || true;fi
 fi
 line="IMAGE_UPLOAD_POST=$kind:$mode STATUS=$status NOTICE=$notice"
 printf '%s\n' "$line" | tee -a "$out/result"
 if [[ "$curl_rc" == 0 && "$status" == "$wanted" && ( "$mode" == none || "$notice" == PASS ) ]];then passed=$((passed+1));else failed=$((failed+1));fi
 faults=0;[[ ! -f "$root/fault-marker" ]] || faults=$(wc -l < "$root/fault-marker")
 expected_faults=1;[[ "$mode" != none ]] || expected_faults=0
 printf 'IMAGE_UPLOAD_FAULTS=%s EXPECTED=%s\n' "$faults" "$expected_faults"
 [[ "$faults" == "$expected_faults" ]] || failed=$((failed+1))
 stop
 cp "$root/blog.wal" "$root/after-upload.wal"
 expected_rows=1;[[ "$mode" != prepare-* ]] || expected_rows=0
 for restart in 1 2;do
  start "$restart"
  status=$(get /__l08_upload_images "$root/rows-$restart.json")
  rows=FAIL;path="";image_status=NA;hash=FAIL;wal=FAIL;files=NA
  if [[ "$expected_rows" == 0 && "$status" == 200 ]] && jq -e 'length == 0' "$root/rows-$restart.json" > /dev/null 2> "$root/rows-parse.stderr";then
   rows=0;hash=NA;files=FAIL
   [[ -n $(find "$root/blog-images" -mindepth 1 -maxdepth 1 -print -quit) ]] || files=PASS
  elif [[ "$expected_rows" == 1 && "$status" == 200 ]] && jq -e --argjson bytes "$image_size" 'length == 1 and .[0].mime == "image/gif" and .[0].size == $bytes and .[0].alt == "시험 한🙂"' "$root/rows-$restart.json" > /dev/null 2> "$root/rows-parse.stderr";then
   rows=1;path=$(jq -r '.[0].path' "$root/rows-$restart.json")
   if [[ "$path" =~ ^/images/[0-9a-f]{32}\.gif$ ]];then
    image_status=$(get "$path" "$root/image-$restart.body")
    [[ $(sha256sum "$root/image-$restart.body" | cut -d' ' -f1) != "$image_hash" ]] || hash=PASS
   fi
  fi
  stop
  cmp -s "$root/blog.wal" "$root/after-upload.wal" && wal=PASS || true
  line="IMAGE_UPLOAD_RESTART=$kind:$mode:$restart ROWS=$rows IMAGE_HTTP=$image_status HASH=$hash WAL=$wal"
  [[ "$expected_rows" != 0 ]] || line+=" FILES=$files"
  printf '%s\n' "$line" | tee -a "$out/result"
  if [[ "$wal" == PASS && ( ( "$expected_rows" == 1 && "$rows" == 1 && "$image_status" == 200 && "$hash" == PASS ) || ( "$expected_rows" == 0 && "$rows" == 0 && "$hash" == NA && "$files" == PASS ) ) ]];then passed=$((passed+1));else failed=$((failed+1));fi
 done
done
done
printf 'IMAGE_UPLOAD_HTTP_PASS=%s FAIL=%s\n' "$passed" "$failed"
[[ "$failed" == 0 ]]
diff -u "$ROOT/tests/l08-platform/image-upload-fault.expected" "$out/result"
echo IMAGE_UPLOAD_AUTHENTICATION=SYNTHETIC_SESSION_AND_TEST_KEY
echo IMAGE_UPLOAD_PASSWORD_LOGIN=UNEXECUTED
