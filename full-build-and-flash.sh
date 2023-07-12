#!/usr/bin/env bash

cd -- "$(dirname "$0")"
set -euo pipefail
# west build -p auto -b "${1:-stamp_c3}" --sysbuild "${2:-app}" -- -DCONF_FILE="prj.conf"
west build -p auto -b "${1:-stamp_c3}" "${2:-app}" #-- -DCONF_FILE="prj.conf"
west flash