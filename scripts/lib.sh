#!/bin/bash

ensureDirExist() {
    [ -d "${1}" ] || mkdir -p "${1}"
}

ensureDirExistAndClean() {
    ([ -d "${1}" ] || mkdir -p "${1}") && rm -rf "${1}/"*
}

ensureDirNotExist() {
    [ -d "${1}" ] && rm -r "${1}"
}

deal_inc_file() {
    local lines=""
    local incs=""
    readarray -t lines < $1
    for line in "${lines[@]}"; do
        path=$(cut -d'=' -f2 < <(echo $line))
        path=${path#*/}
        incs+="/${path} "
    done
    echo $incs
}

deal_src_file() {
    local lines=""
    local srcs=""
    readarray -t lines < $1
    for line in "${lines[@]}"; do
        path=$(cut -d'=' -f2 < <(echo $line))
        path=${path#*/}
        srcs+="/${path} "
    done
    echo $srcs