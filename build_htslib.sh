#!/usr/bin/env bash

set -e -v

cd htslib
git submodule init && git submodule update
mkdir -p build
autoheader
autoconf
./configure --prefix=`pwd`/build/
make
make install


