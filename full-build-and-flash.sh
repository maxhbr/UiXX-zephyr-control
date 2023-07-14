#!/usr/bin/env bash
set -euo pipefail

build() {
    >&2 echo "* build...."
    local board="$1"; shift
    local app="$1"; shift
    # if [[ ! -d "./build" ]]; then
    #     west build \
    #         -p auto \
    #         -b "$board" \
    #         --sysbuild \
    #         "$app" \
    #         -- -DCONF_FILE="prj.conf _priv.prj.conf"
        
    #     build "$board" "$app"
    # else 
        west build \
            -p auto \
            -b "$board" \
            --sysbuild \
            "$app" 
            # \
            # -- -DCONF_FILE="prj.conf _priv.prj.conf"
    # fi
}

flash() {
    >&2 echo "* flash...."
    west flash
}

monitor() {
    >&2 echo "* monitor...."
    local board="$1"; shift
    case $board in
    esp32c3_devkitm|stamp_c3) exec west espressif monitor ;;
    *) echo "Monitor not supported for board"
    esac
}

main() {
    local board="${1:-stamp_c3}"
    local app="${2:-app}"

    build "$board" "$app"
    flash
}

cd -- "$(dirname "$0")"
main "$@"