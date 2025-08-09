#!/bin/bash

check_sum=

function download() {
    wget --show-progress -O packets/$1 $2
    if [[ $? != 0 ]]; then
        echo "download $1 failed, $2"
        exit 1
    fi
}

function get_check_cmd() {
    os=$(uname -s)
    if [[ $os == "Darwin" ]]; then
        check_sum="shasum -a 1"
    elif [[ $os == "Linux" ]]; then
        check_sum="sha1sum"
    else
        echo "unsupported OS $os"
        exit 1
    fi

}

function check_packets {
    check_input=packets/check_sum.txt
    if [[ ! -f ${check_input} ]]; then
        return 1
    fi

    $check_sum -c ${check_input}
    return
}

function generate_check_sum() {
    $check_sum packets/*.gz > packets/check_sum.txt
}

function main() {
    if [[ ! -d packets ]]; then
        mkdir packets
    fi

    get_check_cmd

    check_packets
    if [[ $? == 0 ]]; then
        return
    fi

    download openssl-1.1.1t.tar.gz "https://www.openssl.org/source/openssl-1.1.1t.tar.gz"
    download libevent-2.1.12-stable.tar.gz "https://github.com/libevent/libevent/releases/download/release-2.1.12-stable/libevent-2.1.12-stable.tar.gz"

    generate_check_sum
}

main "$@"
