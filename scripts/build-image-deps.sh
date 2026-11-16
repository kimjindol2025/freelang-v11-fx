#!/usr/bin/env bash
# Isolated, pinned source build. No root access or system installation.
set -Eeuo pipefail
out=${1:?isolated output directory required}
: "${TMPDIR:?caller must select a task temporary directory}"
mkdir -p "$out/downloads" "$out/src" "$out/build" "$out/prefix" "$out/licenses"
out=$(cd "$out" && pwd -P)
fetch(){
  local file=$1 sha=$2 url=$3
  if [[ ! -f "$out/downloads/$file" ]];then wget -q --timeout=30 --tries=2 -O "$out/downloads/$file" "$url";fi
  printf '%s  %s\n' "$sha" "$out/downloads/$file" | sha256sum -c -
  tar -xzf "$out/downloads/$file" -C "$out/src"
}
fetch zlib-1.4.tar.gz 02dd159783677a749c04adbdd02b55b8a2bcc7e054f51b76b549199a916ad584 https://zlib.net/zlib-1.4.tar.gz
fetch libpng-1.6.59.tar.gz 86a3e4b501f7f50e392c4e456ea158893e9a595b1c63eb88c7bb9f6cf8772dad https://downloads.sourceforge.net/project/libpng/libpng16/1.6.59/libpng-1.6.59.tar.gz
fetch libjpeg-turbo-3.2.1.tar.gz c27ad72a68da151177929aa969ae773a2da5ac2af9ce946079314fdac8cece7f https://github.com/libjpeg-turbo/libjpeg-turbo/releases/download/3.2.1/libjpeg-turbo-3.2.1.tar.gz
fetch giflib-6.1.3.tar.gz b65b66b99f0424b93525f987386f22fc5efb9da2bfc92ad4a532249aaffbab0e https://downloads.sourceforge.net/project/giflib/giflib-6.x/giflib-6.1.3.tar.gz
cmake -S "$out/src/zlib-1.4" -B "$out/build/zlib" -DCMAKE_INSTALL_PREFIX="$out/prefix" -DCMAKE_INSTALL_LIBDIR=lib -DZLIB_BUILD_SHARED=OFF -DZLIB_BUILD_STATIC=ON -DZLIB_BUILD_TESTING=OFF
cmake --build "$out/build/zlib" -j1
cmake --install "$out/build/zlib"
cmake -S "$out/src/libpng-1.6.59" -B "$out/build/png" -DCMAKE_INSTALL_PREFIX="$out/prefix" -DCMAKE_INSTALL_LIBDIR=lib -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF -DPNG_TOOLS=OFF -DZLIB_INCLUDE_DIR="$out/prefix/include" -DZLIB_LIBRARY="$out/prefix/lib/libz.a"
cmake --build "$out/build/png" -j1
cmake --install "$out/build/png"
cmake -S "$out/src/libjpeg-turbo-3.2.1" -B "$out/build/jpeg" -DCMAKE_INSTALL_PREFIX="$out/prefix" -DENABLE_SHARED=OFF -DENABLE_STATIC=ON -DWITH_TURBOJPEG=OFF -DWITH_SIMD=OFF -DCMAKE_INSTALL_LIBDIR=lib
cmake --build "$out/build/jpeg" -j1
cmake --install "$out/build/jpeg"
make -C "$out/src/giflib-6.1.3" -j1 libgif.a
cp "$out/src/giflib-6.1.3/libgif.a" "$out/prefix/lib/"
cp "$out/src/giflib-6.1.3/gif_lib.h" "$out/prefix/include/"
cp "$out/src/zlib-1.4/LICENSE" "$out/licenses/zlib-LICENSE"
cp "$out/src/libpng-1.6.59/LICENSE" "$out/licenses/libpng-LICENSE"
cp "$out/src/libjpeg-turbo-3.2.1/LICENSE.md" "$out/licenses/jpeg-LICENSE.md"
cp "$out/src/libjpeg-turbo-3.2.1/README.ijg" "$out/licenses/jpeg-README.ijg"
cp "$out/src/giflib-6.1.3/COPYING" "$out/licenses/giflib-COPYING"
echo IMAGE_DEPENDENCIES_BUILT=4/4
