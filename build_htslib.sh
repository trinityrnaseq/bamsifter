#!/usr/bin/env bash

set -e -v

cd htslib
mkdir build
autoheader
autoconf
./configure --prefix=`pwd`
make
make install


