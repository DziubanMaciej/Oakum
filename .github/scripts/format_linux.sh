#!/bin/bash

install_clang_format() {
    which clang-format && return 0

    which apt-get
    if [ $? != 0 ]; then
        if [ `dpkg --list | grep " clang-format " | wc -l` -eq 0 ]; then
            sudo apt-get install clang-format || return 1
        fi
    fi

    which clang-format
    return $?
}

install_clang_format || {
    echo "ERROR: Could not install clang-format"
    exit 1
}

root_dir=`echo ${BASH_SOURCE[0]} | xargs realpath | xargs dirname | xargs dirname | xargs dirname` # TODO find better way to go up three levels? Adding ".." is ugly.
find "$root_dir" | grep -E "*\.(cpp|inl|h|c|hpp)$" | xargs clang-format -i --verbose
