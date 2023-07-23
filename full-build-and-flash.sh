#!/usr/bin/env bash
set -euo pipefail

build() {
    >&2 echo "* build...."
    local board="$1"; shift
    local app="$1"; shift

    cmd="west build -p auto -b $board $app"

    case $board in
    esp32c3_devkitm|stamp_c3) cmd+=" --sysbuild --" ;;
    nucleo_*) cmd+=" -- -DSHIELD=adafruit_winc1500" ;;
    lpcxpresso55s69_cpu0) cmd+=" -- -DSHIELD=adafruit_winc1500" ;;
    *) cmd+=" --" ;;
    esac

    case $app in
    app) cmd+=' -DCONF_FILE="prj.conf _priv.prj.conf"'
    esac

    (set -x; eval "$cmd")
}

flash() {
    >&2 echo "* flash...."
    local board="$1"; shift
    case $board in
    lpcxpresso55s69_cpu0) west flash --runner jlink ;;
    *) west flash ;;
    esac
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
    flash "$board"
}

cd -- "$(dirname "$0")"
main "$@"