#!/usr/bin/env bash

set -e -v

if [ -d "htslib" ]; then
    rm -rf ./htslib
fi

tar xvf htslib-1.22.1.tar.bz2
mv htslib-1.22.1 htslib
cd htslib
mkdir -p build
#autoheader
#autoconf
./configure --prefix=`pwd`/build/
make
make install


