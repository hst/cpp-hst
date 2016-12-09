#!/bin/sh
set -ev

srcdir="$PWD"

if [ ${BUILD_TYPE} = out_of_source ]; then
    mkdir .build
    cd .build
fi

if [ "$CXX" = "g++" ]; then
    export CXX="g++-4.8"
fi

${srcdir}/autogen.sh
${srcdir}/configure --enable-valgrind CXXFLAGS="-g -O3 -flto" LDFLAGS="-flto"
make
make check
make check-valgrind
make distcheck
