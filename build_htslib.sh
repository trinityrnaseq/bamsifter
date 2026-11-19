#!/usr/bin/env bash

## Builds the bundled htslib release as a static library under htslib/build/

set -e -v

if [ -d "htslib" ]; then
    rm -rf ./htslib
fi

tar xf htslib-1.22.1.tar.bz2
mv htslib-1.22.1 htslib
cd htslib
mkdir -p build

# only local file I/O is needed by bamsifter; avoid optional network/compression deps
./configure --prefix=`pwd`/build/ --disable-libcurl --disable-gcs --disable-s3 --disable-plugins --without-libdeflate

make lib-static
make install
