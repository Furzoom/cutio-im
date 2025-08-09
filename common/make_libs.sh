#!/bin/bash

set -e

G_BASE_PATH="$(cd -- "$(dirname "$0")/.."; pwd)"
G_PREFIX=${G_BASE_PATH}/build/local

# build_install message package_name_without_postfix configure_script others
function build_install() {
    echo "start install $1..."
    pkg_name="$2"
    config=${3:-./configure}
    other_options=${@:4}
    prefix=${G_PREFIX}

    #rm -rf ${pkg_name}
    tar -zxvf "${pkg_name}".tar.gz
    pushd "${pkg_name}" > /dev/null
    ${config} --prefix="${prefix}" ${other_options}
    make -j8
    make -j8 install
    popd > /dev/null
    echo "$1 has installed"
}

function cmake_build_install() {
    echo "start install $1"
    pkg_name="$2"

    pushd "${pkg_name}" > /dev/null
    mkdir -p build && pushd build > /dev/null
    cmake -DCMAKE_INSTALL_PREFIX="${G_PREFIX}" ..
    make -j8
    make -j8 install
    popd > /dev/null
    popd > /dev/null
    echo "$1 has installed"
}

function main() {
    "${G_BASE_PATH}"/common/download_deps.sh

    pushd packets > /dev/null
    echo "start install..."

    export "PKG_CONFIG_PATH=${G_PREFIX}/lib/pkgconfig"

    build_install openssl openssl-1.1.1t ./config
    build_install libevent libevent-2.1.12-stable ./configure
    cmake_build_install yaml-cpp yaml-cpp

    popd > /dev/null

    # Lib
    echo "start make Lib..."
    echo "install success!"
}

main
