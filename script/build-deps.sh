#!/bin/bash

set -e

WORD_DIR=`pwd`

build_boost() {
    pushd ${WORD_DIR}/third-party/boost
    ./bootstrap.sh
    mkdir -p install
    ./b2 install link=static --prefix=`pwd`/install
    popd
}

build_xxhash() {
    pushd ${WORD_DIR}/third-party/xxhash
    make lib
    popd
}

build() {
    build_boost
    build_xxhash
}

${WORD_DIR}/script/use-submodules.sh
build
