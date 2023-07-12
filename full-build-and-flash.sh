#!/usr/bin/env bash
set -euo pipefail

build() {
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
            "$app" \
            -- -DCONF_FILE="prj.conf _priv.prj.conf"
    # fi
}

flash() {
    west flash
}

monitor() {
    local board="$1"; shift
    case $board in
    esp32c3_devkitm|stamp_c3) exec west espressif monitor ;;
    *) echo "Monitor not supported for board"
    esac
}

main() {
    local board="${1:-stamp_c3}"
    local app="${2:-app}"

    >&2 echo "* build...."
    build "$board" "$app"
    >&2 echo "* flash...."
    west flash
    >&2 echo "* monitor...."
    monitor "$board"
}

cd -- "$(dirname "$0")"
main "$@"