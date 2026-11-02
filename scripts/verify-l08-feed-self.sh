#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)"
source "$ROOT/tests/l07/build.sh"
out="${L08_FEED_OUT:-$(mktemp -d)}"
mkdir -p "$out"
command -v xmllint >/dev/null || { echo FEED_XML=UNEXECUTED_MISSING_XMLLINT;exit 2; }
l07_build_compiler "$out" > "$out/compiler-build.log" 2>&1
l07_build_app "$out" "$ROOT/tests/l08-platform/feed.fl" feed > "$out/feed-build.log" 2>&1
pass=0;fail=0
for zone in UTC Asia/Seoul;do
 label=${zone//\//-}
 if TZ="$zone" "$out/feed" > "$out/$label.actual" 2> "$out/$label.stderr";then rc=0;else rc=$?;fi
 printf 'FEED_%s_PROGRAM_EXIT=%s\n' "$label" "$rc"
 if [[ "$rc" == 0 && ! -s "$out/$label.stderr" ]] && diff -u "$ROOT/tests/l08-platform/feed.expected" "$out/$label.actual";then pass=$((pass+1));else fail=$((fail+1));fi
done
awk -v dir="$out" '/^(RSS_FULL|RSS_EMPTY|SITEMAP_FULL|SITEMAP_EMPTY)$/{part=$0;next}{print > (dir "/" part ".xml")}' "$out/UTC.actual"
for name in RSS_FULL RSS_EMPTY SITEMAP_FULL SITEMAP_EMPTY;do
 if xmllint --nonet --noout "$out/$name.xml";then printf '%s_XML=PASS\n' "$name";pass=$((pass+1));else fail=$((fail+1));fi
done
xmllint --nonet --xpath 'string(/rss/channel/item[3]/*[local-name()="encoded"])' "$out/RSS_FULL.xml" > "$out/cdata.actual"
printf 'before]]>after\n' > "$out/cdata.expected"
if cmp -s "$out/cdata.expected" "$out/cdata.actual";then echo CDATA_CONTENT_PRESERVED=PASS;pass=$((pass+1));else echo CDATA_CONTENT_PRESERVED=FAIL;fail=$((fail+1));fi
"$out/compiler" "$ROOT/tests/l08-platform/feed-clock.fl" "$out/feed-clock.c" > "$out/clock-build.log" 2>&1
gcc -O2 -w -I "$ROOT/runtime" -o "$out/feed-clock" "$out/feed-clock.c" "$ROOT/tests/l08-platform/feed-clock.c" "${L07_SRCS[@]}" -Wl,--wrap=clock_gettime "${L07_LIBS[@]}" >> "$out/clock-build.log" 2>&1
if TZ=Asia/Seoul "$out/feed-clock" > "$out/clock.actual" 2> "$out/clock.stderr";then rc=0;else rc=$?;fi
printf 'FEED_CLOCK_PROGRAM_EXIT=%s\n' "$rc"
if [[ "$rc" == 0 && ! -s "$out/clock.stderr" ]] && diff -u "$ROOT/tests/l08-platform/feed-clock.expected" "$out/clock.actual";then pass=$((pass+1));else fail=$((fail+1));fi
if xmllint --nonet --noout "$out/clock.actual";then echo FEED_CLOCK_XML=PASS;pass=$((pass+1));else fail=$((fail+1));fi
printf 'L08_FEED_PASS=%s FAIL=%s\n' "$pass" "$fail"
[[ "$fail" == 0 ]]
